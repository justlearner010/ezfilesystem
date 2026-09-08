#include "fs.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PATH_BUF_SIZE 256   /* 路径缓冲大小：NAME_SIZE=20，层级有限，256 足够 */

/* 全局状态定义（fs.h 中 extern 声明，此处定义才真正分配内存） */
Directory *g_root      = NULL;
Directory *g_cwd       = NULL;
int        g_is_open   = 0;
File      *g_opend_file = NULL;

/* ============ 目录操作 ============ */

Directory *dir_new(const char *name){
    /* calloc 整体清零：parent / 两条链头 / 指针字段不再有垃圾值 */
    Directory *d = calloc(1, sizeof(Directory));
    d->subdirs = malloc(sizeof(HashTable));
    d->files   = malloc(sizeof(HashTable));
    memset(d->subdirs, 0, sizeof(HashTable));   /* 100 个桶必须清零，否则野指针 */
    memset(d->files,   0, sizeof(HashTable));
    strncpy(d->name, name, NAME_SIZE - 1);
    d->name[NAME_SIZE - 1] = '\0';
    return d;
}

Directory *dir_find_child(Directory *d, const char *name){
    return (Directory *)hash_find(d->subdirs, name);
}

void dir_add_child(Directory *d, Directory *child){
    child->nextbro_dir = d->firstchild_dir;   /* ① 头插：新节点指向旧头 */
    d->firstchild_dir  = child;               /* ② 自己成为新头（新→旧） */
    child->parent = d;                        /* ③ 认爹 */
    hash_insert(d->subdirs, child->name, child); /* ④ 哈希表同步 */
}

void dir_remove_child(Directory *d, Directory *child){
    /* 摘链表：支持头结点与中间/尾部两种情况 */
    if (d->firstchild_dir == child) {
        d->firstchild_dir = child->nextbro_dir;
    } else {
        Directory *p = d->firstchild_dir;
        while (p != NULL && p->nextbro_dir != child)
            p = p->nextbro_dir;
        if (p != NULL)
            p->nextbro_dir = child->nextbro_dir;
    }
    hash_delete(d->subdirs, child->name);     /* 哈希表同步 */
}

void dir_destroy(Directory *d){
    /* 第1步：删光名下所有文件（摘链 + 删哈希 + 释放） */
    while (d->firstchild_file) {
        File *f = d->firstchild_file;
        file_remove(d, f);                    /* file_remove 已摘链，头指针会更新 */
        free(f);
    }
    /* 第2步：对每个子目录先摘链再递归销毁（避免悬垂） */
    while (d->firstchild_dir) {
        Directory *c = d->firstchild_dir;
        dir_remove_child(d, c);
        dir_destroy(c);
    }
    /* 第3步：子节点清空后释放自己 */
    hash_destroy(d->subdirs);
    hash_destroy(d->files);
    free(d);
}

void dir_rename(Directory *d, Directory *c, const char *newname) {              
      hash_delete(d->subdirs, c->name);                                           
      strncpy(c->name, newname, NAME_SIZE - 1);                                   
      c->name[NAME_SIZE - 1] = '\0';                                              
      hash_insert(d->subdirs, c->name, c);                                        
  }                             

/* ============ 文件操作 ============ */

File *file_new(const char *name){
    File *f = calloc(1, sizeof(File));   /* content 清零 = 新文件内容为空 */
    strncpy(f->name, name, NAME_SIZE - 1);
    f->name[NAME_SIZE - 1] = '\0';
    return f;
}

File *file_find(Directory *d, const char *name){
    return (File *)hash_find(d->files, name);
}

void file_add(Directory *d, File *f){
    f->nextbro_file = d->firstchild_file;  /* 头插：新→旧 */
    d->firstchild_file = f;
    hash_insert(d->files, f->name, f);
}

void file_remove(Directory *d, File *f){
    /* 摘链表：支持头结点与中间/尾部两种情况 */
    if (d->firstchild_file == f) {
        d->firstchild_file = f->nextbro_file;
    } else {
        File *p = d->firstchild_file;
        while (p != NULL && p->nextbro_file != f)
            p = p->nextbro_file;
        if (p != NULL)
            p->nextbro_file = f->nextbro_file;
    }
    hash_delete(d->files, f->name);
}

/* ============ 遍历（先根 / 后根） ============ */
/* path 由调用方提供缓冲，len 为当前路径有效长度；返回时还原，递归不越拼越长 */

void walk_pre(Directory *cur, char *path, int len) {
    for (File *f = cur->firstchild_file; f; f = f->nextbro_file)
        printf("File %s%s\n", path, f->name);           /* path 末尾带 '/'，根层为空 */

    for (Directory *c = cur->firstchild_dir; c; c = c->nextbro_dir) {
        printf("Dir  %s%s\n", path, c->name);           /* 先根：目录自身先打 */
        int len2 = len + strlen(c->name) + 1;            /* 新路径含末尾 '/' */
        snprintf(path + len, PATH_BUF_SIZE - len, "%s/", c->name);
        walk_pre(c, path, len2);
        path[len] = '\0';                               /* 还原 */
    }
}

void walk_post(Directory *cur, char *path, int len) {
    for (File *f = cur->firstchild_file; f; f = f->nextbro_file)
        printf("File %s%s\n", path, f->name);

    for (Directory *c = cur->firstchild_dir; c; c = c->nextbro_dir) {
        int len2 = len + strlen(c->name) + 1;
        snprintf(path + len, PATH_BUF_SIZE - len, "%s/", c->name);
        walk_post(c, path, len2);
        /* 后根：子树输出完了，最后打目录自身；掐掉末尾 '/' 得到 "父路径/name" */
        path[len2 - 1] = '\0';
        printf("Dir  %s\n", path);
        path[len] = '\0';                               /* 还原 */
    }
}

void file_rename(Directory *d, File *f, const char *newname) {                  
       hash_delete(d->files, f->name);      /* ① 用旧名删掉哈希 key */             
       strncpy(f->name, newname, NAME_SIZE - 1);   /* ② 改名字 */                  
       f->name[NAME_SIZE - 1] = '\0';                                              
       hash_insert(d->files, f->name, f);   /* ③ 用新名插入 key */                 
       /* 链表完全没碰，位置保持 ✓ */                                              
   }             

  void find_walk(Directory *cur, char *path, int len, const char *kw, int *found, int print) {
    for (File *f = cur->firstchild_file; f; f = f->nextbro_file) {
        if (strstr(f->name, kw) != NULL) {          /* human 版 strstr 顶替 KMP */
            (*found)++;
            if (print) printf("%s%s\n", path, f->name);   /* 只有输出趟才打路径 */
        }
    }
    for (Directory *c = cur->firstchild_dir; c; c = c->nextbro_dir) {
        int len2 = len + strlen(c->name) + 1;
        snprintf(path + len, 256 - len, "%s/", c->name);
        find_walk(c, path, len2, kw, found, print);
        path[len] = '\0';                         /* 还原 */
    }
}

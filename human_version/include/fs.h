#ifndef FS_H
#define FS_H

#include "hash.h"

#define PATH_BUF_SIZE 256   /* 路径拼接缓冲大小（NAME_SIZE=20，256 足够） */

/* 文件：name 定长，content 定长数组（human 版不支持任意长度内容） */
typedef struct File {
    char name[NAME_SIZE];
    char content[CONTENT_SIZE];
    struct File *nextbro_file;      /* 同一目录下文件链表的兄弟指针 */
} File;

/* 目录：孩子-兄弟表示法 + 两张哈希表（子目录 / 文件）加速按名查找 */
typedef struct Directory {
    char name[NAME_SIZE];
    struct Directory *parent;          /* 父目录（根为 NULL） */
    struct Directory *firstchild_dir;  /* 子目录链表头 */
    struct Directory *nextbro_dir;     /* 兄弟目录指针 */
    struct File *firstchild_file;      /* 文件链表头 */
    HashTable *subdirs;                /* name -> Directory* */
    HashTable *files;                  /* name -> File* */
} Directory;

/* 全局状态：根目录 / 当前目录 / 打开文件状态机 */
extern Directory *g_root;
extern Directory *g_cwd;
extern int        g_is_open;
extern File      *g_opend_file;

/* ---------- 目录树操作 ---------- */
Directory *dir_new(const char *name);                              /* 新建空目录 */
Directory *dir_find_child(Directory *d, const char *name);         /* 按名找子目录 */
void       dir_add_child(Directory *d, Directory *child);          /* 挂到 d 名下 */
void       dir_remove_child(Directory *d, Directory *child);       /* 从 d 名下摘除（不释放） */
void       dir_destroy(Directory *d);                              /* 递归销毁整棵子树 */
void       dir_rename(Directory *d, Directory *c, const char *newname);

/* ---------- 文件操作 ---------- */
File *file_new(const char *name);                                  /* 新建空文件 */
File *file_find(Directory *d, const char *name);                   /* 按名找文件 */
void  file_add(Directory *d, File *f);                             /* 挂到 d 名下 */
void  file_remove(Directory *d, File *f);                          /* 从 d 名下摘除（不释放） */
void  file_rename(Directory *d, File *f, const char *newname);

/* ---------- 遍历（find_file / ll_pre / ll_post 共用） ----------
 * path 由调用方提供缓冲，len 为 path 当前有效长度；
 * 递归进入子目录时在末尾补 '/'，返回前还原，保证 path 不会越拼越长。
 */
void find_walk(Directory *cur, char *path, int len,
               const char *kw, int *found, int print);
void walk_pre (Directory *cur, char *path, int len);   /* 先根遍历 */
void walk_post(Directory *cur, char *path, int len);   /* 后根遍历 */

#endif /* FS_H */

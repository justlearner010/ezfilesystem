#include "cmd.h"
#include "fs.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== 公共校验 ==================== */

/* Issue #2 / Q1 决策 A：名字超长一律拒绝，避免静默截断 */
static int name_too_long(const char *name) {
    return name == NULL || strlen(name) >= NAME_SIZE;
}

/* ==================== 创建 / 删除 ==================== */

void cmd_create_file(const char *name) {
    if (name_too_long(name)) {
        printf("ERROR: name too long\n");
        return;
    }
    if (file_find(g_cwd, name)) {
        printf("ERROR: file %s already exists\n", name);
    } else {
        file_add(g_cwd, file_new(name));
        printf("SUCCESS: created file %s\n", name);
    }
}

void cmd_create_dir(const char *name) {
    if (name_too_long(name)) {
        printf("ERROR: name too long\n");
        return;
    }
    if (dir_find_child(g_cwd, name)) {
        printf("ERROR: directory %s already exists\n", name);
    } else {
        dir_add_child(g_cwd, dir_new(name));
        printf("SUCCESS: created directory %s\n", name);
    }
}

void cmd_delete_file(const char *name) {
    File *f = file_find(g_cwd, name);
    if (!f) {
        printf("ERROR: file not found\n");
    } else {
        file_remove(g_cwd, f);
        file_free(f);                    /* AI 增强：释放动态 content 与节点 */
        printf("SUCCESS: %s deleted\n", name);
    }
}

void cmd_delete_dir(const char *name) {
    Directory *d = dir_find_child(g_cwd, name);
    if (!d) {
        printf("ERROR: dir not found\n");
    } else {
        dir_remove_child(g_cwd, d);
        dir_destroy(d);
        printf("SUCCESS: %s deleted\n", name);
    }
}

/* ==================== 重命名 ==================== */

void cmd_rename_file(const char *old, const char *neu) {
    File *f = file_find(g_cwd, old);
    if (!f) {
        printf("ERROR: %s not found\n", old);
        return;
    }
    if (name_too_long(neu)) {
        printf("ERROR: name too long\n");
        return;
    }
    if (file_find(g_cwd, neu)) {                 /* 查重只查文件表：同名目录可以共存 */
        printf("ERROR: file %s already exists\n", neu);
        return;
    }
    char oldname[NAME_SIZE];
    strcpy(oldname, f->name);                    /* 改名后 f->name 就变了，先存旧名 */
    file_rename(g_cwd, f, neu);
    printf("SUCCESS: renamed file %s %s\n", oldname, neu);
}

void cmd_rename_dir(const char *old, const char *neu) {
    Directory *d = dir_find_child(g_cwd, old);
    if (!d) {
        printf("ERROR: %s not found\n", old);
        return;
    }
    if (name_too_long(neu)) {
        printf("ERROR: name too long\n");
        return;
    }
    if (dir_find_child(g_cwd, neu)) {            /* 查重只查目录表：同名文件可以共存 */
        printf("ERROR: dir %s already exists\n", neu);
        return;
    }
    dir_rename(g_cwd, d, neu);
    printf("SUCCESS: renamed dir %s %s\n", old, neu);
}

/* ==================== 查找 ==================== */

void cmd_find_file(const char *kw) {
    char path[PATH_BUF_SIZE] = {0};
    int found = 0;

    find_walk(g_cwd, path, 0, kw, &found, 0);    /* 第 1 遍：只统计不输出 */
    if (found == 0) {
        printf("ERROR: %s not found\n", kw);
        return;
    }

    printf("SEARCH RESULTS:\n");                 /* 确认有结果，才打这行头 */
    memset(path, 0, sizeof(path));
    found = 0;
    find_walk(g_cwd, path, 0, kw, &found, 1);    /* 第 2 遍：正式输出 */
}

/* ==================== 列目录 / 遍历 ==================== */

void cmd_ls(void) {
    for (File *f = g_cwd->firstchild_file; f != NULL; f = f->nextbro_file)
        printf("File %s\n", f->name);

    for (Directory *d = g_cwd->firstchild_dir; d != NULL; d = d->nextbro_dir)
        printf("Dir  %s\n", d->name);            /* 注意：Dir 后是两个空格 */
}

void cmd_ll_pre(void) {
    char path[PATH_BUF_SIZE] = {0};
    walk_pre(g_cwd, path, 0);
}

void cmd_ll_post(void) {
    char path[PATH_BUF_SIZE] = {0};
    walk_post(g_cwd, path, 0);
}

/* ==================== 切换目录 ==================== */

void cmd_cd(const char *name) {
    if (strcmp(name, "..") == 0) {
        if (g_cwd->parent != NULL)
            g_cwd = g_cwd->parent;               /* 根目录下 cd .. 静默忽略 */
        return;                                  /* 处理完 `..` 必须返回，否则会去查名为 ".." 的子目录 */
    }
    Directory *d = dir_find_child(g_cwd, name);
    if (!d) {
        printf("ERROR: dir not found\n");
        return;
    }
    g_cwd = d;                                   /* cd 成功无输出 */
}

/* ==================== 文件读写状态机 ==================== */

void cmd_open(const char *name) {
    File *f = file_find(g_cwd, name);
    if (!f) {
        printf("ERROR: %s not found\n", name);
        return;
    }
    g_is_open    = 1;
    g_opend_file = f;
    printf("SUCCESS: opened %s\n", name);
}

void cmd_close_file(void) {
    if (!g_opend_file) {                         /* 从未 open：防 NULL 解引用崩溃 */
        printf("ERROR: invalid operation\n");
        return;
    }
    printf("SUCCESS: closed %s\n", g_opend_file->name);
    g_is_open    = 0;
    g_opend_file = NULL;
}

void cmd_read_file(const char *name) {
    File *f = file_find(g_cwd, name);
    if (!f) {
        printf("ERROR: %s not found\n", name);
        return;
    }
    printf("CONTENT: %s\n", f->content);
}

void cmd_write_file(const char *content) {
    if (!g_opend_file) {
        printf("ERROR: invalid operation\n");
        return;
    }

    File  *f   = g_opend_file;
    size_t add = strlen(content);

    /* 容量不足则倍增扩容（Issue #2 / Q2 决策 C：动态扩容，无长度上限） */
    while (f->content_len + add + 1 > f->content_cap) {
        size_t ncap = f->content_cap ? f->content_cap * 2 : 64;
        char  *nc   = realloc(f->content, ncap);
        if (!nc) {
            printf("ERROR: out of memory\n");
            return;                       /* 扩容失败：内容保持不变 */
        }
        f->content     = nc;
        f->content_cap = ncap;
    }

    memcpy(f->content + f->content_len, content, add + 1);   /* 连结尾 '\0' 一起拷 */
    f->content_len += add;
    printf("SUCCESS: successfully written\n");
}

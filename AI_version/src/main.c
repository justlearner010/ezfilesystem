#include "cmd.h"
#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== 整行读取（不依赖 POSIX getline） ==================== */

/* 读一整行到 *buf（按需扩容，行多长都读得下），行尾的 '\n' 不入缓冲。
 * 返回行长；到达 EOF 且一个字符都没读到时返回 -1。
 *
 * 为什么不用 POSIX 的 getline：MSVC 与 Dev-C++ 自带的 MinGW.org 都没有它，
 * 而那正是 Windows 上最常见的两个 C 环境；自实现一份就能到处编译。
 */
static long read_line(char **buf, size_t *cap, FILE *fp) {
    size_t len = 0;
    int    c;

    if (*buf == NULL || *cap == 0) {
        *cap = 128;
        *buf = malloc(*cap);
        if (*buf == NULL)
            return -1;
    }

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n')
            break;                          /* 行尾：换行符本身不存 */
        if (len + 1 >= *cap) {              /* +1 给结尾 '\0' 留位置 */
            size_t ncap = *cap * 2;
            char  *nb   = realloc(*buf, ncap);
            if (nb == NULL)
                return -1;
            *buf = nb;
            *cap = ncap;
        }
        (*buf)[len++] = (char)c;
    }

    if (c == EOF && len == 0)
        return -1;                          /* 无数据即 EOF */

    (*buf)[len] = '\0';
    return (long)len;
}

/* ==================== 入参解析表 ==================== */

/* 各命令所需参数个数（缺参校验用）；未知命令返回 -1（静默忽略，见 Issue #4 决策） */
static int need_args(const char *cmd) {
    static const struct {
        const char *name;
        int         n;
    } tab[] = {
        {"create_file", 1}, {"create_dir", 1}, {"delete_file", 1}, {"delete_dir", 1},
        {"rename_file", 2}, {"rename_dir", 2}, {"find_file",   1},
        {"ls",          0}, {"ll_pre",     0}, {"ll_post",     0},
        {"cd",          1}, {"open",       1}, {"close_file",  0},
        {"read_file",   1}, {"write_file", 1},
    };

    for (size_t i = 0; i < sizeof(tab) / sizeof(tab[0]); i++)
        if (strcmp(cmd, tab[i].name) == 0)
            return tab[i].n;

    return -1;
}

/* 命令主循环：读一行 → 拆词 → 状态机拦截 → 缺参校验 → 分发
 *
 * AI 版相对 human 版的增强（Issue #3 输入安全）：
 *   Q1 决策 B：整行动态读取，超长行不截断
 *   Q2 决策 A：缺必需参数统一报 invalid operation
 *   Q3 决策 A：write_file 必须带成对引号
 *   Q4 决策 A：多余参数忽略
 */

int main(void) {
    g_root = dir_new("root");
    g_cwd  = g_root;

    char  *line = NULL;
    size_t cap  = 0;

    while (1) {
        printf(">> ");
        long n = read_line(&line, &cap, stdin);     /* 动态读取：超长行不截断 */
        if (n == -1)
            break;                                 /* EOF 退出 */
                                                   /* 行尾 '\n' 已在 read_line 里去掉 */

        /* 在拆词前定位引号（strtok 会破坏 line，引号解析必须提前） */
        char *q1 = strchr(line, '"');
        char *q2 = q1 ? strchr(q1 + 1, '"') : NULL;

        char *cmd = strtok(line, " ");
        if (cmd == NULL)
            continue;                              /* 空行忽略 */

        /* open 状态机拦截：已打开时，非 write_file / close_file 一律拒绝 */
        if (g_is_open && strcmp(cmd, "write_file") != 0 && strcmp(cmd, "close_file") != 0) {
            printf("ERROR: invalid operation\n");
            continue;
        }

        /* write_file：格式必须为 write_file "content"，引号成对 */
        if (strcmp(cmd, "write_file") == 0) {
            if (!g_is_open || !q1 || !q2) {        /* 未打开 / 无引号 / 引号残缺 */
                printf("ERROR: invalid operation\n");
                continue;
            }
            *q2 = '\0';                            /* 截断右引号 */
            cmd_write_file(q1 + 1);                /* 取引号内内容 */
            continue;
        }

        int need = need_args(cmd);
        if (need < 0)
            continue;                              /* 未知命令：静默忽略 */

        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");

        /* 缺必需参数 → 统一报 invalid operation；多余参数忽略 */
        if (need >= 1 && (!arg1 || *arg1 == '\0')) {
            printf("ERROR: invalid operation\n");
            continue;
        }
        if (need >= 2 && (!arg2 || *arg2 == '\0')) {
            printf("ERROR: invalid operation\n");
            continue;
        }

        if      (strcmp(cmd, "create_file") == 0) cmd_create_file(arg1);
        else if (strcmp(cmd, "create_dir")  == 0) cmd_create_dir (arg1);
        else if (strcmp(cmd, "delete_file") == 0) cmd_delete_file(arg1);
        else if (strcmp(cmd, "delete_dir")  == 0) cmd_delete_dir (arg1);
        else if (strcmp(cmd, "rename_file") == 0) cmd_rename_file(arg1, arg2);
        else if (strcmp(cmd, "rename_dir")  == 0) cmd_rename_dir (arg1, arg2);
        else if (strcmp(cmd, "find_file")   == 0) cmd_find_file  (arg1);
        else if (strcmp(cmd, "ls")          == 0) cmd_ls();
        else if (strcmp(cmd, "ll_pre")      == 0) cmd_ll_pre();
        else if (strcmp(cmd, "ll_post")     == 0) cmd_ll_post();
        else if (strcmp(cmd, "cd")          == 0) cmd_cd(arg1);
        else if (strcmp(cmd, "open")        == 0) cmd_open(arg1);
        else if (strcmp(cmd, "close_file")  == 0) cmd_close_file();
        else if (strcmp(cmd, "read_file")   == 0) cmd_read_file(arg1);
    }

    free(line);
    dir_destroy(g_root);   /* AI 增强：退出前释放整棵目录树，ASan 泄漏检测干净 */
    return 0;
}

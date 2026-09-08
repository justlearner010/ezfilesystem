#define _POSIX_C_SOURCE 200809L   /* getline 需要 POSIX 支持 */

#include "cmd.h"
#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 命令所需参数个数（缺参校验用）；未知命令返回 -1（静默忽略，Issue #4 再统一决策） */
static int need_args(const char *cmd) {
    struct { const char *name; int n; } tab[] = {
        {"create_file", 1}, {"create_dir", 1}, {"delete_file", 1}, {"delete_dir", 1},
        {"rename_file", 2}, {"rename_dir", 2}, {"find_file", 1},
        {"ls", 0}, {"ll_pre", 0}, {"ll_post", 0}, {"cd", 1}, {"open", 1},
        {"close_file", 0}, {"read_file", 1}, {"write_file", 1},
    };
    for (size_t i = 0; i < sizeof(tab) / sizeof(tab[0]); i++)
        if (strcmp(cmd, tab[i].name) == 0)
            return tab[i].n;
    return -1;
}

int main(void) {
    g_root = dir_new("root");
    g_cwd  = g_root;

    char *line = NULL;
    size_t cap = 0;

    while (1) {
        printf(">> ");
        ssize_t n = getline(&line, &cap, stdin);   /* 动态读取：超长行不截断（Q1 决策 B） */
        if (n == -1) break;                        /* EOF */
        line[strcspn(line, "\n")] = '\0';          /* 去换行 */

        /* 在拆词前定位引号（strtok 会破坏 line，引号解析必须提前） */
        char *q1 = strchr(line, '"');
        char *q2 = q1 ? strchr(q1 + 1, '"') : NULL;

        char *cmd = strtok(line, " ");
        if (!cmd) continue;                        /* 空行忽略 */

        /* open 状态机拦截：已打开时非 write_file/close_file 一律拒绝 */
        if (g_is_open && strcmp(cmd, "write_file") != 0 && strcmp(cmd, "close_file") != 0) {
            printf("ERROR: invalid operation\n");
            continue;
        }

        /* write_file：格式必须为 write_file "content"，引号成对（Q3 决策 A） */
        if (strcmp(cmd, "write_file") == 0) {
            if (!g_is_open || !q1 || !q2) {        /* 未打开 / 无引号 / 引号残缺 */
                printf("ERROR: invalid operation\n");
                continue;
            }
            *q2 = '\0';                            /* 截断右引号 */
            cmd_write_file(q1 + 1);                /* 引号内内容 */
            continue;
        }

        int need = need_args(cmd);
        if (need < 0) continue;                    /* 未知命令：静默忽略（Issue #4 再定） */

        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");

        /* Q2 决策 A：缺必需参数 → 统一报 invalid operation；Q4 决策 A：多余参数忽略 */
        if (need >= 1 && (!arg1 || *arg1 == '\0')) { printf("ERROR: invalid operation\n"); continue; }
        if (need >= 2 && (!arg2 || *arg2 == '\0')) { printf("ERROR: invalid operation\n"); continue; }

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
        /* 若走到这里说明 need>=0 但不在表中，不会发生 */
    }

    free(line);
    dir_destroy(g_root);   /* AI：退出前释放整棵目录树，ASan 泄漏检测干净 */
    return 0;
}
#include "cmd.h"
#include "fs.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    g_root = dir_new("root");
    g_cwd  = g_root;

    char line[256];
    while (1) {
        printf(">> ");                                  /* 每次读取命令前 */
        if (!fgets(line, sizeof line, stdin)) break;    /* EOF 退出 */
        line[strcspn(line, "\n")] = '\0';               /* 去换行 */

        char *cmd = strtok(line, " ");
        if (!cmd) continue;                             /* 空行忽略 */

        /* open 状态机拦截：已打开时，非 write_file/close_file 一律拒绝 */
        if (g_is_open && strcmp(cmd, "write_file") != 0 && strcmp(cmd, "close_file") != 0) {
            printf("ERROR: invalid operation\n");
            continue;
        }

        /* write_file 特殊：内容被引号包裹、可含空格，单独解析 */
        if (strcmp(cmd, "write_file") == 0) {
            char *content = strtok(NULL, "\"");         /* 跳过左引号，取引号内内容 */
            if (!g_is_open)
                printf("ERROR: invalid operation\n");
            else if (content)
                cmd_write_file(content);
            continue;
        }

        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");

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
        /* 未知命令：题目未定义，human 版静默忽略 */
    }
    return 0;
}
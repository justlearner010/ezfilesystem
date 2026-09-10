#ifndef CMD_H
#define CMD_H

/* 15 条命令的处理函数；入参解析在 main.c 完成，此处只做业务与输出。
 * 所有输出格式（含空格数量）以 SPEC.md §5 为准。
 */

void cmd_create_file(const char *name);
void cmd_create_dir (const char *name);
void cmd_delete_file(const char *name);
void cmd_delete_dir (const char *name);
void cmd_rename_file(const char *old, const char *neu);
void cmd_rename_dir (const char *old, const char *neu);
void cmd_find_file  (const char *kw);
void cmd_ls(void);
void cmd_ll_pre(void);
void cmd_ll_post(void);
void cmd_cd(const char *name);
void cmd_open(const char *name);
void cmd_close_file(void);
void cmd_read_file(const char *name);
void cmd_write_file(const char *content);

#endif /* CMD_H */

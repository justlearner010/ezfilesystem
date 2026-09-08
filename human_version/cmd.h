#ifndef CMD_H
#define CMD_H

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

#endif
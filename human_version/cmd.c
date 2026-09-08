#include "cmd.h"
#include "fs.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmd_create_file(const char *name) {
    if (file_find(g_cwd, name)){
        printf("ERROR: file %s already exists\n",name);
        
    }
    else {
        file_add(g_cwd, file_new(name));
        printf("SUCCESS: created file %s\n",name);
    }
}

void cmd_create_dir(const char *name) {                                         
       if (dir_find_child(g_cwd, name))                                            
           printf("ERROR: directory %s already exists\n", name);                   
       else {                                                                      
           dir_add_child(g_cwd, dir_new(name));                                    
           printf("SUCCESS: created directory %s\n", name);                        
       }                                                                           
   }                  

void cmd_delete_file(const char *name) {
    File *f = file_find(g_cwd, name);
    if (!f) {
        printf("ERROR: file not found\n");
    }else {
        file_remove(g_cwd,f);
        free(f);
        printf("SUCCESS: %s deleted\n",name);
        
    }
}

void cmd_delete_dir(const char *name) {
    Directory *d = dir_find_child(g_cwd,name);
    if (!d) {
        printf("ERROR: dir not found\n");
    }else {
        dir_remove_child(g_cwd, d);
        dir_destroy(d);
        printf("SUCCESS: %s deleted\n",name);
    }
}


void cmd_rename_file(const char *old, const char *neu) {
    File *f = file_find(g_cwd, old);
    char oldname[NAME_SIZE];
    if (!f) {
        printf("ERROR: %s not found\n",old);
        return;
    }
    if (file_find(g_cwd, neu)) {
        printf("ERROR: file %s already exists\n",neu);
        return;
    }
    strcpy(oldname, f->name);
    file_rename(g_cwd,f, neu);
    printf("SUCCESS: renamed file %s %s\n",oldname,neu);
}

void cmd_rename_dir(const char *old, const char *neu) {
    Directory *d = dir_find_child(g_cwd, old);  
    if (!d) {
        printf("ERROR: %s not found\n",old);
        return;
    }
    if(dir_find_child(g_cwd, neu)) {
        printf("ERROR: dir %s already exists\n",neu);
        return;
    }
    dir_rename(g_cwd, d, neu);                
    printf("SUCCESS: renamed dir %s %s\n", old, neu);
    
}

void cmd_find_file(const char *kw) {                                            
       char path[256] = {0};                                                       
       int found = 0;                                                              
       find_walk(g_cwd, path, 0, kw, &found, 0);   /* 第1遍：只统计不输出 */
    if (found == 0) { printf("ERROR: %s not found\n", kw); return; }            
                                                                                   
       printf("SEARCH RESULTS:\n");             /* 确认有结果，才打这行头 */       
       memset(path, 0, sizeof(path));                                              
       found = 0;                                                                  
       find_walk(g_cwd, path, 0, kw, &found, 1);   /* 第2遍：正式输出 */}                                                                      

void cmd_ls() {
    for (File *f = g_cwd->firstchild_file; f != NULL; f= f->nextbro_file) {
        printf("File %s\n",f->name);
    }

    for (Directory *d = g_cwd->firstchild_dir; d!= NULL; d =d->nextbro_dir) {
        printf("Dir  %s\n",d->name);
    }
}

void cmd_ll_pre() {
    char path[256] = {0};
    walk_pre(g_cwd, path, 0);
}

void cmd_ll_post() {
    char path[256] = {0};
    walk_post(g_cwd, path, 0);
}

void cmd_cd(const char *name) {
    if (strcmp(name, "..") == 0){
        if (g_cwd->parent != NULL) {
            g_cwd = g_cwd->parent;
        }
        return;                          /* ★ 处理完 `..` 必须返回，否则会继续查名为 ".." 的子目录 */
    }
    Directory *d = dir_find_child(g_cwd, name);
    if (!d) {
        printf("ERROR: dir not found\n");
        return;
    }
    g_cwd = d;
}

void cmd_open(const char *name) {
    File *f = file_find(g_cwd,name);
    if (!f) {
        printf("ERROR: %s not found\n",name);
        return;
    }
    g_is_open = 1;
    g_opend_file = f;
    printf("SUCCESS: opened %s\n",name);
}

void cmd_close_file() {
    printf("SUCCESS: closed %s\n",g_opend_file->name);
    g_is_open = 0;
    g_opend_file = NULL;
}

void cmd_read_file(const char *name) {
    File *f = file_find(g_cwd, name);                         
    if (!f) { printf("ERROR: %s not found\n", name); return; }    
    printf("CONTENT: %s\n",f->content);
}

void cmd_write_file(const char *content) {
    if (!g_opend_file) {
        printf("ERROR: invalid operation\n");
        return;
    }
    strcat(g_opend_file->content,content);
    printf("SUCCESS: successfully written\n");
}
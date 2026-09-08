#ifndef FS_H
#define  FS_H
#define PATH_BUF_SIZE 256
#include "hash.h"

typedef struct File { //文件的结构体
    char name[NAME_SIZE];
    char content[CONTENT_SIZE];
    struct File *nextbro_file;
} File;

typedef struct Directory { //目录的结构体
    char name[NAME_SIZE];
     struct Directory *parent;
     struct Directory *firstchild_dir;
     struct Directory *nextbro_dir;
     struct File *firstchild_file;
    HashTable *subdirs;
    HashTable *files;
} Directory;

extern Directory *g_root; // 根目录
extern Directory *g_cwd; //当前目录
extern int g_is_open;// 文件打开状态
extern File *g_opend_file;//当前打开的文件

//相关函数
   /* 目录树操作 */                                                                                
Directory *dir_new(const char *name); //新建空目录

Directory *dir_find_child(Directory *d, const char *name);                                      
void dir_add_child(Directory *d, Directory *child);       //新建子目录           
void dir_remove_child(Directory *d, Directory *child);    //删除子目录  
void dir_destroy(Directory *d);                           //删除整个目录
                                                                                                    
    /* 文件操作 */                                                                                  
File *file_new(const char *name);//新建文件                                                               
File *file_find(Directory *d, const char *name); //查找文件                                               
void file_add(Directory *d, File *f);//添加文件                                                          
void file_remove(Directory *d, File *f);//移除文件                                                       
                                                                                                    
    /* 遍历（find_file / ll_pre / ll_post 共用） */                                                 
void walk_pre (Directory *cur, char *path, int len);   //先根遍历                              
void walk_post(Directory *cur, char *path, int len);   //后根遍历    
#endif
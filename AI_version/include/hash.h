#ifndef HASH_H
#define HASH_H

/* 链地址法哈希表：每个目录挂两张表（子目录表 + 文件表）。
 *
 * key 为节点名（定长拷贝，不持有外部指针），value 为 void* 泛型指针，
 * 表内不负责 value 的生命周期，由 fs 模块自行释放。
 */

#define HASH_SIZE  100   /* 桶数量 */
#define NAME_SIZE  20    /* 文件 / 目录名最大长度（含结尾 '\0'） */

typedef struct HashNode {
    char key[NAME_SIZE];
    void *value;
    struct HashNode *next;
} HashNode;

typedef struct HashTable {
    HashNode *table[HASH_SIZE];
} HashTable;

unsigned hash_key(const char *key);
void     hash_insert(HashTable *ht, const char *key, void *value);
void    *hash_find(HashTable *ht, const char *key);
int      hash_delete(HashTable *ht, const char *key);
void     hash_destroy(HashTable *ht);

#endif /* HASH_H */

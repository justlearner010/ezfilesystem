#ifndef HASH_H
#define HASH_H

#define HASH_SIZE 100
#define CONTENT_SIZE 1000
#define NAME_SIZE 20
typedef struct HashNode {
char key[NAME_SIZE];
void *value;
    struct HashNode *next;
} HashNode;
typedef struct HashTable {
    HashNode *table[HASH_SIZE];
} HashTable;


unsigned hash_key(const char *key);
void hash_insert(HashTable* ht, const char *key,void *value);
void *hash_find(HashTable *ht, const char *key);
int hash_delete(HashTable *ht, const char *key);
void hash_destroy(HashTable *ht);
#endif
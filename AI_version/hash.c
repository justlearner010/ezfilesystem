#include "hash.h"
#include <string.h>
#include <stdlib.h>
unsigned hash_key(const char *key){
    unsigned h = 0;
    for (int i = 0;key[i] != '\0'; i++){
        h += (unsigned char)key[i];
        
    }
    return h % HASH_SIZE;
}

void hash_insert(HashTable* ht, const char *key,void *value){
    unsigned bucket = hash_key(key);
    HashNode* node = malloc(sizeof(HashNode));
    node->value = value;
    strncpy(node->key, key, NAME_SIZE - 1);   // 拷贝内容，而不是存指针
    node->key[NAME_SIZE - 1] = '\0';
    // 链表的头插法
    node->next = ht->table[bucket];
    ht->table[bucket] = node;
    
}

void *hash_find(HashTable *ht, const char *key){
    unsigned bucket = hash_key(key);
    for (HashNode* p = ht->table[bucket]; p!= NULL;p = p->next){
        if (strcmp(p->key, key)== 0) return p->value;
    }
    return NULL;
}

int hash_delete(HashTable *ht, const char *key){
    unsigned bucket = hash_key(key);
    HashNode *cur = ht->table[bucket];
    HashNode *prev = NULL;

    while (cur != NULL){
        if (strcmp(cur->key, key)== 0)
            break;
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) return 0;

    if (prev == NULL){
        ht->table[bucket] = cur->next;// 删除的是头节点
    }else{
        prev->next = cur->next;
    }
    free(cur);
    return 1;
}

/* 只释放桶内链表节点，不释放卷表本身（卷表由 dir_destroy 负责 free） */
void hash_destroy(HashTable *ht){
    for (int i = 0;i < HASH_SIZE;i++){
        HashNode *p = ht->table[i];
        while (p) {
            HashNode *nxt = p->next;
            free(p);
            p = nxt;
        }
        ht->table[i] =NULL;
    }
    return;
}
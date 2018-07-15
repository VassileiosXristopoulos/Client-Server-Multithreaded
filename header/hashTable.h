#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "docKey.h"

typedef struct hash_entry hash_entry;
typedef struct hash_bucket hash_bucket;
typedef struct hash_table hash_table;


struct hash_entry{
	docKey* dockey;
	char *text;
	int matched;
	hash_entry* next;
};

struct hash_bucket{
	hash_entry* elements;
	int numOfEntries;
};

struct hash_table{
	int size;
	int entriesInBucket;
	hash_bucket **buckets;
};

hash_entry* hash_entry_Init(docKey*,char*);

hash_bucket* hash_bucket_Init();

hash_table* hash_table_Init(int,int);
hash_table* hash_table_Insert(docKey*,hash_table*,char *);
int hash(int,int);
void PrintAllHash(hash_table*);
hash_entry** getAllElements(hash_table *,int);
int getTotalElements(hash_table *);
void hash_table_Destroy(hash_table* );
void hash_entry_destroy(hash_entry*);
char * Hash_Search(char *, int ,hash_table* );
int Hash_GetAllLines(hash_table* );
int Hash_GetChars(hash_table* );
int Hash_GetWords(hash_table*);
#endif
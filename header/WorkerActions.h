#ifndef WORKERACTIONS_H
#define WORKERACTIONS_H
#include "hashTable.h"
#include "myFunctions.h"
void Worker_Search(int ,Trie_node *,char **,int ,char *,hash_table*);
char * answerForWord(Trie_node*,char*,int**,int,char**,int,hash_table*);
int CountPaths(char*);
void SaveToTrie(char *,docKey*,Trie_node*);
#endif

#ifndef TRIE_H
#define TRIE_H
#include <stdio.h>
#include <stdlib.h>
#include "postingList.h"

typedef struct Trie_node{
	int isleaf;
	struct Trie_node *sibling;
	struct Trie_node *child;
	char  value;
	int df;
	PostingList *pl_ptr;
}Trie_node;

void Trie_node_Init(Trie_node*);
void Trie_Insert(char*,docKey*,Trie_node*);
Trie_node * Trie_Search(char*,Trie_node*);
void Trie_Destroy(Trie_node*);
#endif
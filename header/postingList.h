#ifndef POSTINGLIST_H
#define POSTINGLIST_H
#include "docKey.h"
typedef struct PostingList PostingList;
struct PostingList{
	docKey *key;
	int word_frequency;
	PostingList *next;
};
int postingList_Update(docKey*,PostingList**);
void postingList_Print(PostingList**,char*);
void postingList_Destroy(PostingList*);
#endif

#ifndef DOCKEY_H
#define DOCKEY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct docKey docKey;
struct docKey{
	int* key;
	int size;
	char* filePath;
};
docKey *createKey(int ,char *);
void destroyDocKey(docKey*);
int AreSame(docKey *,docKey* );
docKey* addLine(int,docKey*);
#endif
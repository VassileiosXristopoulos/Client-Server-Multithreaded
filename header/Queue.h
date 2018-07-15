#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Queue Queue;

struct Queue{
	char *content;
	Queue *next;
};

void Queue_Print(Queue*);
Queue* Queue_Insert(Queue*,char*,int*);
Queue* newNode(char*);
void Queue_Destroy(Queue*);
#endif
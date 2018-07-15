#ifndef STACK_H
#define STACK_H
#include "myFunctions.h"
typedef struct Stack Stack;
struct Stack{
	char ** items;
	int top;
	int capacity;
};

Stack* Stack_CreateStack(int);
int Stack_isFull(Stack*);
void Stack_push(Stack*,char*);
int Stack_isEmpty(Stack*);
int Stack_pop(Stack*,char**);
void Stack_Destroy(Stack*);
char ** Stack_Peek(Stack*);
int Stack_GetSize(Stack*);
int inStack(char *,Stack *);
int CheckStack(Stack*);
#endif
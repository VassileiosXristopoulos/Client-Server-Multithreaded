#include "../header/Queue.h"


Queue* Queue_Insert(Queue* myQueue,char *text,int*data_size){
	Queue *iterator=myQueue;
	if(iterator==NULL){
	 myQueue=newNode(text);
	 *data_size+=strlen(text)+1;
	 return myQueue;
	}
	while(iterator->next!=NULL){
		if(strcmp(iterator->content,text)==0) return myQueue;
		iterator=iterator->next;
	}
	if(strcmp(iterator->content,text)==0) return myQueue;
	iterator->next=newNode(text);
	*data_size+=strlen(text)+1;
	return myQueue;
}

Queue * newNode(char *text){
	Queue *node=malloc(sizeof(Queue));
	node->content=malloc((strlen(text)+1)*sizeof(char));
	strcpy(node->content,text);
	node->next=NULL;
	return node;
}

void Queue_Print(Queue* myQueue){
	Queue *iterator=myQueue;
	printf("PRINTING QUEUE...\n");
	while(iterator!=NULL){
		printf("%s\n",iterator->content );
		iterator=iterator->next;
	}
}
void Queue_Destroy(Queue * myQueue){
	Queue *iterator=myQueue;
	Queue *temp;
	while(iterator!=NULL){
		temp=iterator->next;
		free(iterator->content);
		free(iterator);
		iterator=temp;
	}
	free(iterator);
}
#include <stdio.h>
#include <stdlib.h>
#include "../header/postingList.h"

int postingList_Update(docKey* mykey,PostingList** pl_ptr){
	PostingList* iterator;
	if((*pl_ptr)==NULL){ //list empty
		(*pl_ptr)=malloc(sizeof(PostingList));
		(*pl_ptr)->key=createKey(mykey->key[0],mykey->filePath);
		(*pl_ptr)->word_frequency=1;
		(*pl_ptr)->next=NULL;
		return 1;
	}
	else{ //list not empty, search for doc
		int found=0; 
		PostingList *tmp;
		PostingList* iterator= (*pl_ptr);
		while(iterator!=NULL){
			if(strcmp(mykey->filePath,iterator->key->filePath)==0){
				iterator->key=addLine(mykey->key[0],iterator->key);
				iterator->word_frequency++;
				found=1;
				break;
			}
			else{
				tmp=iterator;
				iterator=iterator->next;
			}
		}
		if(found==0){ //doesn't exist in list
			iterator= malloc(sizeof(PostingList));
			iterator->key=createKey(mykey->key[0],mykey->filePath);
			iterator->word_frequency=1;
			iterator->next=NULL;
			tmp->next=iterator;
			return 1; 
		}
	}
	return 0;
}

void postingList_Print(PostingList* *pl_ptr,char* word){
	PostingList *iterator=(*pl_ptr);
	while(iterator!=NULL){
		printf("[" );
		for(int i=0;i<iterator->key->size;i++) printf("%d ",iterator->key->key[i] );
		printf("%s,%d] \n",iterator->key->filePath,iterator->word_frequency );
		iterator=iterator->next;

	}
	printf("\n\n");
}


void postingList_Destroy(PostingList * pl_ptr){
	PostingList *iterator=pl_ptr;
	PostingList *temp;
	while(iterator!=NULL){
		temp=iterator->next;
		destroyDocKey(iterator->key);
		free(iterator);
		iterator=temp;
	}
	free(iterator);
}
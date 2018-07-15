#include "../header/hashTable.h"
#include "../header/myFunctions.h"

hash_entry* hash_entry_Init(docKey *mykey,char *content){
	hash_entry* newEntry=malloc(sizeof(hash_entry));
	newEntry->dockey=createKey(mykey->key[0],mykey->filePath);
	newEntry->text=malloc((strlen(content)+1)*sizeof(char));
	newEntry->matched=0;
	newEntry->next=NULL;
	strcpy(newEntry->text,content);
	return newEntry;
}

hash_bucket* hash_bucket_Init(){
	hash_bucket* mybucket=malloc(sizeof(hash_bucket));
	mybucket->numOfEntries=0;
	mybucket->elements=NULL;
	return mybucket;
}


hash_table* hash_table_Init(int size,int entries){
	hash_table* mytable=malloc(sizeof(hash_table));
	mytable->size=size;
	mytable->entriesInBucket=entries;
	mytable->buckets=malloc(size*sizeof(hash_bucket));
	for(int i=0;i<size;i++) mytable->buckets[i]=NULL;
	return mytable;
}

hash_table* hash_table_Insert(docKey* mykey,hash_table* hashTable,char *content){
	int position=hash(mykey->key[0],hashTable->size);
	if(hashTable->buckets[position]==NULL){
		hashTable->buckets[position]=hash_bucket_Init();
		hash_entry *newEntry=hash_entry_Init(mykey,content);
		hashTable->buckets[position]->elements=newEntry;
		hashTable->buckets[position]->numOfEntries++;
	}	
	else{
		if(hashTable->buckets[position]->numOfEntries==hashTable->entriesInBucket){ //no room for entry
			int size=getTotalElements(hashTable);
			hash_entry** allEntries=getAllElements(hashTable,size);

			int old_size=hashTable->size;
			int entriesPerBucket=hashTable->entriesInBucket;
			hash_table_Destroy(hashTable);
			hashTable=hash_table_Init(old_size*2,entriesPerBucket);

			for(int i=0;i<size;i++){
				hashTable=hash_table_Insert(allEntries[i]->dockey,hashTable,allEntries[i]->text);
			}
			hashTable=hash_table_Insert(mykey,hashTable,content);
			for(int i=0;i<size;i++) hash_entry_destroy(allEntries[i]);
			free(allEntries);
		}
		else{
			hashTable->buckets[position]->numOfEntries++;
			hash_entry *iterator=hashTable->buckets[position]->elements;
			while(iterator->next!=NULL) iterator=iterator->next;
			//iterator points at the last element of list
			hash_entry *newEntry=hash_entry_Init(mykey,content);
			iterator->next=newEntry; //inert at end of list

		}
	}
	return hashTable;
}


int hash(int key,int tableSize){
	return key % tableSize;
}

void PrintAllHash(hash_table *hashTable){
	for(int i=0;i<hashTable->size;i++){
		printf("             BUCKET %d           \n",i );
		if(hashTable->buckets[i]!=NULL){
			hash_entry *iterator=hashTable->buckets[i]->elements;
			while(iterator!=NULL){
				int id=iterator->dockey->key[0];
				char *path=iterator->dockey->filePath;
				char *content=iterator->text;
				printf("text-id:%d\n",id );
				iterator=iterator->next;
			}
		}
		else{
			printf("NULL\n");
		}
	}
}

hash_entry** getAllElements(hash_table *hashTable,int size){
	int count=0;
	hash_entry **allEntries=malloc(size*sizeof(hash_entry));
	for(int i=0;i<hashTable->size;i++){
		//printf("------------BUCKET %d --------------\n",i );
		if(hashTable->buckets[i]!=NULL){
			hash_entry *iterator=hashTable->buckets[i]->elements;
			while(iterator!=NULL){
				int id=iterator->dockey->key[0];
				char *path=iterator->dockey->filePath;
				docKey *newKey=createKey(id,path);
				char *content=iterator->text;
				hash_entry * newEntry=hash_entry_Init(newKey,content);
				allEntries[count++]=newEntry;
				destroyDocKey(newKey);
				iterator=iterator->next;
			}
		}
	}
	return allEntries;
}

int getTotalElements(hash_table *hashTable){
	int size=0;
	for(int i=0;i<hashTable->size;i++){
		if(hashTable->buckets[i]!=NULL)
			size+=hashTable->buckets[i]->numOfEntries;
	}
	return size;
}

void hash_table_Destroy(hash_table* hashTable){
	for(int i=0;i<hashTable->size;i++){
		if(hashTable->buckets[i]!=NULL){
			if(hashTable->buckets[i]->elements!=NULL)
				hash_entry_destroy(hashTable->buckets[i]->elements);
			free(hashTable->buckets[i]);
		}	
	}
	free(hashTable->buckets);
	free(hashTable);
}

void hash_entry_destroy(hash_entry* entry){
	hash_entry *iterator=entry;
	hash_entry *temp;
	while(iterator!=NULL){
		temp=iterator->next;
		destroyDocKey(iterator->dockey);
		free(iterator->text);
		free(iterator);
		iterator=temp;
	}
	free(iterator);
}


char * Hash_Search(char *path, int line,hash_table* hashTable){
	int buck=hash(line,hashTable->size);
	hash_bucket * ptr =hashTable->buckets[buck];
	if(ptr->elements!=NULL){
		hash_entry *iterator=ptr->elements;
		while(iterator!=NULL){
			if((iterator->dockey->key[0]==line)&&(strcmp(iterator->dockey->filePath,path))==0){
				if(iterator->matched==0){
					iterator->matched=1;
				}
				return iterator->text;
			}
		iterator=iterator->next;
		}
	}
	return NULL;
}

int Hash_GetAllLines(hash_table* hashTable){
	int ret=0;
	for(int i=0;i<hashTable->size;i++){
		if(hashTable->buckets[i]==NULL) continue;
		hash_bucket* bucket=hashTable->buckets[i];
		ret+=bucket->numOfEntries;
	}
	return ret;
}

int Hash_GetChars(hash_table* hashTable){
	int ret=0;
	for(int i=0;i<hashTable->size;i++){
		if(hashTable->buckets[i]==NULL) continue;
		hash_bucket* bucket=hashTable->buckets[i];
		hash_entry*iterator=bucket->elements;
		for(int j=0;j<bucket->numOfEntries;j++){
			ret+=strlen(iterator->text)+1;
			iterator=iterator->next;
		}
	}
	return ret-1;
}

int Hash_GetWords(hash_table* hashTable){
	int ret=0;
	for(int i=0;i<hashTable->size;i++){
		if(hashTable->buckets[i]==NULL) continue;
		hash_bucket* bucket=hashTable->buckets[i];
		hash_entry*iterator=bucket->elements;
		for(int j=0;j<bucket->numOfEntries;j++){
			char *text=iterator->text;
			int num=CountWords(iterator->text,strlen(iterator->text));
			ret+=num;
			iterator=iterator->next;
		}
	}
	return ret;
}
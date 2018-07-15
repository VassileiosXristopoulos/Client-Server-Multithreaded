
#include "../header/docKey.h"


docKey *createKey(int key,char *path){
	docKey *mykey=malloc(sizeof(docKey));
	mykey->size=1;
	mykey->key=malloc((mykey->size)*sizeof(int));
	mykey->key[0]=key;
	mykey->filePath=malloc((strlen(path)+1)*sizeof(char));
	strcpy(mykey->filePath,path);
	return mykey;
}

docKey * addLine(int line,docKey *oldKey){
	for(int i=0;i<oldKey->size;i++){
		if(oldKey->key[i]==line){ //line exists in postingList
			return oldKey;
		}
	}
	oldKey->size++;
	int *new=malloc((oldKey->size)*sizeof(int));
	for(int i=0;i<(oldKey->size-1);i++){
		new[i]=oldKey->key[i];
	}
	new[oldKey->size-1]=line;
	int  *temp =oldKey->key;
	oldKey->key=new;
	free(temp);
	return oldKey;
}
void destroyDocKey(docKey* mykey){
	free(mykey->filePath);
	free(mykey->key);
	free(mykey);
}


int AreSame(docKey *key1,docKey* key2){
	int ret=0;
	if(key1->key==key2->key){
		if(strcmp(key1->filePath,key2->filePath)==0) ret=1;
	}
	return ret;
}
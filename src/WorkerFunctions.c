#include "../header/WorkerActions.h"


void Worker_Search(int numOFfiles,Trie_node *Root,char **path_array,int fdSnd,char*recieved_content,hash_table* hashTable){
	int numOfWords=CountWords(recieved_content,strlen(recieved_content));

	char **query_words=malloc(numOfWords*sizeof(char*));
	query_words[0]=strtok(recieved_content," ");
	for(int k=1;k<numOfWords;k++){
		query_words[k]=strtok(NULL," ");
	}
	//pathsForWord is an int array with rows being indexes of query words 
	//and columns being files, so as to know in which files each word is found
	int **pathsForWord=malloc(numOfWords*sizeof(int*)); //-1 if word doesn't have path, 1 otherwise
														//indexes of array are the indexes of
														//query_words and path_array
	for(int i=0;i<numOfWords;i++){
		pathsForWord[i]=malloc(numOFfiles*sizeof(int));
		for(int k=0;k<numOFfiles;k++) pathsForWord[i][k]=-1;
	}
	
	char **answers_array=malloc(numOFfiles*sizeof(char*));
	for(int k=0;k<numOFfiles;k++){
		char *answer=answerForWord(Root,path_array[k],pathsForWord,k,query_words,numOfWords,hashTable);
		if(answer==NULL) {
			answers_array[k]=NULL;
			continue;
		}
		answers_array[k]=malloc((strlen(answer)+1)*sizeof(char));
		strcpy(answers_array[k],answer);
		free(answer);
	}

	int total_len=0;
	for(int i=0;i<numOFfiles;i++){
		if(answers_array[i]!=NULL) {
			total_len+=strlen(answers_array[i])+2;
		}
	}
	char* answer=malloc((total_len+1)*sizeof(char));

	int pos,flag=0;
	for(int i=0;i<numOFfiles;i++){
		if(answers_array[i]!=NULL){
			pos=i;
			flag=1;
			break;
		}
	}

	if(flag==0) {
		write(fdSnd,"-1",3);
	}
	else{
		strcpy(answer,answers_array[pos]);
		for(int i=pos+1;i<numOFfiles;i++){
			if(answers_array[i]!=NULL){
				strcat(answer,"\n");
				strcat(answer,answers_array[i]);
				strcat(answer,"\n");
			}
		}
		int bytes_written;
		int pipe_size=fpathconf(fdSnd, _PC_PIPE_BUF);

		if(( bytes_written=write(fdSnd,answer,strlen(answer)+1))<0)
			perror("write");
		while(bytes_written<strlen(answer)+1){
			str_cut(answer,bytes_written,strlen(answer));
			bytes_written=write(fdSnd,answer,strlen(answer)+1);
		}
	}
	free(answer);

	/*--------------------------------------------------*/

	for(int i=0;i<numOFfiles;i++){
		free(answers_array[i]);
	}
	free(query_words);
	free(answers_array);
	for(int i=0;i<numOfWords;i++){
		free(pathsForWord[i]);
	}
	free(pathsForWord);
}


char * answerForWord(Trie_node* Root,char *path,int** wordpath,int pathNum,char** query_words,int numOfWords,hash_table*hashTable){
	int data_size=0;
	Queue *myQueue=NULL;
	
	for(int i=0;i<numOfWords;i++){	
		Trie_node * mynode=Trie_Search(query_words[i],Root);
		if(mynode==NULL) continue;
		PostingList *iterator=mynode->pl_ptr;
		while(iterator!=NULL){
			if(strcmp(iterator->key->filePath,path)==0){
				wordpath[i][pathNum]=1;
				for(int k=0;k<iterator->key->size;k++){
				/*---Hash_Search returns the content of the line searched----*/
					char *contex=Hash_Search(path,iterator->key->key[k],hashTable);
					if(contex!=NULL){
						int entrySize=getDigits(iterator->key->key[k])+strlen(contex)+2;
						char *entry=malloc(entrySize*sizeof(char));
						char *key=malloc((getDigits(iterator->key->key[k])+1)*sizeof(char));
						sprintf(key,"%d",iterator->key->key[k]);
						strcpy(entry,key);
						strcat(entry," ");
						strcat(entry,contex);
						//insert to Queue
						myQueue=Queue_Insert(myQueue,entry,&data_size);
						free(key);
						free(entry);
					}
				}

				break;
			}
			iterator=iterator->next;
		}
	}

	char *answer=malloc((data_size+strlen(path)+1)*sizeof(char));
	strcpy(answer,path);
	Queue* iterator=myQueue;
	if(myQueue==NULL) return NULL;
	//pop from queue, merge,return.
	while(iterator!=NULL){
		strcat(answer,"\n");
		strcat(answer,iterator->content);
		iterator=iterator->next;
	}
	Queue_Destroy(myQueue);
	return answer;
}

int CountPaths(char *text){
	int count=0;
	for(int i=0;i<=strlen(text);i++){
		if(text[i]=='\n'|| text[i]=='\0'){
			if(text[i-1]!='\n'){
		 		count++;
	 		}
		}
	}
	return count;
}


void SaveToTrie(char *text,docKey *mykey,Trie_node *Root){
	int ctr=CountWords(text,strlen(text));
	char ** wordSep;
	int *sizes=malloc(ctr*sizeof(int));
	/* counting the length of each word */
	CountWordLength(text,sizes);
	/*allocating the array on which the words will be stored */
	wordSep=malloc(ctr*sizeof(char*));
	for(int i=0;i<ctr;i++){ 
		wordSep[i]=malloc((sizes[i]+1)*sizeof(char));
	}
	SaveWords(wordSep,text); //saving each word to wordSep
	for(int i=0;i<ctr;i++){
		if((strchr(wordSep[i],'>')==NULL)&&(strchr(wordSep[i],'<')==NULL)){
			Trie_Insert(wordSep[i],mykey,Root);
		}
	}
	for(int i=0;i<ctr;i++){

		free(wordSep[i]);
	}
		free(wordSep);
		free(sizes);
}
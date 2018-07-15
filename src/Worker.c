#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include "../header/docKey.h"
#include "../header/myFunctions.h"
#include "../header/hashTable.h"
#include "../header/WorkerActions.h"

int proceed=1;
static void handler(int signo){
	proceed=0;
}
int main(int argv,char *argc[]){
	sigset_t waitset;
    siginfo_t info;
    signal(SIGTERM, handler);

	sigemptyset( &waitset );
    sigaddset( &waitset, SIGCONT);
    sigprocmask( SIG_BLOCK, &waitset, NULL );
	int len,pipe_size=0;

	char *writePipe=argc[1];
	char *readPipe=argc[2];
	char* worker_num=argc[3];


	int fdRcv=open(readPipe,0666,O_RDONLY | O_NONBLOCK);
	int fdSnd=open(writePipe,0666,O_WRONLY | O_NONBLOCK);

	pipe_size=fpathconf(fdRcv, _PC_PIPE_BUF);
	char *recieved_content=malloc((pipe_size+1)*sizeof(char));
	int val;

	val=read(fdRcv,recieved_content,pipe_size);

	int pathsNum=CountPaths(recieved_content);
	char **directories=malloc(pathsNum*sizeof(char*));

	directories[0]=strtok(recieved_content,"\n");

	for(int i=1;i<pathsNum;i++){
		directories[i]=strtok(NULL,"\n");
	}

	fflush(stdout);
	
	DIR *dir;
	struct dirent *ent;
	Trie_node *Root; //Trie creation
	Root=malloc(sizeof(Trie_node));
	Trie_node_Init(Root);
	int numOFfiles=0;
	char **path_array;
	for(int j=0;j<pathsNum;j++){
		
		if ((dir = opendir(directories[j])) != NULL) {
				while((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".")!=0 && strcmp(ent->d_name, "..")!=0 ) {
						numOFfiles++;
					}
				}
		closedir(dir);
		}

	}
	hash_table * hashTable=hash_table_Init(1,numOFfiles);
	path_array=malloc(numOFfiles*sizeof(char*));
	int pos=0;
	for(int j=0;j<pathsNum;j++){
		
		if ((dir = opendir(directories[j])) != NULL) {
		  /* get all the files and directories within directory */
		  	while((ent = readdir(dir)) != NULL) {
				if (strcmp(ent->d_name, ".")!=0 && strcmp(ent->d_name, "..")!=0 ) {
					int pathSize=strlen(directories[j])+strlen(ent->d_name)+2;
	    			char *filePath=malloc(pathSize*sizeof(char));
	    			strcpy(filePath,directories[j]);
	    			strcat(filePath,"/");
	    			strcat(filePath,ent->d_name); //full path of file

	    			path_array[pos]=malloc((strlen(filePath)+1)*sizeof(char));
	    			strcpy(path_array[pos++],filePath);
	    			char *text; //text of each line
	    			char *content=readFile(filePath); //whole file

	    			int lines=getlines(filePath);
	    			
	    			int count=1;
	    			//create docKey
	    			/*docKey is the (line_id,filePath) pair for each LINE of a file*/
	    			text=strtok(content,"\n"); //separate file by lines
	    			if(text!=NULL){
		    			docKey *mykey=createKey(count++,filePath); // i is number of line
		    			hashTable=hash_table_Insert(mykey,hashTable,text);
		    			SaveToTrie(text,mykey,Root);
		    			destroyDocKey(mykey);
		    		}

	    			for(int i=1;i<lines;i++){
	    				text=strtok(NULL,"\n");
	    				if(text!=NULL){
		    				docKey *mykey=createKey(count++,filePath);
		    				hashTable=hash_table_Insert(mykey,hashTable,text);
		    				SaveToTrie(text,mykey,Root);
		    				destroyDocKey(mykey);
		    			}
	    			}
	    			free(content);
	    			free(filePath);
				}
		    	
		  	}
		  	closedir (dir);
		} 
		else {
		  /* could not open directory */
		  perror (directories[j]);
		  return EXIT_FAILURE;
		}

	}
	free(recieved_content);

	free(directories);
	char *answer;

 	int n=1;
 	printf("Worker %d saved all data %d !\n",getpid(),getppid());

	while(1){
		int result=sigwaitinfo( &waitset, &info );

		
		pipe_size=fpathconf(fdRcv, _PC_PIPE_BUF);
		recieved_content=malloc((pipe_size+1)*sizeof(char));
		read(fdRcv,recieved_content,pipe_size);

		if(strcmp(recieved_content,"/exit")==0){
			free(recieved_content);
			break;
		}

		Worker_Search(numOFfiles,Root,path_array,fdSnd,recieved_content,hashTable);			
		free(recieved_content);		
	}		
	printf("Worker %d out!\n",getpid());
	
	for(int i=0;i<numOFfiles;i++) free(path_array[i]);
	free(path_array);
	Trie_Destroy(Root);	
	hash_table_Destroy(hashTable);	
	close(fdRcv);
	close(fdSnd);
	exit(0);
}
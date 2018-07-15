#include "../header/myFunctions.h"


char* getFullTime(){
	time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time=asctime(tm);
    time[strlen(time)-1]=0;
    return time;
}
void perror_exit ( char * message ) {
	perror ( message ) ;
	exit ( 0 ) ;
}
int getDigits(int num){
	int _ret=1;
	if(num>0) _ret=floor(log10(abs(num)))+1;
	return _ret;
}
char *readFile(char *filename) {
    FILE *f = fopen(filename, "rt");
    if(f==NULL) return NULL;
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}
/*https://stackoverflow.com/questions/20342559/how-to-cut-part-of-a-string-in-c*/
int str_cut(char *str, int begin, int len){
    int l = strlen(str);
    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}
int CountWords(char *text,int len){
	int count=0;
	for(int i=0;i<=len;i++){
		if(text[i]==' '|| text[i]=='\0' || text[i]=='\t'){
				if(text[i-1]!=' ' && text[i-1]!='\0' && text[i-1]!='\t'){
			 		count++;
		 		}
		}
	}
	return count;
}

int isNum(char*word){
	char ptr;
	ptr=word[0];
	int _ret=1;
	for(int i=0;i<strlen(word);i++){
		if((ptr < '0' || ptr > '9')) _ret=0;
		ptr++;
	}
	return _ret;
}

void CountWordLength(char *text,int*sizes){
	/*source:
	http://www.includehelp.com/c-programs/read-a-string-and-print-the-length-of-the-each-word.aspx*/ 
	/*take a whole text, and count length of each word, so as to allocate the
	proper amount of space of each word*/
	int new_ctr=0,j=0;
	for(int i=0;i<=strlen(text);i++){
		if(text[i]==' '|| text[i]=='\0'){
			if(text[i-1]!=' ' && text[i-1]!='\0'){
				sizes[new_ctr++]=j;
				j=0; 
			}
		}
		else j++;
	}	
}
void SaveWords(char ** wordSep,char* doc){
	int i,j=0,ctr=0;
	/*source:
	https://www.w3resource.com/c-programming-exercises/string/c-string-exercise-31.php*/
//the technique i use is i read characters and define a word when 
//whitespace is found, therefore there is the danger of having
//consequtive whitespaces and allocating a word, hence the condition
	for(i=0;i<=strlen(doc);i++){
		if(doc[i]==' '|| doc[i]=='\0'){
			if(doc[i-1]!=' ' && doc[i-1]!='\0'){ //avoid allocating spaces as words
				wordSep[ctr++][j]='\0';
				j=0; //set index 0 for next word
			}
		}
		else{
			wordSep[ctr][j]=doc[i];
			j++;
		}
	}
}
struct hostent *resolveAddress(char *host){
	struct hostent *rem;
	if( (rem = gethostbyname(host)) == NULL)
		perror_exit("host");
	return rem;
}

int getSocketSize(int fd,int endpoint){ //endpoit is either receive or send
	//SO_SNDBUF for send buffer
	//SO_RCVBUF for receive buffer
	int n;
	unsigned int m = sizeof(int);
	getsockopt(fd,SOL_SOCKET,endpoint,(void *)&n, &m);
	return n;
}

char * ReadFromSocket(int fd){
	int buff_size=getSocketSize(fd,SO_RCVBUF);

	char * content=malloc((buff_size+1)*sizeof(char));
	int new_buff_size=buff_size;

	int val;
	if((val=read(fd,content,buff_size))<0)
		perror("read");

	content[val]='\0';
	while(val==buff_size){
		new_buff_size+=buff_size;
		char *new_content=malloc((new_buff_size+1)*sizeof(char));
		strcpy(new_content,content);
		new_content[new_buff_size]='\0';
		memset(content,0,strlen(content));
		val=read(fd,content,buff_size);
		content[buff_size]='\0';
		strcat(new_content,content);
		free(content);
		content=malloc((new_buff_size+1)*sizeof(char));
		strcpy(content,new_content);
		free(new_content);
	}
	return content;
}

void WriteCommandToSocket(int fd,char *response){
	int bytes_written=write(fd,response,strlen(response)+1);
	while(bytes_written<strlen(response)+1){
		str_cut(response,bytes_written,strlen(response));
		bytes_written=write(fd,response,strlen(response)+1);
	}
	close(fd);
}

void WriteToSocket(int fd,char *answer,char *permissions){
	char *response=makeMessage(answer,permissions);
	WriteCommandToSocket(fd,response);
	free(response);
	
}
Socket_response* setSocket(int port){
	int server_sock;
	struct sockaddr_in server; 
	struct sockaddr * serverPtr =( struct sockaddr *)&server ;
	if (( server_sock = socket ( AF_INET , SOCK_STREAM , IPPROTO_TCP) ) < 0)
		perror_exit ( " socket " ) ;
	server.sin_family = AF_INET ;
	server.sin_addr.s_addr = htonl(INADDR_ANY); 
	server.sin_port = htons ( port ) ;
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
    	perror_exit("setsockopt(SO_REUSEADDR) failed"); 

	if ( bind ( server_sock , serverPtr , sizeof ( server ) ) < 0)
		perror_exit ( " bind " ) ;
	if ( listen ( server_sock , 5) < 0) perror_exit ( " listen " ) ;
	Socket_response* response=malloc(sizeof(Socket_response));
	response->fd=server_sock;
	response->ptr=serverPtr;
	return response;
}

char *makeMessage(char *content,char *condition){
	char *lengthheader;
	char* httpHeader;
	char *response;
	char *date=getFullTime();
	int size=strlen(date)+strlen("Date: GMT")+1;
	char *dateheader=malloc((size+1)*sizeof(char*));
	sprintf(dateheader,"Date: %s GMT",date);
	char *serverheader="Server: myhttpd/1.0.0 (Ubuntu64)";
	char *typeheader="Content-Type: text/html";
	char *connectionheader="Connection: Closed";
	int okperms=0;
	if(content==NULL){
		httpHeader="HTTP/1.1 404 Not Found";
		response="<html>Sorry dude, couldn't find this file.</html>";
		size=strlen("Content-Length: ")+getDigits(strlen(response))+1;
		lengthheader=malloc((size+1)*sizeof(char*));
		sprintf(lengthheader,"Content-Length: %ld",strlen(response));
	}
	else if(strcmp(condition,"permissions_ok")==0){
		okperms=1;
		httpHeader="HTTP/1.1 200 OK";
		size=strlen("<html>")+strlen(content)+strlen("</html>")+10;
		response=malloc((size+1)*sizeof(char*));
		sprintf(response,"<html>%s</html>",content);
		size=strlen("Content-Length: ")+getDigits(strlen(response))+1;
		lengthheader=malloc((size+1)*sizeof(char*));
		sprintf(lengthheader,"Content-Length: %ld",strlen(response));
	}
	else if(strcmp(condition,"permissions_no")==0){
		httpHeader="HTTP/1.1 403 Forbidden";
		response="<html>Trying to access this file but I don't think I can make it.</html>"; 
		size=strlen("Content-Length: ")+getDigits(strlen(response))+1;
		lengthheader=malloc((size+1)*sizeof(char*));
		sprintf(lengthheader,"Content-Length: %ld",strlen(response));
	}
	int length=strlen(response)+strlen(httpHeader)+strlen(dateheader)+strlen(serverheader)+strlen(lengthheader)+strlen(typeheader)+strlen(connectionheader);
	char *message=malloc((length+9)*sizeof(char*));

	sprintf(message,"%s\n%s\n%s\n%s\n%s\n%s\n\n%s",httpHeader,dateheader,serverheader,lengthheader,typeheader,connectionheader,response);
	free(lengthheader);
	free(dateheader);
	if(okperms==1) free(response);
	return message;
}

void GetRequest(int* fds,struct sockaddr **adresses,socklen_t clientlen,int*response){
	int maxfd;
	if(fds[0]>fds[1]) maxfd=fds[0]+1;
	else maxfd=fds[1]+1;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fds[0],&readfds);
	FD_SET(fds[1],&readfds);
	int status;

		status=select(maxfd,&readfds,NULL,NULL,NULL);
		if(status>0){
			for(int i=0;i<2;i++){
				if (FD_ISSET(fds[i], &readfds)) {
					int fd_for_thread;
					fd_for_thread=accept(fds[i], adresses[i], &clientlen);
					if(fd_for_thread<0)
						perror("socket");
					response[i+1]=fd_for_thread;
					response[0]=i+1; //valid response
					return;
				}
			}
		
		}
		response[0]=-1;
} 

char * GetRequestContent(char *request){
	char *line=strtok(request,"\n"); //get first line
	char *ptr;
	ptr=line;
	while(ptr[0]!=' ') //eliminate GET
		ptr++;
	ptr=ptr+2; //eliminate the space and dash before content of request
	char *content=strtok(ptr," "); //eliminate text AFTER content of request
	char *response=malloc((strlen(content)+1)*sizeof(char));
	strcpy(response,content);
	//free(request);
	return response;
}

char *getElapsedTime(int elapsed){
	int days=0,hours,minutes,seconds;
	if(elapsed>86400){
		days=elapsed/86400;
		elapsed=elapsed%86400;
	}
	else days=0;

	hours=elapsed/3600;
	minutes=(elapsed%3600)/60;
	seconds=(elapsed%3600)%60;

	int digs=getDigits(hours);
	char * c_hours=malloc(3*sizeof(char));
	char * c_mins=malloc(3*sizeof(char));
	char * c_secs=malloc(3*sizeof(char));
	if(digs==1)
		sprintf(c_hours,"0%d",hours);
	else sprintf(c_hours,"%d",hours);
	digs=getDigits(minutes);
	if(digs==1)
		sprintf(c_mins,"0%d",minutes);
	else sprintf(c_mins,"%d",minutes);
	digs=getDigits(seconds);
	if(digs==1)
		sprintf(c_secs,"0%d",seconds);
	else sprintf(c_secs,"%d",seconds);

	char *myTime;
	if(days==0){
		myTime=malloc(12*sizeof(char));
		sprintf(myTime,"%s:%s:%s",c_hours,c_mins,c_secs);
	}
	else {
		char *c_days=malloc((getDigits(days)+1)*sizeof(char));
		sprintf(c_days,"%d",days);
		int size=12+getDigits(days)+strlen(" days");
		myTime=malloc(size*sizeof(char));
		sprintf(myTime,"%s days %s:%s:%s",c_days,c_hours,c_mins,c_secs);
		free(c_days);
	}
	free(c_secs);
	free(c_mins);
	free(c_hours);
	return myTime;
}

void WriteRequestToSocket(char *content,char *host,int port,int socket){
	int len=strlen("GET ")+strlen(" HTTP/1.1\nHost: ")+strlen(host)+getDigits(port)+strlen(content)+1;
	char *request=malloc((len+1)*sizeof(char));
	sprintf(request,"GET %s HTTP/1.1\nHost:%s %d\n",content,host,port);
	printf("%s\n",request );
	write(socket,request,strlen(request)+1);
	free(request);
}

int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    if(remove(pathname) < 0)
    {
        perror("ERROR: remove");
        return -1;
    }
    return 0;
}
void delete_all(char *directory){
	if (nftw(directory, rmFiles,10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0)
	    {
	        perror("ERROR: ntfw");
	        exit(1);
	    }
}


char *createExecutorFIle(char *save_dir){
	char *ret="JobExecutorFile.txt";
	FILE *f=fopen(ret,"w");
	DIR* dir;
	struct dirent *ent;
	if((dir=opendir(save_dir)) != NULL){
		while((ent=readdir(dir)) != NULL){
			if (strcmp(ent->d_name, ".")!=0 && strcmp(ent->d_name, "..")!=0 ) {
				fprintf(f, "%s/%s\n",save_dir,ent->d_name );
			}
		}
		fclose(f);
		closedir(dir);
	}
	else return NULL;
	return ret;
}

void GenerateWorkers(int Workers, char **fifo_array,char **fifo_array2){
	pid_t pid;
	pid_t temp_pid=fork();
	if(temp_pid!=0){ //   Job Executor
		for(int i=1;i<Workers;i++){
			pid=fork();
			if(pid==0){
				TriggerWorker(fifo_array[i],fifo_array2[i],i);
				exit(0);
			}
		}
	}
	else{
		TriggerWorker(fifo_array[0],fifo_array2[0],0);
		exit(0);
	}
}


/*set up the names for each pipe, ascending order*/
char ** makeNumberedArray(char *base,int num,int start){
	int digits;
	char **array=malloc(num*sizeof(char*));
	for( int i=0;i<num;i++) {
		digits=getDigits(i);
		array[i]=malloc((strlen(base)+digits+1)*sizeof(char));
	}
	for(int i=0;i<num;i++){
		digits=getDigits(i);
		sprintf(array[i], "%s%d",base, ++start);
	}
	return array;
}

char ** getContent(int* linesPerProcess,int Workers,char *content,int lines){
	char **retContent=malloc(Workers*sizeof(char*));
	char *copy_content=malloc((strlen(content)+1)*sizeof(char));
	strcpy(copy_content,content);
	for(int i=0;i<Workers;i++){
		char **temp_content=malloc(linesPerProcess[i]*sizeof(char*));
		temp_content[0]=strtok(content,"\n");

		for(int k=1;k<linesPerProcess[i];k++){
			temp_content[k]=strtok(NULL,"\n");
		}
		char *tmp;
		/*each line in temp_content..merge them*/
		int total_size=1;
		for(int k=0;k<linesPerProcess[i];k++) total_size+=strlen(temp_content[k])+1;
		retContent[i]=malloc((total_size+1)*sizeof(char));
		strcpy(retContent[i],temp_content[0]);

		for(int k=1;k<linesPerProcess[i];k++){
			strcat(retContent[i],"\n");
			strcat(retContent[i],temp_content[k]);
		}

		if(i<Workers-1){ 
			//i dont care for modification at last loop
			strcpy(content,copy_content);
			for(int k=0;k<linesPerProcess[i];k++){
				tmp=strchr(content,'\n');
				content=tmp;
	    		memmove(content, content+1, strlen(content));
			}
			memcpy(copy_content,content,strlen(content)+1);
			copy_content[strlen(content)]='\0';
		}

		free(temp_content);
	}

	free(copy_content);
	return retContent;

}


void TriggerWorker(char* Writepipe,char* ReadPipe,int num){
	char *number=malloc((getDigits(num)+1)*sizeof(char));
	sprintf(number,"%d",num);
	char *array[]={"./Worker",Writepipe,ReadPipe,number,NULL};
	int retval;
	if((retval=execvp(array[0],array))==-1)
		perror("exec");
}
int getlines(char* docName){
	/*source:
	https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer*/
	FILE* myfile = fopen(docName, "r");
	int ch, lines = 0;
	do{
	    ch = fgetc(myfile);
	    if(ch == '\n') lines++;
	}while (ch != EOF);
	fclose(myfile);
	return lines;
}

char *GetAnswer(int nfds,fd_set myset,int Workers,int *fdRcv){
	int retval;
	char **answers_array=malloc(Workers*sizeof(char*));
	for(int i=0;i<Workers;i++) answers_array[i]=NULL;
		struct timeval tv;
	tv.tv_sec=3;
	tv.tv_usec = 0;
	int answers=0;
	for(int j=0;j<Workers;j++){ //loop Worker-times to catch every event
		retval=select(nfds,&myset,NULL,NULL,&tv);
		if(retval>0){ // if no error is returned
			for(int i=0;i<Workers;i++){
				if(FD_ISSET(fdRcv[i],&myset)){
					int pipe_size=fpathconf(fdRcv[i], _PC_PIPE_BUF);
					int new_pipe_size=pipe_size;
					char *recieved_content=malloc((pipe_size+1)*sizeof(char));
					int r=read(fdRcv[i],recieved_content,pipe_size);
					recieved_content[pipe_size]='\0';
		/*------------------ manipulate the case of message bigger than pipe-size------------*/			
					while(r==pipe_size){
						new_pipe_size+=pipe_size;
						char* new_recieved=malloc((new_pipe_size+1)*sizeof(char));
						strcpy(new_recieved,recieved_content);
						new_recieved[new_pipe_size]='\0';
						memset(recieved_content,0,strlen(recieved_content));
						r=read(fdRcv[i],recieved_content,pipe_size);
						recieved_content[pipe_size]='\0';
						strcat(new_recieved,recieved_content);
						free(recieved_content);
						recieved_content=malloc((new_pipe_size+1)*sizeof(char));
						strcpy(recieved_content,new_recieved);
						free(new_recieved);
					}
					if(strcmp(recieved_content,"-1")!=0){
						answers_array[i]=malloc((strlen(recieved_content)+1)*sizeof(char));
						strcpy(answers_array[i],recieved_content);
					}
					free(recieved_content);
				}
			}
		}
	}
	int len=0;
	for(int i=0;i<Workers;i++){
		if(answers_array[i]!=NULL)
			len+=strlen(answers_array[i])+1;
	}
	char *final_answer=malloc((len+1)*sizeof(char));
	if(answers_array[0]!=NULL){
		strcpy(final_answer,answers_array[0]);
		free(answers_array[0]);
	}
	else{
		strcpy(final_answer,"");
	}
	for(int i=1;i<Workers;i++){
		if(answers_array[i]!=NULL){
			strcat(final_answer,"\n");
			strcat(final_answer,answers_array[i]);
			free(answers_array[i]);
		}
	}
	free(answers_array);
	return final_answer;
		
	
}


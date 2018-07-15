#include "../header/Crawler_Thread_Pool.h"

int main(int argc,char *argv[]){
	time_t crawler_start,crawler_end;
	time(&crawler_start);

	char *server_destination=argv[2];
	int server_port=atoi(argv[4]);
	int command_port=atoi(argv[6]);
	int num_of_threads=atoi(argv[8]);
	char *save_dir=argv[10];
	char *starting_url=argv[11];
	/*--------- PROJECT 2 INITIALIZATION ----------*/
	int lines,retval,Workers=4,nfds;
	char **fifo_array,**fifo_array2;
	int *fdRcv=malloc(Workers*sizeof(int));
    int * fdSnd=malloc(Workers*sizeof(int));
    fd_set myset;
    FD_ZERO(&myset);
	/*---------------------------------------------*/
	
	DIR* dir = opendir(save_dir);
	if(dir)
		delete_all(save_dir);
	closedir(dir);
	mkdir(save_dir,0700);
	struct sockaddr_in var;
	socklen_t socketLen =sizeof(var);
	Socket_response* socket_info=setSocket(command_port);
	int socket_fd=socket_info->fd;
	struct sockaddr * socketPtr=socket_info->ptr;

	fd_set readfds;
	int status,connection_fd;
	printf("STARTING URL: %s\n",starting_url );
	Crawler_Thread_Pool *ThreadPool=Crawler_Thread_Pool_Init(num_of_threads,server_destination,server_port,save_dir);
	Crawler_Thread_Pool_AddTask(ThreadPool,starting_url);

	pthread_mutex_lock(&(ThreadPool->have_finished));
		while(ThreadPool->threads_working==0)
			pthread_cond_wait(&(ThreadPool->crawl_started),&(ThreadPool->have_finished));
	pthread_mutex_unlock(&(ThreadPool->have_finished));

	int crawling_ended=0;
	while(1){
		pthread_mutex_lock(&(ThreadPool->have_finished));
	/*-------- INITIALIZATION OF PROJECT2 COMPONENTS -------*/
		if((ThreadPool->threads_working==0)&&(crawling_ended==0)){
			pthread_mutex_unlock(&(ThreadPool->have_finished));
			crawling_ended=1;
			//init project2
			remove("pipes");
			mkdir("pipes",0700);
			char *file=createExecutorFIle(save_dir);
			lines=getlines(file);
			if(file==NULL){
				printf("No site directories exist to give to Executor..\n");
				break;
			}
			fifo_array=makeNumberedArray("./pipes/f_",Workers,1);//Workers send-executor receives
			fifo_array2=makeNumberedArray("./pipes/fn_",Workers,1);//Executor sends-Workers receive
			
			for(int i=0;i<Workers;i++) mkfifo(fifo_array[i], 0666);
			for(int i=0;i<Workers;i++) mkfifo(fifo_array2[i], 0666);

			
			for(int i=0;i<Workers;i++) {
		    	fdSnd[i]=open(fifo_array[i],0666,O_WRONLY | O_NONBLOCK); //set up write-pipes
		    	fdRcv[i]=open(fifo_array2[i],0666,O_RDONLY | O_NONBLOCK); //set up recieve-pipes
		    	FD_SET(fdRcv[i],&myset);
		    }
		    int linesPerProcess=lines/Workers;
		    char *content=readFile(file);

			int length=strlen(content);
			content[length-1]='\0';
			/*-----calculate how many lines each Worker gets------*/
			int *linesPerProcess_array=malloc(Workers*sizeof(int));
			for(int k=0;k<Workers-1;k++){
				linesPerProcess_array[k]=linesPerProcess;

			}
			int difference=lines%linesPerProcess;
			if(difference==0) linesPerProcess_array[Workers-1]=linesPerProcess;
			else linesPerProcess_array[Workers-1]=linesPerProcess+difference;//give more to last worker

			char **contentPerProcess=getContent(linesPerProcess_array,Workers,content,lines);
			free(content);
			free(linesPerProcess_array);
			
			for(int i=0;i<Workers;i++){
				write(fdSnd[i],contentPerProcess[i],strlen(contentPerProcess[i])+1);
				free(contentPerProcess[i]);
			}
			free(contentPerProcess);
			nfds=fdRcv[Workers-1]+1; //biggest file desc we have plus one
			GenerateWorkers(Workers,fifo_array2,fifo_array);
		}
		else{
			pthread_mutex_unlock(&(ThreadPool->have_finished));
		}

		Crawler_check_threads(ThreadPool);
		FD_ZERO(&readfds);
		FD_SET(socket_fd,&readfds);
		struct timeval tv;
		tv.tv_sec=0.1;
		tv.tv_usec = 0;
		status=select(socket_fd+1,&readfds,NULL,NULL,&tv);
		if(status>0){
			pthread_mutex_lock(&(ThreadPool->have_finished));
			if(crawling_ended==0){
				connection_fd=accept(socket_fd,socketPtr,&socketLen);
				char *message="Crawling still in progress, please wait.\n";
				WriteToSocket(connection_fd,message,"permissions_ok");
			}
			else{ //CRAWRLING HAS ENDED, EXECUTE COMMANDS
				pthread_mutex_unlock(&(ThreadPool->have_finished));
				connection_fd=accept(socket_fd,socketPtr,&socketLen);
				if(connection_fd<0) perror("socket");
				char *command=ReadFromSocket(connection_fd);
				command[strlen(command)]='\0';
				fflush(stdout);

				 if(strcmp(command,"STATS")==0){
					time(&crawler_end);
					int elapsed=(int)(crawler_end-crawler_start); //round down to seconds
					char *myTime=getElapsedTime(elapsed);

					pthread_mutex_lock(&(ThreadPool->TaskQueue->rw_mutex));
					int pages_served=ThreadPool->TaskQueue->pages_served;
					int bytes_served=ThreadPool->TaskQueue->bytes_served;
					pthread_mutex_unlock(&(ThreadPool->TaskQueue->rw_mutex));

					int size=getDigits(pages_served)+getDigits(bytes_served)+3+strlen("Crawler up for ,served pages,bytes")+strlen(myTime)+1;
					char *result=malloc((size+1)*sizeof(char));
					sprintf(result,"Crawler up for %s, served %d pages,%d bytes.\n",myTime,pages_served,bytes_served);

					WriteCommandToSocket(connection_fd,result);
					free(result);
					free(myTime);
					free(command);
				}
				else if(strcmp(command,"SHUTDOWN")==0){
					WriteCommandToSocket(connection_fd,"System shutting down.\n");
					free(command);
					break;
				}
				else{
					char *copy=malloc((strlen(command))+1*sizeof(char));
					strcpy(copy,command);
					char *str=strtok(command," ");
					printf("%s\n", str);
					if(strcmp(str,"SEARCH")==0){//search
						//text is SEARCH_word1_word2_word3 in variable
						char *query=strchr(copy,' ');
						memmove(query,query+1,strlen(query));
						printf("%s\n",query );

						for(int i=0;i<Workers;i++)
							write(fdSnd[i],query,strlen(query)+1);
						kill(0,SIGCONT); //wake up all workers
						char *answer=GetAnswer(nfds,myset,Workers,fdRcv);
						WriteCommandToSocket(connection_fd,answer);
						free(answer);						
					}
					free(copy);
					free(command);
				}
			}
		}
		FD_ZERO(&readfds);
		FD_SET(socket_fd,&readfds);
	}
	ThreadPool->alive=0;
	//singal all threads in order to finish
	lock_signal_all(ThreadPool->TaskQueue->m_lock,num_of_threads);
	/*--------wait all threads to finish------------*/
	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	while(ThreadPool->initialized>0)
		pthread_cond_wait(&(ThreadPool->cond_initialized),&(ThreadPool->thread_ctrl));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));

	Crawler_Thread_Pool_Destroy(ThreadPool);
	int ret=close(socket_fd);

	for(int i=0;i<Workers;i++)
		write(fdSnd[i],"/exit",strlen("/exit")+1);
	kill(0,SIGCONT); //wake up all Workers

	waitpid(-1,NULL,0);

	free(socket_info);
	for(int i=0;i<Workers;i++) {
    	close(fdRcv[i]); 
    	close(fdSnd[i]);
    }
	free(fdRcv);
	free(fdSnd);
	for(int i=0;i<Workers;i++){
		free(fifo_array[i]);
		free(fifo_array2[i]);
	}

	free(fifo_array);
	free(fifo_array2);
	return 0;
	
}
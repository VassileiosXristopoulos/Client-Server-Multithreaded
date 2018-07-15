
#include "../header/myFunctions.h"
#include "../header/Thread_Pool.h"
#include "../header/Lock.h"
#include "../header/Task_Queue.h"
int main(int argc,char*argv[]){
	time_t server_start,server_end;
	time(&server_start);
	if(argc!=9){
		perror_exit("Wrong arguments\n");
		exit(0);
	}
	int serving_port=atoi(argv[2]);
	int command_port=atoi(argv[4]);
	int ports[2]={command_port,serving_port};
	int num_of_threads=atoi(argv[6]);
	char *root_dir=argv[8];

    struct sockaddr_in var;
	socklen_t clientlen =sizeof(var);
	Socket_response* serving_info=setSocket(ports[1]);
	int serving_sock=serving_info->fd;
	struct sockaddr * servingPtr=serving_info->ptr;

	Socket_response* command_info=setSocket(ports[0]);
	int command_sock=command_info->fd;
	struct sockaddr * commandPtr=command_info->ptr;

	int fd_for_thread;
	int fds[2]={command_sock,serving_sock};

	char *content,*mresponse;
	struct sockaddr* adresses[2]={commandPtr,servingPtr};
	int*response=malloc(3*sizeof(int));
	for(int i=0;i<3;i++) response[i]=0;
	Thread_Pool * ThreadPool= Thread_Pool_Init(num_of_threads,root_dir);
	while(1){
		check_threads(ThreadPool);
		GetRequest(fds,adresses,clientlen,response);
		int pos=response[0];
/*response 0 returns the position of the fd in the response array - aka the port*/
		if(pos>0){ 
			fd_for_thread=response[pos];

			char *message=ReadFromSocket(fd_for_thread);//read the whole request
			message[strlen(message)]='\0';
		
			if(pos==1){ // command port				
				if(strcmp(message,"SHUTDOWN")==0){ 
				/*----------- wait until no thread is operating -------*/
					pthread_mutex_lock(&(ThreadPool->have_finished));
					while(ThreadPool->threads_working>0){ 
						pthread_cond_wait(&(ThreadPool->all_idle),&(ThreadPool->have_finished));
					}
					pthread_mutex_unlock(&(ThreadPool->have_finished));
				/*----------------------------------------------------*/
					free(message);
					break;
				}
				else if(strcmp(message,"STATS")==0){
					time(&server_end);
					int elapsed=(int)(server_end-server_start); //round down to seconds
					char *myTime=getElapsedTime(elapsed); 

					pthread_mutex_lock(&(ThreadPool->TaskQueue->rw_mutex));
					int pages_served=ThreadPool->TaskQueue->pages_served;
					int bytes_served=ThreadPool->TaskQueue->bytes_served;
					pthread_mutex_unlock(&(ThreadPool->TaskQueue->rw_mutex));
					int size=getDigits(pages_served)+getDigits(bytes_served)+3+strlen("Server up for ,served pages,bytes")+strlen(myTime)+1;
					char *result=malloc((size+1)*sizeof(char));
					sprintf(result,"Server up for %s, served %d pages,%d bytes\n",myTime,pages_served,bytes_served);

					WriteCommandToSocket(fd_for_thread,result);
					free(result);
					free(myTime);
				}
				else{
					free(message);
					perror_exit("Wrong command!!");
				}
			}
			else if(pos==2){ // serving port
				printf("REACHED THIS PART..\n");
				char *mymessage=malloc((strlen(message)+1)*sizeof(char));
				strcpy(mymessage,message);

				content=GetRequestContent(mymessage);//distinguish the command
				if(content==NULL) printf("NULLL ILITHIEEEEE\n");
				Thread_Pool_AddTask(ThreadPool, fd_for_thread,content);
				free(content);
			//	free(mymessage);
			}
			else{
				free(message);
				perror_exit("Error on getting request");
			}
			free(message);
		}
	}

	ThreadPool->alive=0; //break the infinite loop of the threads
	lock_signal_all(ThreadPool->TaskQueue->m_lock,num_of_threads);//unlock them all
	mresponse="System shutting down..\n";
	WriteCommandToSocket(fd_for_thread,mresponse);

	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	while(ThreadPool->initialized>0)
		pthread_cond_wait(&(ThreadPool->all_initialized),&(ThreadPool->thread_ctrl));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));

	Thread_Pool_Destroy(ThreadPool);
	free(response);
	free(command_info);
	free(serving_info);
	return 0;
}





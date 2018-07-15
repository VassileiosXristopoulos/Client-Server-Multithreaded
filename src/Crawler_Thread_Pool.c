#include "../header/Crawler_Thread_Pool.h"
#define h_addr h_addr_list[0] /* for backward compatibility */
int thread_pos;
Connection_info * Connection_Info_Init(int port,char *host){
	Connection_info* myConnection=malloc(sizeof(Connection_info));
	myConnection->socketPtr=(struct sockaddr *)&myConnection->server;
	myConnection->size=sizeof(myConnection->server);
	
	if((myConnection->socket= socket(AF_INET , SOCK_STREAM , 0))<0)
		perror_exit("socket");

	struct hostent *rem;
	if( (rem = gethostbyname(host)) == NULL)
		perror_exit("host");

	myConnection->server.sin_family=AF_INET;
	memcpy(&myConnection->server.sin_addr,rem->h_addr,rem->h_length);
	myConnection->server.sin_port=htons(port);

	return myConnection;
}

Crawler_Thread_Pool* Crawler_Thread_Pool_Init(int threads,char*host,int port,char *save_dir){
	Crawler_Thread_Pool* ThreadPool=malloc(sizeof(Crawler_Thread_Pool));
	ThreadPool->port=port;
	ThreadPool->host=malloc((strlen(host)+1)*sizeof(char));
	strcpy(ThreadPool->host,host);
	ThreadPool->save_dir=malloc((strlen(save_dir)+1)*sizeof(char));
	strcpy(ThreadPool->save_dir,save_dir);
	ThreadPool->ConnectionInfo=Connection_Info_Init(port,host);
	ThreadPool->TaskQueue =Crawler_Task_Queue_Init(save_dir);
	ThreadPool->threads=threads;

	pthread_mutex_init(&(ThreadPool->socket_ctrl),NULL);
	/*---------------- for crawler to know when no one is doing work-----------*/

	ThreadPool->threads_working=0;  
	pthread_mutex_init(&(ThreadPool->have_finished),NULL);
	pthread_cond_init(&(ThreadPool->all_idle),NULL);
	pthread_cond_init(&(ThreadPool->crawl_started),NULL);
	
	ThreadPool->alive=1;

/*------------- for crawler to know when threads initialized------------*/
	pthread_mutex_init(&(ThreadPool->thread_ctrl),NULL);
	pthread_cond_init(&(ThreadPool->cond_initialized),NULL);
	ThreadPool->initialized=0;
/*-------------------------------------------------------------------------*/
	int err;
	
	ThreadPool->thread_array=malloc(threads*sizeof(pthread_t));
	thread_pos=0;// the first thread exits on it's own in order to create new
	for(int i=0;i<threads;i++){
		pthread_create(&ThreadPool->thread_array[i],NULL,Thread_Crawlers,(void*)&(ThreadPool));
		pthread_detach(ThreadPool->thread_array[i]);
	}

/*----------------------wait all threads to initialize---------------------*/

	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	while(ThreadPool->initialized<ThreadPool->threads)
		pthread_cond_wait(&(ThreadPool->cond_initialized),&(ThreadPool->thread_ctrl));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));

/*-------------------------------------------------------------------------*/

	return ThreadPool;
}

void Crawler_Thread_Pool_AddTask(Crawler_Thread_Pool* ThreadPool,char *file){
	Crawler_Task_Queue_AddTask(ThreadPool->TaskQueue,ThreadPool->ConnectionInfo->socket,file);
}

void * Thread_Crawlers(void * arg){
	Crawler_Thread_Pool *ThreadPool=*(Crawler_Thread_Pool**)(arg);
	Crawler_Task_Queue *MyTaskQueue=ThreadPool->TaskQueue;

	struct sockaddr *socketPtr=ThreadPool->ConnectionInfo->socketPtr;
	struct sockaddr_in server=ThreadPool->ConnectionInfo->server;
	/*------------- updating how many threads have been initialized-------*/
	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	ThreadPool->initialized++;

	if(ThreadPool->initialized==ThreadPool->threads)
		pthread_cond_signal(&(ThreadPool->cond_initialized));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));
/*--------------------------------------------------------------------*/
	int chosen_to_death=0;
	if(ThreadPool->thread_array[thread_pos]==pthread_self())
		chosen_to_death=1;
	double deadline=5.0;
	time_t start,end;
	time(&start);
	printf("Thread1 %ld created--chosen to death=%d\n",pthread_self(),chosen_to_death);

	lock_wait(MyTaskQueue->m_lock);// waiting to wake up
	while(ThreadPool->alive){ // I have task to execute
		
/*------------ update how many threads are working -------------------*/
		pthread_mutex_lock(&(ThreadPool->have_finished));
		ThreadPool->threads_working++;
		if(ThreadPool->threads_working==1)pthread_cond_signal(&(ThreadPool->crawl_started));
		pthread_mutex_unlock(&(ThreadPool->have_finished));
/*--------------------------------------------------------------------*/


	/*------------------- getting the work done --------------------------*/
		printf("going to get..\n");
		Task *myTask=Crawler_Task_Queue_GetTask(MyTaskQueue);
		printf("got..\n");
		pthread_mutex_lock(&(ThreadPool->socket_ctrl));
		int sock;
		if(( sock =  socket(AF_INET , SOCK_STREAM , 0))<0){
			perror_exit("socket");
		}
		while(connect(sock,socketPtr,sizeof(server))<0);

		char *name=strchr(myTask->fileName,'/');
		printf("goint to write--->%s\n",name );
		WriteRequestToSocket(name,ThreadPool->host,ThreadPool->port,sock);
		char *response=ReadFromSocket(sock);
		pthread_mutex_unlock(&(ThreadPool->socket_ctrl));
		printf("here -- > %s\n",name );
		int len=strlen(name)+strlen(ThreadPool->save_dir)+2;
		char *path=malloc(len*sizeof(char));
		sprintf(path,"%s%s",ThreadPool->save_dir,name);
		printf("path--->%s\n",path );
		FILE* f=fopen(path,"w");
		fprintf(f,"%s",response);
		fclose(f);
		printf("!!!!!!!!!%s\n",response );

		pthread_mutex_lock(&(MyTaskQueue->rw_mutex));
		MyTaskQueue->pages_served++;
		MyTaskQueue->bytes_served+=strlen(response)+1;
		pthread_mutex_unlock(&(MyTaskQueue->rw_mutex));
	
		response[strlen(response)]='\0';
		char *response_copy=malloc((strlen(response)+1)*sizeof(char));
		strcpy(response_copy,response);

		response_copy[strlen(response)]='\0';
		char *rest,*nextToken;
		rest=response_copy;
		//printf("rest----????>>>%s\n",rest );
		while((nextToken=strtok_r(rest," ",&rest))!=NULL){
			char *ending=strchr(nextToken,'.');
			if(ending!=NULL){
				if(strcmp(ending,".html")==0){
					Crawler_Thread_Pool_AddTask(ThreadPool,nextToken);
				}
			}
		}
		free(response);
		free(response_copy);
		free(path);
		free(myTask->fileName);
		free(myTask);
	/*--------------------------------------------------------------------*/
	
	/*-------------- updating how many threads are working ---------------*/
		pthread_mutex_lock(&(ThreadPool->have_finished));
		printf("minus minus\n");
		ThreadPool->threads_working--;
		pthread_mutex_unlock(&(ThreadPool->have_finished));
	/*--------------------------------------------------------------------*/
		
			if(chosen_to_death==1){
			time(&end);
			if(end-start>=5) break;
		}

		lock_wait(MyTaskQueue->m_lock);
	}

	for(int i=0;i<ThreadPool->threads;i++){
		if(ThreadPool->thread_array[i]==pthread_self()){
			ThreadPool->thread_array[i]=0;
		}
	}
	printf("Thread %ld exiting\n",pthread_self() );


	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	ThreadPool->initialized--;
	if(ThreadPool->initialized==0)pthread_cond_signal(&(ThreadPool->cond_initialized));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));
}

void Crawler_Thread_Pool_Destroy(Crawler_Thread_Pool* ThreadPool){
	free(ThreadPool->thread_array);
	free(ThreadPool->host);
	free(ThreadPool->save_dir);
	free(ThreadPool->ConnectionInfo);
	Crawler_Task_Queue_Destroy(ThreadPool->TaskQueue);
	free(ThreadPool);
}


void Crawler_check_threads(Crawler_Thread_Pool* ThreadPool){
	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	if(ThreadPool->initialized<ThreadPool->threads){
		for(int i=0;i<ThreadPool->threads;i++){
			if(ThreadPool->thread_array[i]==0){
				pthread_create(&ThreadPool->thread_array[i],NULL,Thread_Crawlers,(void*)&(ThreadPool));
				pthread_detach(ThreadPool->thread_array[i]);
			}
		}
	/*wait for all threads to be initialized before continuing*/
		while(ThreadPool->initialized<ThreadPool->threads)
			pthread_cond_wait(&(ThreadPool->cond_initialized),&(ThreadPool->thread_ctrl));
	}
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));
}
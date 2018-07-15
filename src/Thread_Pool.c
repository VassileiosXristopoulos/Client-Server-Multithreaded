#include "../header/Thread_Pool.h"
#include "../header/Lock.h"
#include "../header/Task_Queue.h"
int thread_pos;
Thread_Pool* Thread_Pool_Init(int threads,char* root_dir){

	Thread_Pool * myPool=malloc(sizeof(Thread_Pool));
	if(myPool==NULL) perror_exit("Cannot allocate Thread Pool");
	myPool->TaskQueue =Task_Queue_Init(root_dir);
	myPool->threads=threads;

/*---------------- for server to know when no one is doing work-----------*/

	myPool->threads_working=0;  
	pthread_mutex_init(&(myPool->have_finished),NULL);
	pthread_cond_init(&(myPool->all_idle),NULL);
	myPool->alive=1;

/*------------- for server to know when all threads initialized------------*/
	pthread_mutex_init(&(myPool->thread_ctrl),NULL);
	pthread_cond_init(&(myPool->all_initialized),NULL);
	myPool->initialized=0;
/*-------------------------------------------------------------------------*/
	int err;
	myPool->thread_array=malloc(threads*sizeof(pthread_t));
	thread_pos=0; // the first thread exits on it's own in order to create new
	for(int i=0;i<threads;i++){
		pthread_create(&myPool->thread_array[i],NULL,Thread_Operation,(void*)&(myPool));
		pthread_detach(myPool->thread_array[i]);
	}

/*----------------------wait all threads to initialize---------------------*/

	pthread_mutex_lock(&(myPool->thread_ctrl));
	while(myPool->initialized<myPool->threads)
		pthread_cond_wait(&(myPool->all_initialized),&(myPool->thread_ctrl));
	pthread_mutex_unlock(&(myPool->thread_ctrl));

/*-------------------------------------------------------------------------*/
	return myPool;
}

void Thread_Pool_AddTask(Thread_Pool*myPool,int fd,char *command){
	Task_Queue_AddTask(myPool->TaskQueue,fd,command);
}


void *Thread_Operation(void * Pool){
	Thread_Pool *ThreadPool=*(Thread_Pool**)(Pool);
	Task_Queue *MyTaskQueue=ThreadPool->TaskQueue;
/*------------- updating how many threads have been initialized-------*/
	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	ThreadPool->initialized++;
	if(ThreadPool->initialized==ThreadPool->threads)
		pthread_cond_signal(&(ThreadPool->all_initialized));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));
/*--------------------------------------------------------------------*/
	int chosen_to_death=0;
	if(ThreadPool->thread_array[thread_pos]==pthread_self())
		chosen_to_death=1;
	double deadline=5.0;
	time_t start,end;
	time(&start);
	printf("Thread %ld created--chosen to death=%d\n",pthread_self(),chosen_to_death);
	lock_wait(MyTaskQueue->m_lock);// waiting to wake up

	while(ThreadPool->alive){ // I have task to execute
/*------------ update how many threads are working -------------------*/
		pthread_mutex_lock(&(ThreadPool->have_finished));
		ThreadPool->threads_working++;
		pthread_mutex_unlock(&(ThreadPool->have_finished));
/*--------------------------------------------------------------------*/

/*------------------- getting the work done --------------------------*/
		Task* myTask=Task_Queue_GetTask(MyTaskQueue);
		int descriptor=myTask->descriptor;
		printf("%s\n",myTask->fileName );
		char * content = readFile(myTask->fileName);
		if(content==NULL) WriteToSocket(descriptor,NULL,"");
		WriteToSocket(descriptor,content,"permissions_ok");
		
		if(content!=NULL) {
			pthread_mutex_lock(&(ThreadPool->TaskQueue->rw_mutex));
			ThreadPool->TaskQueue->pages_served++;
			ThreadPool->TaskQueue->bytes_served+=strlen(content)+1;
			pthread_mutex_unlock(&(ThreadPool->TaskQueue->rw_mutex));
		}

		free(myTask->fileName);
		free(myTask);
		if(content!=NULL) free(content);
/*--------------------------------------------------------------------*/

/*-------------- updating how many threads are working ---------------*/
		pthread_mutex_lock(&(ThreadPool->have_finished));
		ThreadPool->threads_working--;
		//if no more threads are working, let server know, he may be wanting to shutdown
		if(ThreadPool->threads_working==0) pthread_cond_signal(&(ThreadPool->all_idle));
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

	printf("Thread %ld exiting..\n",pthread_self());
	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	ThreadPool->initialized--;
	if(ThreadPool->initialized==0)pthread_cond_signal(&(ThreadPool->all_initialized));
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));
	
}

void Thread_Pool_Destroy(Thread_Pool *ThreadPool){
	free(ThreadPool->thread_array);
	Task_Queue_Destroy(ThreadPool->TaskQueue);
	free(ThreadPool);
}

void check_threads(Thread_Pool* ThreadPool){
	pthread_mutex_lock(&(ThreadPool->thread_ctrl));
	if(ThreadPool->initialized<ThreadPool->threads){
		for(int i=0;i<ThreadPool->threads;i++){
			if(ThreadPool->thread_array[i]==0){
				pthread_create(&ThreadPool->thread_array[i],NULL,Thread_Operation,(void*)&(ThreadPool));
				pthread_detach(ThreadPool->thread_array[i]);
				printf("thread reloaded..\n");
			}
		}
		while(ThreadPool->initialized<ThreadPool->threads)
			pthread_cond_wait(&(ThreadPool->all_initialized),&(ThreadPool->thread_ctrl));
	}
	pthread_mutex_unlock(&(ThreadPool->thread_ctrl));
}


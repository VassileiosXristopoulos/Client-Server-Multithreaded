#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "myFunctions.h"
#include "Task_Queue.h"

# define perror2 (s , e ) fprintf ( stderr , "%s:%s\n",s,strerror ( e ) );

typedef struct Thread_Pool Thread_Pool;
struct Thread_Pool{
	int threads;
	Task_Queue * TaskQueue;
	int threads_working;
	pthread_t *thread_array;
	pthread_cond_t all_idle;
	pthread_mutex_t have_finished;
	int alive;
	pthread_cond_t all_initialized;
	pthread_mutex_t thread_ctrl;
	int initialized;
};

Thread_Pool* Thread_Pool_Init(int,char*);
void Thread_Pool_AddTask(Thread_Pool*,int,char*);
void *Thread_Operation(void*);
void Thread_Pool_Destroy(Thread_Pool *);
void check_threads(Thread_Pool* );
#endif
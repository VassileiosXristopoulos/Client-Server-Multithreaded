#ifndef CRAWLER_THREAD_POOL_H
#define CRAWLER_THREAD_POOL_H
#include "Task_Queue.h"
#include "Crawler_Task_Queue.h"
#include "Lock.h"
# define perror2 (s , e ) fprintf ( stderr , "%s:%s\n",s,strerror ( e ) );

typedef struct Connection_info Connection_info;
struct Connection_info{
	int socket;
	struct sockaddr *socketPtr;
	struct sockaddr_in server;
	socklen_t size;
};


typedef struct Crawler_Thread_Pool Crawler_Thread_Pool;
struct Crawler_Thread_Pool{
	int threads;
	int port;
	char *host;
	char *save_dir;
	pthread_mutex_t MapInfo_mutex;
	Crawler_Task_Queue * TaskQueue;
	int threads_working;
	pthread_t *thread_array;
	pthread_cond_t all_idle;
	pthread_cond_t crawl_started;
	pthread_mutex_t have_finished;
	int alive;
	pthread_cond_t cond_initialized;
	pthread_mutex_t thread_ctrl;
	int initialized;
	Connection_info* ConnectionInfo;
	pthread_mutex_t socket_ctrl;
};

Connection_info* Connection_info_Init(int,int,char*);
Crawler_Thread_Pool* Crawler_Thread_Pool_Init(int,char *,int,char*);
void * Thread_Crawlers(void * );
void Crawler_Thread_Pool_AddTask(Crawler_Thread_Pool*,char*);
void Crawler_Thread_Pool_Destroy(Crawler_Thread_Pool* );
void Crawler_check_threads(Crawler_Thread_Pool* );
#endif

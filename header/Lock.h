#ifndef LOCK_H
#define LOCK_H
#include "myFunctions.h"
typedef struct lock lock;
struct lock{
	pthread_cond_t cond;
	pthread_mutex_t has_tasks;
	int numOfTasks;
};
lock *lock_init();
void lock_wait(lock *);
void lock_signal(lock *);
void lock_signal_all(lock *,int);
void lock_free(lock* );
#endif
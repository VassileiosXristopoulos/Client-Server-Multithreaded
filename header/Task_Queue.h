#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H
#include "myFunctions.h"
#include "Lock.h"
typedef struct Task_Queue Task_Queue;
typedef struct Task Task;
struct Task_Queue{
	int numOfTasks;
	int pages_served;
	int bytes_served;
	Task * front;
	Task * end;
	lock *m_lock;
	char *root_dir;
	pthread_mutex_t rw_mutex;
};
struct Task{
	int descriptor;
	char *fileName;
	Task *next;
};


Task_Queue* Task_Queue_Init(char*);
void Task_Queue_AddTask(Task_Queue*,int,char*);
Task* Task_Queue_GetTask(Task_Queue *);
void Task_Queue_Destroy(Task_Queue* );
#endif
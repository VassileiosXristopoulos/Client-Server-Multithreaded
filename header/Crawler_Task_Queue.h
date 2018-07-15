#ifndef CRAWLER_TASK_QUEUE_H
#define CRAWLER_TASK_QUEUE_H
#include "Task_Queue.h"

typedef struct MapInfo MapInfo;
struct MapInfo{
	char *fileName;
	MapInfo* next;
};
typedef struct Map Map;
struct Map{
	MapInfo* front;
	MapInfo* end;
	int length;
};
typedef struct Crawler_Task_Queue Crawler_Task_Queue;
struct Crawler_Task_Queue{
	int numOfTasks;
	int pages_served;
	int bytes_served;
	Task * front;
	Task * end;
	lock *m_lock;
	char *root_dir;
	Map *myMap;
	pthread_mutex_t rw_mutex;
};

Crawler_Task_Queue* Crawler_Task_Queue_Init(char*);
int notInMap(Map *,char *);
void Crawler_Task_Queue_AddTask(Crawler_Task_Queue* ,int ,char *);
Task* Crawler_Task_Queue_GetTask(Crawler_Task_Queue * );
void Map_Add(Map*,char*,char*);
void Crawler_Task_Queue_Destroy(Crawler_Task_Queue*);
void Map_delete(Map*);
void Map_free(Map *);
#endif
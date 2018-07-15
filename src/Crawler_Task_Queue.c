#include "../header/Crawler_Task_Queue.h"

Crawler_Task_Queue* Crawler_Task_Queue_Init(char *dir){
	Crawler_Task_Queue *myQueue=malloc(sizeof(Crawler_Task_Queue));
	if(myQueue==NULL) perror_exit("Cannot allocate Task Queue");
	myQueue->numOfTasks=0; //only as a guard for the type of insertion-delete
	myQueue->pages_served=0; // how many pages already served, for STATS
	myQueue->bytes_served=0; // total bytes of the served pages, for STATS
	myQueue->front=NULL; //first node of list
	myQueue->end=NULL; //last node of list
	myQueue->m_lock=lock_init(); // has control of threads
	myQueue->myMap=malloc(sizeof(Map));
	myQueue->myMap->front=NULL;
	myQueue->myMap->end=NULL;
	myQueue->myMap->length=0;
	myQueue->root_dir = malloc((strlen(dir)+1)*sizeof(char));
	strcpy(myQueue->root_dir,dir);
	pthread_mutex_init(&(myQueue->rw_mutex),NULL); // to control queue modifications
	return myQueue;
}

void Crawler_Task_Queue_AddTask(Crawler_Task_Queue* myQueue,int fd,char *file){
	pthread_mutex_lock(&(myQueue->rw_mutex));

	if(notInMap(myQueue->myMap,file)){

		Task *newTask=malloc(sizeof(Task));
		newTask->descriptor=fd;
		newTask->fileName=malloc((strlen(file)+1)*sizeof(char));
		strcpy(newTask->fileName,file);
		printf("added --> %s\n",newTask->fileName );
		newTask->next=NULL;
		if(myQueue->numOfTasks==0){
			myQueue->front=newTask;
			myQueue->end=myQueue->front;
		}
		else if(myQueue->numOfTasks>0){
			Task *temp=myQueue->end;
			temp->next=newTask;
			myQueue->end=temp->next;
		}
		myQueue->numOfTasks++;
		printf("----------%s\n",file );
		Map_Add(myQueue->myMap,file,myQueue->root_dir);

		lock_signal(myQueue->m_lock);
	}
	pthread_mutex_unlock(&(myQueue->rw_mutex));
}

Task* Crawler_Task_Queue_GetTask(Crawler_Task_Queue * myQueue){
	pthread_mutex_lock(&(myQueue->rw_mutex));
	Task* ret=malloc(sizeof(Task));
	if(myQueue->numOfTasks==1){

		Task *temp=myQueue->front;
		ret->fileName=malloc((strlen(temp->fileName)+1)*sizeof(char));
		strcpy(ret->fileName,temp->fileName);
		ret->descriptor=temp->descriptor;

		free(myQueue->front->fileName);
		free(myQueue->front);
		myQueue->front=NULL;
		myQueue->end=NULL;

	}
	else if (myQueue->numOfTasks>1){
		Task *tmp=myQueue->front;
		while(tmp->next != myQueue->end)
			tmp=tmp->next;

		Task *temp=tmp->next;
		ret->fileName=malloc((strlen(temp->fileName)+1)*sizeof(char));
		strcpy(ret->fileName,temp->fileName);
		ret->descriptor=temp->descriptor;
		free(temp->fileName);
		free(temp);
		myQueue->end=tmp;
		myQueue->end->next=NULL;
	}
	else perror_exit("TaskQueue empty");
	myQueue->numOfTasks--;
	pthread_mutex_unlock(&(myQueue->rw_mutex));
	printf("got --> %s\n",ret->fileName );
	return ret;
}

void Crawler_Task_Queue_Destroy(Crawler_Task_Queue* TaskQueue){
	if(TaskQueue->numOfTasks>0){
		Task* temp=TaskQueue->front;
		while(temp!=NULL){
			Task *temp1=temp->next;
			free(temp->fileName);
			free(temp);
			temp=temp1;
		}
	}
	free(TaskQueue->root_dir);
	lock_free(TaskQueue->m_lock);
	Map_free(TaskQueue->myMap);
	free(TaskQueue);
}


int notInMap(Map *myMap,char *file){
	MapInfo* iterator=myMap->front;
	while(iterator!=NULL){
		if(strcmp(iterator->fileName,file)==0)
			return 0;
		iterator=iterator->next;
	}
	return 1;
}

void Map_Add(Map *myMap,char *file,char *home_dir){
	char *copy=malloc((strlen(file)+1)*sizeof(char));
	strcpy(copy,file);

	char *rest=copy;
	char *dir=strtok_r(rest,"/",&rest);
	dir=strtok_r(rest,"/",&rest);

	char *directory=malloc((strlen(dir)+strlen(home_dir)+2)*sizeof(char));
	sprintf(directory,"%s/%s",home_dir,dir);
	printf("dir---->%s\n",directory );
	DIR* dirc = opendir(directory);
	if(!dirc)
		mkdir(directory,0700);
	closedir(dirc);
	MapInfo *node=malloc(sizeof(MapInfo));
	node->fileName=malloc((strlen(file)+1)*sizeof(char));
	strcpy(node->fileName,file);
	node->next=NULL;
	if(myMap->front==NULL){
		myMap->front=node;
		myMap->end=node;
	}
	else{
		MapInfo*temp=myMap->end;
		temp->next=node;
		myMap->end=temp->next;
	}
	free(copy);
	free(directory);
	myMap->length++;
}
void Map_delete(Map*myMap){
	if(myMap->length==1){
		free(myMap->front->fileName);
		free(myMap->front);
	}
	else{
		MapInfo*temp=myMap->front;
		while(temp->next!=myMap->end)
			temp=temp->next;
		MapInfo* tmp=temp->next;
		free(tmp->fileName);
		free(tmp);
		myMap->end=temp;
		myMap->end->next=NULL;
	}
	myMap->length--;
}


void Map_free(Map *myMap){
	while(myMap->length>0)
		Map_delete(myMap);
	free(myMap);
}
#include "../header/Task_Queue.h"

Task_Queue* Task_Queue_Init(char * dir){
	Task_Queue *myQueue=malloc(sizeof(Task_Queue));
	if(myQueue==NULL) perror_exit("Cannot allocate Task Queue");
	myQueue->numOfTasks=0; //only as a guard for the type of insertion-delete
	myQueue->pages_served=0; // how many pages already served, for STATS
	myQueue->bytes_served=0; // total bytes of the served pages, for STATS
	myQueue->front=NULL; //first node of list
	myQueue->end=NULL; //last node of list
	myQueue->m_lock=lock_init(); // has control of threads
	myQueue->root_dir = malloc((strlen(dir)+1)*sizeof(char));
	strcpy(myQueue->root_dir,dir);
	pthread_mutex_init(&(myQueue->rw_mutex),NULL); // to control queue modifications
	return myQueue;
}

void Task_Queue_AddTask(Task_Queue* myQueue,int fd,char *file){
	/*insert the connection's file descriptor to queue, along with the requested file name*/
	pthread_mutex_lock(&(myQueue->rw_mutex));
	Task *newTask=malloc(sizeof(Task));
	newTask->descriptor=fd;
	newTask->fileName=malloc((strlen(file)+strlen(myQueue->root_dir)+2)*sizeof(char));
	strcpy(newTask->fileName,myQueue->root_dir);
	strcat(newTask->fileName,"/");
	strcat(newTask->fileName,file);
	printf("added -->%s\n",newTask->fileName );
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
	myQueue->numOfTasks++; // increase the number of tasks
	lock_signal(myQueue->m_lock); //signal a thread 
	pthread_mutex_unlock(&(myQueue->rw_mutex));
}

Task* Task_Queue_GetTask(Task_Queue *myQueue){
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
	return ret;
}

void Task_Queue_Destroy(Task_Queue* TaskQueue){
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
	free(TaskQueue);
}
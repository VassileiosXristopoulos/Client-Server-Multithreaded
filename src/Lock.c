#include "../header/Lock.h"

void lock_wait(lock *m_lock){
	pthread_mutex_lock(&(m_lock->has_tasks));
	while(m_lock->numOfTasks==0){ //wait until task is given
		pthread_cond_wait(&(m_lock->cond),&(m_lock->has_tasks));
	}
	m_lock->numOfTasks--; //restore value
	pthread_mutex_unlock(&(m_lock->has_tasks));
}

lock * lock_init(){
	lock *mylock=malloc(sizeof(lock));
	pthread_mutex_init(&(mylock->has_tasks),NULL);
	pthread_cond_init(&(mylock->cond),NULL);
	mylock->numOfTasks=0;
	return mylock;
}

void lock_signal(lock *m_lock){
	pthread_mutex_lock(&(m_lock->has_tasks));
	m_lock->numOfTasks++;
	pthread_cond_signal(&(m_lock->cond));
	pthread_mutex_unlock(&(m_lock->has_tasks));
}

void lock_signal_all(lock *m_lock,int threads){
	for(int i=0;i<threads;i++)
		lock_signal(m_lock);
}

void lock_free(lock* m_lock){
	pthread_cond_destroy(&(m_lock->cond));
	pthread_mutex_destroy(&(m_lock->has_tasks));
	free(m_lock);
}
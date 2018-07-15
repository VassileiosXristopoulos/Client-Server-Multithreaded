#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H
#define _XOPEN_SOURCE 500


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <time.h>
#include <ftw.h>
#include <dirent.h>
#include "Queue.h"
#include "trie.h"
typedef struct FTW FTW;
typedef struct Socket_response Socket_response;
struct Socket_response{
	int fd;
	struct sockaddr * ptr;
};

int isNum(char*);
int getDigits(int );
void perror_exit (char *);
char* getFullTime();
char *readFile(char *);
int str_cut(char *, int , int );
int getSocketSize(int ,int );
Socket_response* setSocket(int);
char *makeMessage(char *,char *);
void GetRequest(int* ,struct sockaddr **,socklen_t ,int*);
char * ReadFromSocket(int );
void WriteToSocket(int ,char *,char *);
char * GetRequestContent(char *);
char *getElapsedTime(int );
int CountWords(char *,int );
void CountWordLength(char *,int*);
void SaveWords(char ** ,char* );
void WriteRequestToSocket(char *,char *,int,int );
int rmFiles(const char *, const struct stat *, int , FTW *);
void delete_all(char *);
char *createExecutorFIle(char *);
char ** makeNumberedArray(char *,int ,int );
char ** getContent(int* ,int ,char *,int );
void TriggerWorker(char* ,char* ,int );
int getlines(char* );
void GenerateWorkers(int , char **,char **);
char *GetAnswer(int ,fd_set ,int ,int *);
void WriteCommandToSocket(int,char*);
#endif

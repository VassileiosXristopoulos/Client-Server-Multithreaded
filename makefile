CC = gcc # Compiler

all:src/myhttpd src/crawler src/Worker 

src/myhttpd: src/Server.o src/myFunctions.o src/Thread_Pool.o src/Task_Queue.o src/Lock.o 
	$(CC) -o myhttpd Server.o myFunctions.o Thread_Pool.o Task_Queue.o Lock.o -lm -lpthread

src/Server.o:
	$(CC) -c -g src/Server.c

src/Thread_Pool.o:
	$(CC) -c -g src/Thread_Pool.c

src/Task_Queue.o:
	$(CC) -c -g src/Task_Queue.c

src/Lock.o:
	$(CC) -c -g src/Lock.c

src/crawler: src/Crawler.o src/myFunctions.o src/Crawler_Thread_Pool.o src/Task_Queue.o src/Lock.o src/Crawler_Task_Queue.o 
	$(CC) -o mycrawler Crawler.o myFunctions.o Crawler_Thread_Pool.o Task_Queue.o Lock.o Crawler_Task_Queue.o -lm -lpthread

src/Crawler.o:
	$(CC) -c -g src/Crawler.c

src/Crawler_Thread_Pool.o:
	$(CC) -c -g src/Crawler_Thread_Pool.c

src/Crawler_Task_Queue.o:
	$(CC) -c -g src/Crawler_Task_Queue.c

src/Worker: src/Worker.o src/myFunctions.o src/docKey.o src/HashTable.o src/trie.o src/postingList.o src/Queue.o src/WorkerFunctions.o src/Stack.o
	$(CC) -o Worker Worker.o myFunctions.o Stack.o docKey.o HashTable.o trie.o postingList.o Queue.o WorkerFunctions.o -lm

src/Worker.o:
	$(CC) -c -g src/Worker.c 

src/WorkerFunctions.o:
	$(CC) -c -g src/WorkerFunctions.c

src/Stack.o:
	$(CC) -c -g src/Stack.c

src/docKey.o:
	$(CC) -c -g src/docKey.c

src/Queue.o:
	$(CC) -c -g src/Queue.c

src/trie.o:
	$(CC) -c -g src/trie.c


src/postingList.o:
	$(CC) -c -g src/postingList.c

src/HashTable.o:
	$(CC) -c -g src/HashTable.c

src/myFunctions.o:
	$(CC) -c -g src/myFunctions.c


.Phony: clean

clean:
	rm -r myhttpd Server.o myFunctions.o Thread_Pool.o Task_Queue.o Lock.o
	rm -r mycrawler Crawler.o Crawler_Task_Queue.o Crawler_Thread_Pool.o
	rm -r Worker trie.o Worker.o postingList.o WorkerFunctions.o Stack.o docKey.o HashTable.o Queue.o

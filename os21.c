#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t empty;
pthread_cond_t full;
//its a global no bully

struct item{
int val;
int sleeptime;
}; 

int * curitem = malloc(sizeof(int));
struct item buffer[32]; //the main buffer

void consumer(int threadcap)
{
if(*curitem < 1)
	pthread_cond_wait(&empty); //wait for something to feed it
if(pthread_mutex_lock(&lock)){
	sleep(buffer[curitem].sleeptime);
	printf("%d is my number", buffer[curitem].val);
	*curitem = *curitem - 1;
	pthread_mutex_unlock(&lock);
	if(*curitem == threadcap - 2)
		pthread_cond_signal (&full); //if it was full, now signal that it's empty
}
//not handling errors here
return;
}

void producer(int threadcap)
{
if(curitem > threadcap - 1)
	pthread_cond_wait(&full); //wait for something to come eat
if(pthread_mutex_lock(&lock)){
	sleep(buffer[curitem].sleeptime);
curitem++;
buffer[curitem].val = rander() % 100;
buffer[curitem].sleeptime = 2 + rander() % 8;
*curitem = *curitem + 1;	
pthread_mutex_unlock(&lock);
if(curitem == 1) //if it was empty, now there is something to eat, so wake him up
	pthread_cond_signal (&empty);
}
//rand in here somewhere
return;
}

int main(int argc, char **argv)
{
int curthread = 0;
*curitem = 0;
pthread_mutex_init(&lock, NULL);
pthread_cond_init(&full, NULL);
pthread_cond_init(&empty, NULL); //setup our conditional flags
int threadcap = atoi(argv[1]); //max number of threads from line
pthread_t totalthreads[threadcap]; //list of threads to manage
char cbuffer; //i/o buffer
int i = 0; int j = 0; //loop chaos vars
while(true){
//p producer
//c consumer
	if(curthread < threadcap-1) {//max size 
		cbuffer = getchar(); //read the 1 char of input
		if(cbuffer == 'p'){
			pthread_create(totalthreads[*curthread], NULL, producer, threadcap)
			curthread++;
		}
		if(cbuffer == 'c'){
			pthread_create(totalthreads[*curthread], NULL, consumer, threadcap)
			curthread++;
		}
	}
	for(i = curthread; i > -1; i--){
	//check all threads
	if(totalthreads[i] == NULL){ //this won't actually work TODO make work
		for(j = i; j < *curthread-1; j++)
			totalthreads[j] = totalthreads[j+1]; //shuffle all items down a slot
		curthread--; //this inner for loop removes empty spaces in the array, and gets rid of 1
		//empty per run. So we can lower the current number by 1 as well.
	}
	}
}
return 0;
}


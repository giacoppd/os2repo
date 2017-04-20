#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include "rand.c"

pthread_mutex_t lock;
pthread_cond_t empty;
pthread_cond_t full;
//its a global no bully

struct item{
int val;
int sleeptime;
};

struct item buffer[32]; //the main buffer

void * consumer(void *dummy)
{
int * curitem = (int *)(dummy);
if(*curitem < 0)
	pthread_cond_wait(&empty, &lock); //wait for something to feed it
if(pthread_mutex_lock(&lock) == 0){
	sleep(buffer[*curitem].sleeptime);
	printf("%d is my number", buffer[*curitem].val);
		*curitem = *curitem - 1;
	pthread_mutex_unlock(&lock);
        if(*curitem == 31)
		pthread_cond_signal(&full); //if it was full (at 31), now signal that it's empty
}
//not handling errors here
return NULL;
}

void * producer(void *dummy)
{
int * curitem = (int *)(dummy);
if(*curitem > 31)
	pthread_cond_wait(&full, &lock); //wait for something to come eat
if(pthread_mutex_lock(&lock) == 0){
	*curitem = *curitem + 1;
	sleep(generate_rand(3,7));
	buffer[*curitem].val = generate_rand(0,99);
	buffer[*curitem].sleeptime = generate_rand(2,9);
	pthread_mutex_unlock(&lock);
	if(*curitem == 0) //if it was empty, now there is something to eat, so wake him up
		pthread_cond_signal(&empty);
}
//rand in here somewhere
 return NULL;
}

int main(int argc, char **argv)
{
int * curitem = malloc(sizeof(int));
int curthread = 0; //loopvar
*curitem = -1;
pthread_mutex_init(&lock, NULL);
pthread_cond_init(&full, NULL);
pthread_cond_init(&empty, NULL); //setup our conditional flags
int threadcap = atoi(argv[1]); //max number of threads from line
pthread_t totalthreads[threadcap]; //list of threads to manage
pthread_t prod;
pthread_create(&prod, NULL, producer, (void*)curitem);
for(; curthread < threadcap; curthread++)
  pthread_create(&totalthreads[curthread], NULL, consumer, (void*)curitem);

return 0;
}

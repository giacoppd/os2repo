#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include "random.c"

pthread_mutex_t lock;
pthread_mutex_t lock2; /* current item lock */
pthread_cond_t empty;
pthread_cond_t full;
/* its a global no bully */

struct item{
	int val;
	int sleeptime;
};

struct item * buffer; /* the main buffer pointer */

void * consumer(void *dummy)
{
	pthread_mutex_lock(&lock2);
	int * curitem = (int *)(dummy);
	pthread_mutex_unlock(&lock2);
	while (1)
	{
		pthread_mutex_lock(&lock);
		while (*curitem < 0)
			pthread_cond_wait(&empty, &lock);
			/* wait for producer */

		sleep(buffer[*curitem].sleeptime);
		printf("%d is my number\n", buffer[*curitem].val);
		fflush(stdout);
		buffer[*curitem].val = 0;
		buffer[*curitem].sleeptime = 0;
		*curitem = *curitem - 1;
		pthread_mutex_unlock(&lock);

		/* if it was full (at 31), now signal that it's empty */
		if (*curitem == 30)
			pthread_cond_signal(&full);

	/* not handling errors here */
	}
	return NULL;
}

void * producer(void *dummy)
{
	pthread_mutex_lock(&lock2);
	int * curitem = (int *)(dummy);
	pthread_mutex_unlock(&lock2);
	int i = 0;
	while (1)
	{
		/*for(i = 0; i < 32; i++)
		*  printf("%d ", buffer[i].val);
		*printf("\n");
		*/
		fflush(stdout);
		sleep(generate_rand(3,7));
                pthread_mutex_lock(&lock);

		while (*curitem > 30)
			/* wait for something to come eat */
			pthread_cond_wait(&full, &lock);

		*curitem = *curitem + 1;
		i = (int)generate_rand(0,10);
		buffer[*curitem].val = i;
		printf("%d\n", buffer[*curitem].val);
                fflush(stdout);
		i = (int)generate_rand(2,9);
		buffer[*curitem].sleeptime = i;
		/* if it was empty, now there is something to eat, so wake up */
		if (*curitem == 0) 
		  pthread_cond_signal(&empty);
		pthread_mutex_unlock(&lock);
	      
	}

	return NULL;
}

int main(int argc, char **argv)
{
	int * curitem = malloc(sizeof(int));
	int curthread = 0; /* loopvar */
	buffer = malloc(sizeof(struct item)*32);
	*curitem = -1;
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&lock2, NULL);
	pthread_cond_init(&full, NULL);
	pthread_cond_init(&empty, NULL); /* setup our conditional flags */
	int threadcap = atoi(argv[1]); /* max number of threads from line */
	pthread_t conthreads[threadcap]; /* list of consumers */
	pthread_t prod;
	pthread_create(&prod, NULL, producer, (void*)curitem);

	for(; curthread < threadcap; curthread++)
		pthread_create(&conthreads[curthread], NULL, consumer, (void*)curitem);
        
	pthread_join(prod, NULL);

	for(curthread = 0; curthread < threadcap; curthread++)
		pthread_join(conthreads[curthread], NULL);
        
	return 0;
}

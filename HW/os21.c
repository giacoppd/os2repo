#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include "random.c"
#include <sys/types.h>
#include <sys/syscall.h>


pthread_mutex_t lock;
pthread_mutex_t lock2; //current item lock
pthread_cond_t empty;
pthread_cond_t full;
//its a global no bully

struct item{
        int val;
        int sleeptime;
};

struct item * buffer; /*the main buffer pointer*/

void * consumer(void *dummy)
{
        pthread_mutex_lock(&lock2);
        int * curitem = (int *)(dummy);
        pid_t x = syscall(__NR_gettid);
        pthread_mutex_unlock(&lock2);
        while(1)
        {
                pthread_mutex_lock(&lock);
                fflush(stdout);
                while(*curitem < 0){
//                        printf("I AM HUNGRY!\n");
                        pthread_cond_wait(&empty, &lock); /*wait for producer*/
                }
                sleep(buffer[*curitem].sleeptime);
                printf("(CONSUMER) My ID is %d and I ate the number %d in %d seconds\n\n",x,buffer[*curitem].val, buffer[*curitem].sleeptime);
                buffer[*curitem].val = 0;
                buffer[*curitem].sleeptime = 0;
                *curitem = *curitem - 1;
                pthread_mutex_unlock(&lock);

                if(*curitem == 30){
                        pthread_cond_signal(&full); /*if it was full (at 31), now signal that it's empty*/
                }
        /*not handling errors here*/
        }
        return NULL;
}
void * producer(void *dummy)
{
        pthread_mutex_lock(&lock2);
        int * curitem = (int *)(dummy);
        pid_t x = syscall(__NR_gettid);
        pthread_mutex_unlock(&lock2);
        int i = 0;
        while(1)
        {
                sleep(generate_rand(3,7));
                fflush(stdout);
                pthread_mutex_lock(&lock);

                while(*curitem > 30){
                        pthread_cond_wait(&full, &lock); //wait for something to come eat
                }

                *curitem = *curitem + 1;
                i = (int)generate_rand(0,10);
                printf("(PRODUCER) My ID is %d and I created the value %d\n\n",x,i);
                buffer[*curitem].val = i;
                i = (int)generate_rand(2,9);
                buffer[*curitem].sleeptime = i;
                if(*curitem == 0){ /*if it was empty, now there is something to eat, so wake him up*/
                    pthread_cond_signal(&empty);
                }
                pthread_mutex_unlock(&lock);
        }

        return NULL;
}

int main(int argc, char **argv)
{
        int * curitem = malloc(sizeof(int));
        int curthread = 0; /*loopvar*/
        buffer = malloc(sizeof(struct item)*32);
        *curitem = -1;
        pthread_mutex_init(&lock, NULL);
        pthread_mutex_init(&lock2, NULL);
        pthread_cond_init(&full, NULL);
        pthread_cond_init(&empty, NULL); /*setup our conditional flags*/
        int threadcap = atoi(argv[1]); /*max number of threads from line*/
        pthread_t conthreads[threadcap]; /*list of consumers*/
        pthread_t prod;
        pthread_create(&prod, NULL, producer, (void*)curitem);

        for(; curthread < threadcap; curthread++)
                pthread_create(&conthreads[curthread], NULL, consumer, (void*)curitem);

        pthread_join(prod, NULL);

        for(curthread = 0; curthread < threadcap; curthread++)
                pthread_join(conthreads[curthread], NULL);

        return 0;
}


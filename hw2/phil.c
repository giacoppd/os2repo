#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
// #include<time.h>
#include "random.c"

#define THREAD_AMT 5

void *philosopher_spawn(void *num);
void think(long num);
void get_forks(long num);
void eat(long num);
void put_forks(long num);
long check_fork(long num);

sem_t forks[THREAD_AMT];
const char *philosopher[THREAD_AMT] = {"Alice", "Bob", "Cathy", "Dave", "Fred"};

int main()
{
        pthread_t threads[THREAD_AMT];
        long i;

        for (i = 0; i < THREAD_AMT; i++)
        {
                sem_init(&forks[i], 1, 1);
        }

        for (i = 0; i < THREAD_AMT; i++)
        {
                if (pthread_create(&threads[i], NULL, philosopher_spawn, (void *)i))
                {
                        fprintf(stderr, "Philosopher Creation ERROR\n");
                        return 1;
                }
        }

        for (i = 0; i < THREAD_AMT; i++)
        {
                if ( pthread_join(threads[i], NULL) )
                {
                        fprintf(stderr, "Philosopher joining thread ERROR\n");
                        return 1;
                }
        }

        return 0;
}

/* check for the other fork */
long check_fork(long num)
{
        if (num == 0)
                return 0;
        else if (num == 1)
                return 0;
        else if (num == 2)
                return 1;
        else if (num == 3)
                return 2;
        else if (num == 4)
                return 3;

        return 0;
}

void *philosopher_spawn(void *num)
{
        long ph_id = (long) num;
        printf("------ %s joined the table\n", philosopher[ph_id]);

        while (1)
        {
                think(ph_id);
                get_forks(ph_id);
                eat(ph_id);
                put_forks(ph_id);
        }

        pthread_exit(0);
}

void think(long num)
{
        long sleepTimer = generate_rand(1, 20);
        printf("%s is THINKING for %ld second \n\n", philosopher[num], sleepTimer);
        sleep(sleepTimer);
}

void get_forks(long num)
{
        /* locks the semaphore */
        sem_wait(&forks[check_fork(num)]);
        if (num == 0)
            sem_wait(&forks[4]);
        else
            sem_wait(&forks[num]);
        printf("%s PICKED UP the forks\n\n", philosopher[num]);
}

void eat(long num)
{
        long sleepTimer = generate_rand(2, 9);
        printf("%s is EATING for %ld second \n\n", philosopher[num], sleepTimer);
        sleep(sleepTimer);
}

void put_forks(long num)
{
        /* release the semaphore */
        sem_post(&forks[check_fork(num)]);
        if (num == 0)
            sem_post(&forks[4]);
        else
            sem_post(&forks[num]);

        printf("%s PUTS DOWN the forks\n\n", philosopher[num]);
}

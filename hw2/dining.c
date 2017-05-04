#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>

#define THINK  0
#define HUNGRY 1
#define EAT    2
#define LEFT (ph_num+4)%5
#define RIGHT (ph_num+1)%5

sem_t mutex;
sem_t wake_up[5];

void * dining(void *num);
void take(int);
void put(int);
void try_eat(int);

int state[5];
int phil_num[5]={0,1,2,3,4};

char names[5][10]={"Jeff",
    "Kathy",
    "Rick",
    "Phil",
    "Cynthia"
};

int main()
{
    srand(time(0));
    int i;
    pthread_t thread_id[5];
    sem_init(&mutex,0,1);
    for(i=0;i<5;i++)
        sem_init(&wake_up[i],0,0);//Semaphore set for waking up philosophers ready to eat
    for(i=0;i<5;i++)
    {
        pthread_create(&thread_id[i],NULL,dining,&phil_num[i]);
        printf("%s is thinking\n",names[i]);//Start thinking as soon as the philosopher exists
    }
    for(i=0;i<5;i++)
        pthread_join(thread_id[i],NULL);//JOIN UP!
}

void *dining(void *num)
{
    while(1)
    {
        int *i = num;//Each person at the table has a their number
                     //number is important to waking up the correct philosopher
        take(*i);
        sleep(1);
        put(*i);
    }
}

void take(int ph_num)
{
    sem_wait(&mutex);//Lock the data so state can be set
    state[ph_num] = HUNGRY;//Needed for waking philosophers later
    printf("%s is Hungry\n",names[ph_num]);//Declare your status for program operation clarity
    try_eat(ph_num);//Try to grab some forks and eat
    sem_post(&mutex);//Release the data for other philosophers waiting to do the above
    sleep((rand() % 7) + 2);//Sleep for 2-9 seconds as you are eating
    sem_wait(&wake_up[ph_num]);//Wait to be woken up
}

void try_eat(int ph_num)
{
    if (state[ph_num] == HUNGRY && state[LEFT] != EAT && state[RIGHT] != EAT)//If neither your left or right are eating go and eat!
    {
        state[ph_num] = EAT;//Change state
        printf("%s takes fork %d and %d\n",names[ph_num],LEFT+1,ph_num+1);//Declare the state of the forks
        printf("%s is Eating\n",names[ph_num]);//Declare your own state
        sem_post(&wake_up[ph_num]);//This becomes relevant after putting forks down.
                                   //a thread will wake up either the left or right
    }                              //philosopher if they are hungry and the philosophers
}                                  //to the left and right are not eating


void put(int ph_num)
{
    sem_wait(&mutex);//Need to modify data again so lock, or wait to lock.
    state[ph_num] = THINK;//Think after you eat
    printf("%s puts fork %d and %d down\n",names[ph_num],LEFT+1,ph_num+1);//Put the forks down
    printf("%s is thinking\n",names[ph_num]);//Declare state
    try_eat(LEFT);//Try to get your buddies to eat
    try_eat(RIGHT);//Left then right. Order doesn't matter, forks will get around the table eventually
    sem_post(&mutex);
    sleep((rand() % 19) + 1);//Think for an amount of time
}

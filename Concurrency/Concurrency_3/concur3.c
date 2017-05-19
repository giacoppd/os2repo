#include <pthread.h>
#include <stdio.h>
#include <sys/queue.h>
#include <stdlib.h>
#include "random.c"
#define THREADS 5

SLIST_HEAD(listhead, entry) head = SLIST_HEAD_INITIALIZER(head);


struct entry{
	SLIST_ENTRY(entry) entries;
	int data;
};


	pthread_mutex_t nsearch;
	pthread_mutex_t ninsert;
	pthread_mutex_t insertl;
	pthread_mutex_t searchl;
	int searchc;	
	int insertc;

void *searcher(void * n)
{
	while(1){
	sleep(1);
	pthread_mutex_lock(&searchl);
	searchc++;
	if(searchc == 1)
		pthread_mutex_lock(&nsearch);
	pthread_mutex_unlock(&searchl);

	char id = (intptr_t) n;
	struct entry *itr;
	if(!SLIST_EMPTY(&head)){
			printf("I am  a searcher my thread ID is %c and I see this integer at the top: %d\n", id, SLIST_FIRST(&head)->data);
		
	}
	else
		printf("I am a searcher my thread ID is %c and I do not see any integers!\n", id);
	pthread_mutex_lock(&searchl);
	searchc--;
	if(searchc == 0){
		pthread_mutex_unlock(&nsearch);	
	//	printf("RELEASE\n");
	}
	pthread_mutex_unlock(&searchl);
	}
}

void *insert(void * n)
{
   while(1)
   {

	sleep(1);

	pthread_mutex_lock(&insertl);
	insertc++;
	if(insertc == 1)
		pthread_mutex_lock(&ninsert);



	char id = (intptr_t) n;
	struct entry *tmp;
	tmp = malloc(sizeof(struct entry));
	tmp->data = generate_rand(0,100);
	SLIST_INSERT_HEAD(&head,tmp,entries);
	printf("I am inserter %d and I am going to enter this integer %d\n",id, tmp->data);

	insertc--;

	if(insertc == 0){
		pthread_mutex_unlock(&ninsert);
	//	printf("RELEASE!!\n");
	}

	pthread_mutex_unlock(&insertl);
   }
}
 
void *delete(void * n)
{
while(1)
{
	sleep(1);
	pthread_mutex_lock(&nsearch);
	pthread_mutex_lock(&ninsert);
	char id = (intptr_t) n;
if(!SLIST_EMPTY(&head)){
	struct entry *remover;
	remover = SLIST_FIRST(&head);
	printf("I am a remover, my thread ID is %c and I am REMOVING %d\n", id,remover->data);
	SLIST_REMOVE_HEAD(&head, entries);
	free(remover);
}
else
	printf("I am a remover, my thread ID is %c and there is nothing to remove!\n",id);


	pthread_mutex_unlock(&nsearch);
	pthread_mutex_unlock(&ninsert);
}
}


int main()
{
pthread_mutex_init(&nsearch, NULL);
pthread_mutex_init(&searchl, NULL);
pthread_mutex_init(&insertl, NULL);
pthread_mutex_init(&ninsert, NULL);
SLIST_INIT(&head);
int i = 0;
pthread_t threads[THREADS];

char idA = 'A';
char idB = 'B';
char idC = 'C';
char idD = 'D';
char idE = 'E';
char idF = 'F';
pthread_create(&threads[0], NULL,delete, (void *)(intptr_t) idA);
pthread_create(&threads[1], NULL,delete, (void *)(intptr_t) idB);
pthread_create(&threads[2], NULL,insert  , (void *)(intptr_t) idC);
pthread_create(&threads[3], NULL,insert  , (void *)(intptr_t) idD);
pthread_create(&threads[4], NULL,searcher  , (void *)(intptr_t) idE);
pthread_create(&threads[5], NULL,searcher  , (void *)(intptr_t) idF);


for (i = 0; i < THREADS; i++)
        {
                if ( pthread_join(threads[i], NULL) )
                {
                        fprintf(stderr, "Joining thread ERROR\n");
                        return 1;
                }
        }
return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define NUM_THREADS 4

pthread_mutex_t ins_del_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sea_del_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stop_search  = PTHREAD_COND_INITIALIZER;

int DEL_FLAG = 0;
int INS_FLAG = 0;
int NUM_SEARCHES = 0;

struct list_head {
	int ldata;
	struct list_head *next;
	struct list_head *prev;
} linked_list;

void init_list (struct list_head * lh)
{
	lh->ldata = 0;
	lh->next = lh;
	lh->prev = lh;
}

struct list_head * search (struct list_head * lh, int data)
{
	struct list_head * item = lh->next;

	while(item != lh) {
		if (item->ldata == data)
			return item;

		item = item->next;
	}
	return NULL;
}

void insert(struct list_head * lh, int data)
{
	struct list_head * newhead = malloc(sizeof(struct list_head));
	newhead->ldata = data;
	newhead->next = lh->next;
	newhead->prev = lh;
	lh->next->prev = newhead;
	lh->next = newhead;
}

int delete(struct list_head * lh)
{
	if (lh == NULL || lh == &linked_list)
		return 1;

	lh->prev->next = lh->next;
	lh->next->prev = lh->prev;

	free(lh);
	return 0;
}

void* searcher (void * arg)
{
	int data = *(int*)arg;

	pthread_mutex_lock(&sea_del_mutex);
	if (DEL_FLAG) {
		printf(ANSI_COLOR_RED "Searcher thread is blocked.\n" ANSI_COLOR_RESET);
		pthread_cond_wait(&stop_search, &sea_del_mutex);
		printf(ANSI_COLOR_RED "Resuming searcher thread.\n" ANSI_COLOR_RESET);
	}

	NUM_SEARCHES++;
	pthread_mutex_unlock(&sea_del_mutex);


	// Search for item here
	printf(ANSI_COLOR_CYAN "Searcher thread is searching for %d. (%d searchers total)\n" ANSI_COLOR_RESET, data, NUM_SEARCHES); 
	fflush(stdout);

	sleep(1);
	struct list_head * result = search(&linked_list, data);

	pthread_mutex_lock(&sea_del_mutex);
	NUM_SEARCHES--;
	pthread_mutex_unlock(&sea_del_mutex);

	if (result == NULL) {
		printf(ANSI_COLOR_CYAN "Searcher thread is done, data %d not found. (%d searchers left)\n" ANSI_COLOR_RESET, data, NUM_SEARCHES);
		fflush(stdout);
	}
	else {
		printf(ANSI_COLOR_CYAN "Searcher thread is done, data %d found at address %p. (%d searchers left)\n" ANSI_COLOR_RESET, data, (void*)result, NUM_SEARCHES);
		fflush(stdout);
	}

	return NULL;
}

void* inserter(void * arg)
{
	int data = *(int*)arg;
	int flag = 0;

	if (INS_FLAG || DEL_FLAG) {
		printf(ANSI_COLOR_RED "Inserter thread is blocked.\n" ANSI_COLOR_RESET);
		flag = 1;
	}

	pthread_mutex_lock(&ins_del_mutex);
	INS_FLAG = 1;

	if (flag)
		printf(ANSI_COLOR_RED "Resuming Inserter thread.\n" ANSI_COLOR_RESET);

	// Insert an item here	
	printf(ANSI_COLOR_GREEN "Inserter thread is inserting data %d\n" ANSI_COLOR_RESET, data); 
	fflush(stdout);
	
	sleep(1);
	insert(&linked_list, data);

	INS_FLAG = 0;
	pthread_mutex_unlock(&ins_del_mutex);

	printf(ANSI_COLOR_GREEN "Inserter thread is done.\n" ANSI_COLOR_RESET);
	fflush(stdout);
	return NULL;
}

void* deleter(void * arg)
{
	int data = *(int *) arg;
	int flag = 0;
	if (INS_FLAG || DEL_FLAG) {
		printf(ANSI_COLOR_RED "Deleter thread is blocked.\n" ANSI_COLOR_RESET);
		flag =1;
	}
	pthread_mutex_lock(&ins_del_mutex);
	DEL_FLAG = 1;
	sleep(1);
	int var = NUM_SEARCHES;
	if (var && !flag) {
		printf(ANSI_COLOR_RED "Deleter thread is blocked.\n" ANSI_COLOR_RESET);
		flag =1;
	}


	while (var != 0) {
		sleep(1);
		var = NUM_SEARCHES;
	}

	if (flag)
		printf(ANSI_COLOR_RED "Resuming deleter thread.\n" ANSI_COLOR_RESET);

	// Delete an item here
	printf(ANSI_COLOR_YELLOW "Deleter thread is deleting item with data %d\n" ANSI_COLOR_RESET, data); 
	fflush(stdout);

	delete(search(&linked_list, data));
	
	DEL_FLAG = 0;
	pthread_cond_broadcast(&stop_search);
	pthread_mutex_unlock(&ins_del_mutex);

	printf(ANSI_COLOR_YELLOW "Deleter thread is done\n" ANSI_COLOR_RESET);
	fflush(stdout);
	return NULL;
}


void set_test1()
{
	pthread_t sea[NUM_THREADS];
	pthread_t ins[NUM_THREADS];

	int data[NUM_THREADS];

	printf("--- STARTING INSERT/SEARCH THREADS ---\n" );
	fflush(stdout);

	for (int i = 0; i < NUM_THREADS; i++) {
		data[i] = i+100;
		pthread_create(&ins[i], NULL, (void *) inserter, (void *) &data[i]);
		pthread_create(&sea[i], NULL, (void *) searcher, (void *) &data[i]);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(sea[i], NULL);
		pthread_join(ins[i], NULL);
	}

	printf("--- INSERT/SEARCH THREADS JOINED ---\n\n" );
	fflush(stdout);
}

void set_test2()
{
	pthread_t ins[NUM_THREADS];
	pthread_t del[NUM_THREADS];

	int ins_data[NUM_THREADS];
	int del_data[NUM_THREADS];

	printf("--- STARTING INSERT/DELETE THREADS ---\n" );
	fflush(stdout);

	for (int i = 0; i < NUM_THREADS; i++) {
		ins_data[i] = i + 100 + NUM_THREADS;
		del_data[i] = i + 100;
		pthread_create(&ins[i], NULL, (void *) inserter, (void *) &ins_data[i]);
		pthread_create(&del[i], NULL, (void *) deleter, (void *) &del_data[i]);
	}


	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(del[i], NULL);
		pthread_join(ins[i], NULL);
	}

	printf("--- INSERT/DELETE THREADS JOINED ---\n\n" );
	fflush(stdout);
}

void set_test3()
{
	pthread_t sea[NUM_THREADS];
	pthread_t del[NUM_THREADS];

	int data[NUM_THREADS];

	printf("--- STARTING DELETE/SEARCH THREADS ---\n" );
	fflush(stdout);

	for (int i = 0; i < NUM_THREADS; i++) {
		data[i] = i + 100 + NUM_THREADS;
		pthread_create(&del[i], NULL, (void *) deleter , (void *) &data[i]);
		pthread_create(&sea[i], NULL, (void *) searcher, (void *) &data[i]);
	}

	for (int i = 0; i < NUM_THREADS; i++){
		pthread_join(sea[i], NULL);
		pthread_join(del[i], NULL);
	}

	printf("--- DELETE/SEARCH THREADS JOINED ---\n\n");
	fflush(stdout);
}

int main()
{
	init_list(&linked_list);
	printf("\n");
	set_test1();
	set_test2();
	set_test3();
	return 0;
}

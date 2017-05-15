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

#define NUM_THREADS 1

pthread_mutex_t ins_del_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sea_del_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stop_search  = PTHREAD_COND_INITIALIZER;

int DEL_FLAG = 0;
int NUM_SEARCHES = 0;

void searcher (int * arg)
{
	int work = *arg;

	pthread_mutex_lock(&sea_del_mutex);
	if (DEL_FLAG)
		pthread_cond_wait(&stop_search, &sea_del_mutex);

	NUM_SEARCHES++;
	pthread_mutex_unlock(&sea_del_mutex);


	// Search for item here
	printf(ANSI_COLOR_CYAN "Searcher thread is searching for %d seconds (%d threads total)\n" ANSI_COLOR_RESET, work, NUM_SEARCHES); 
	fflush(stdout);
	sleep(work);

	pthread_mutex_lock(&sea_del_mutex);
	NUM_SEARCHES--;
	pthread_mutex_unlock(&sea_del_mutex);

	printf(ANSI_COLOR_CYAN "Searcher thread is done (%d remaining).\n" ANSI_COLOR_RESET, NUM_SEARCHES); 
	fflush(stdout);
}

void inserter(int * arg)
{
	int work = *arg;
	pthread_mutex_lock(&ins_del_mutex);

	// Insert an item here	
	printf(ANSI_COLOR_GREEN "Inserter thread is inserting for %d seconds\n" ANSI_COLOR_RESET, work); 
	fflush(stdout);
	sleep(work);

	pthread_mutex_unlock(&ins_del_mutex);

	printf(ANSI_COLOR_GREEN "Inserter thread is done.\n" ANSI_COLOR_RESET);
	fflush(stdout);
}

void deleter(int * arg)
{
	int work = *arg;
	pthread_mutex_lock(&ins_del_mutex);
	DEL_FLAG = 1;
	sleep(1);
	int var = NUM_SEARCHES;
	while (var != 0) {
		sleep(1);
		var = NUM_SEARCHES;
	}

	// Delete an item here
	printf(ANSI_COLOR_YELLOW "Deleter thread is deleting for %d seconds\n" ANSI_COLOR_RESET, work); 
	fflush(stdout);
	sleep(work);
	
	DEL_FLAG = 0;
	pthread_cond_broadcast(&stop_search);
	pthread_mutex_unlock(&ins_del_mutex);

	printf(ANSI_COLOR_YELLOW "Deleter thread is done\n" ANSI_COLOR_RESET);
	fflush(stdout);
}

void rand_test()
{
	pthread_t sea[NUM_THREADS*5];
	pthread_t ins[NUM_THREADS];
	pthread_t del[NUM_THREADS];

	int sea_work[NUM_THREADS*5];
	int ins_work[NUM_THREADS];
	int del_work[NUM_THREADS];

	srand(time(NULL));

	for (int i = 0; i < NUM_THREADS; i++) {
		ins_work[i] = rand() % 10 + 2;
		pthread_create(&ins[i], NULL, (void *) inserter, (void *) &ins_work[i]);
	}
	for (int i = 0; i < NUM_THREADS; i++) {
		del_work[i] = rand() % 10 + 2;
		pthread_create(&del[i], NULL, (void *) deleter, (void *) &del_work[i]);
	}
	for (int i = 0; i < 5*NUM_THREADS; i++) {
		sea_work[i] = rand() % 10 * 2;
		pthread_create(&sea[i], NULL, (void *) searcher, (void *) &sea_work[i]);
		sleep(1);
	}

	printf(ANSI_COLOR_RED "--- ALL THREADS STARTED ---\n" ANSI_COLOR_RESET);
	fflush(stdout);

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(ins[i], NULL);
	for (int i = 0; i < 5*NUM_THREADS; i++)
		pthread_join(sea[i], NULL);
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(del[i], NULL);

	printf(ANSI_COLOR_RED "--- ALL THREADS JOINED ---\n" ANSI_COLOR_RESET);
	fflush(stdout);
}

void set_test()
{
	pthread_t sea[NUM_THREADS*4];
	pthread_t ins[NUM_THREADS*4];
	pthread_t del[NUM_THREADS*4];

	int sea_work[NUM_THREADS*4];
	int ins_work[NUM_THREADS*4];
	int del_work[NUM_THREADS*4];

	for (int i = 0; i < 2*NUM_THREADS; i++) {
		ins_work[i] = 5;
		pthread_create(&ins[i], NULL, (void *) inserter, (void *) &ins_work[i]);
	}
	for (int i = 0; i < 2*NUM_THREADS; i++) {
		del_work[i] = 5;
		pthread_create(&del[i], NULL, (void *) deleter, (void *) &del_work[i]);
	}
	for (int i = 2*NUM_THREADS; i < 4*NUM_THREADS; i++) {
		ins_work[i] = 5;
		pthread_create(&ins[i], NULL, (void *) inserter, (void *) &ins_work[i]);
	}
	for (int i = 2*NUM_THREADS; i < 4*NUM_THREADS; i++) {
		del_work[i] = 5;
		pthread_create(&del[i], NULL, (void *) deleter, (void *) &del_work[i]);
	}

	printf(ANSI_COLOR_RED "--- NON-SEARCH THREADS STARTED ---\n" ANSI_COLOR_RESET);
	printf(ANSI_COLOR_RED "Creating new search thread every 2 seconds\n" ANSI_COLOR_RESET);
	fflush(stdout);

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4*NUM_THREADS; i++) {
			sea_work[i] = 4;
			pthread_create(&sea[i], NULL, (void *) searcher, (void *) &sea_work[i]);
			sleep(2);
		}
		for (int i = 0; i < 4*NUM_THREADS; i++)
			pthread_join(sea[i], NULL);
	}


	for (int i = 0; i < 4*NUM_THREADS; i++)
		pthread_join(ins[i], NULL);
	for (int i = 0; i < 4*NUM_THREADS; i++)
		pthread_join(del[i], NULL);

	printf(ANSI_COLOR_RED "--- ALL THREADS JOINED ---\n" ANSI_COLOR_RESET);
	fflush(stdout);
}

int main()
{
	//rand_test();
	set_test();
	return 0;
}

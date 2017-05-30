/*
 
 * Concurrency Assignment 4
 * Operating Systems II
 * Luke Morrison, Daniel Schroeder, Brian Ozarowicz
 * Group 11-05
 * Spring 2017

Consider a sharable resource with the following characteristics:

-	As long as there are fewer than three processes using the resource, 
	new processes can start using it right away.

-	Once there are three processes using the resource, 
	all three must leave before any new processes can begin using it.

Implement a mutual exclusion solution that meets the above constraints.

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <semaphore.h>

#define NUM_CONSUMER 8
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

struct shared{
	int value;
} my_shared;

sem_t mutex;

void* consumer (void *number);

int main(){

	sem_init(&mutex, 0, 3);  //assign value of three
	
	pthread_t consumer_thread[NUM_CONSUMER];
	int consumers[NUM_CONSUMER];
	int i;
	for (i=0; i<NUM_CONSUMER; i++) {
	consumers[i] = i;
	}
	
	
	
	for (i=0 ; i<NUM_CONSUMER; i++) {
		pthread_create(&consumer_thread[i], NULL, consumer, (void *)&consumers[i]);
	}
	for (i=0 ; i<NUM_CONSUMER; i++) {
		pthread_join(consumer_thread[i],NULL);
	}
}



void* consumer (void *number)
{
	//check if resource has < 3 threads
	int num = *(int *)number;
	printf(ANSI_COLOR_RED "Customer %d is alive.\n" ANSI_COLOR_RESET, num);
	fflush(stdout);
	
	sem_wait(&mutex);
	int val;
	sem_getvalue(&mutex, &val);
	printf("Value of mutex is: %d\n", val);
	
	
	

}
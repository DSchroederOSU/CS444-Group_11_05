/* In class recitation
   Concurrency Assignment 
   Spring 2017
*/
#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>


#define N 5

const char *phils[N] = { "phil1", "phil2", "phil3", "phil4", "phil5"};  
pthread_mutex_t forks[N];

void eat();
void getFork();
void think();
void putFork();


int main(){
	
	srand(time(NULL));
	int i;
	for(i = 0; i < 10; i++){
		eat();
	} 


return 0;
}


void eat(){
	int eat_time = (rand() % 8) + 2;
	printf("Random number for eating is %d\n", eat_time);

}




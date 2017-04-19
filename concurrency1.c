/* Concurrency Assignemnt1
   Operating Systems II
   Daniel Schroeder, Brian Ozarowicz
   Spring 2017
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <immintrin.h>

#define MAX_SIZE 32

//this calls assembler instruction rdrand
//and places the random number value into *arand
int rdrand64_step (uint64_t *arand){
	
	unsigned char ok;
	__asm__ __volatile__ ("rdrand %0; setc %1"
			     : "=r" (*arand), "=qm" (ok)
			     );
	return (int) ok;
}

pthread_mutex_t my_mutex;
pthread_cond_t full;
pthread_cond_t empty;

//this is the struct for the items in the buffer
struct buffer_item{
	int num; //this is the number to print	
	int wait; //this is the number of seconds to wait
};

//create buffer of size MAX_SIZE
struct buffer_item buffer[MAX_SIZE];


//functions
struct buffer_item make_item();
int check_rdrand();
void* produce(void *data);
void* consume(void *data);
int get_random_number(int min, int max);
int rdrand_flag;
int check_buffer_open();
int check_buffer_for_item();
int ccounter=0;
int pcounter=0;

int main( int argc, char *argv[]){
	
	if(argc > 2){ 
		printf("Usage: [executable] [(int) NumThreads]\n");
		return 1;
	}
	
	int number_of_threads = atoi( argv[1] );

	printf("before arrays number of threads is: %d\n", number_of_threads);	
	pthread_t *prod_threads;
	prod_threads = malloc(sizeof(pthread_t)*number_of_threads);
	pthread_t *cons_threads;
	cons_threads = malloc(sizeof(pthread_t)*number_of_threads);
	printf("after arrays\n");	
	
	int i;
		
	//intitallize buffer
	struct buffer_item none;
	none.num = -1;
	none.wait = -1;
	
	for(i = 0; i < MAX_SIZE; i++){
		buffer[i] = none;
	}
	
	//check to make sure rdrand is supported 
	if(check_rdrand() == 1) rdrand_flag = 1;

	
	pthread_mutex_init(&my_mutex, NULL);
	
	pthread_cond_init(&full, NULL);
	pthread_cond_init(&empty, NULL);

	printf("REAHCED\n")	;
	for(i = 0; i < number_of_threads; i++){
	
		//pthread_t producer_thread, consumer_thread;
		//prod_threads[i] = producer_thread;
		//cons_threads[i] = consumer_thread;
		pthread_create(&prod_threads[i], NULL, produce, NULL);
		printf("THIS IS PRINTING\n");
		pthread_create(&cons_threads[i], NULL, consume, NULL);

		printf("REACHED\n");
	}

	for(i = 0; i < number_of_threads; i++){
		pthread_join(prod_threads[i], NULL);
		pthread_join(cons_threads[i], NULL);
	}
	
		
			
	return 0;
}	

//wait a random amount of time 3-7 seconds,
//then add item to buffer
//if buffer is full, block until consumer removes an item
void* produce(void *data){
	int num = pcounter;
	pcounter++;	
	int wait_to_produce = get_random_number(3,7);
	
	printf("PRODUCER%d: Sleeping for %d seconds\n",num, wait_to_produce);	
	sleep(wait_to_produce);	
	
	pthread_mutex_lock(&my_mutex);
	int buffer_check = check_buffer_open();
	while(buffer_check == -1){
		pthread_cond_wait(&full, &my_mutex);	
		buffer_check = check_buffer_open();	
	}
	struct buffer_item tmp = make_item();
	printf("PRODUCER%d: adding item with num: %d and wait: %d\n", num, tmp.num, tmp.wait);
	buffer[buffer_check] = tmp;	
	pthread_cond_signal(&empty);
	pthread_mutex_unlock(&my_mutex);	
	pthread_exit(0);
}	

//Sleep for the amount of time in buffer_item->wait
//then print the number in buffer_item->num
//if buffer is empty, block until producer adds an item
void* consume(void *data){

	int num = ccounter;
	ccounter++;
	pthread_mutex_lock(&my_mutex);
	int buffer_check = check_buffer_for_item();
	while(buffer_check == -1){
		pthread_cond_wait(&empty, &my_mutex);
		buffer_check = check_buffer_for_item();
	}
	int temp = buffer[buffer_check].num;
	int time = buffer[buffer_check].wait;
	buffer[buffer_check].num = -1;
	
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&my_mutex);
	printf("CONSUMER%d: waiting %d seconds to consume %d\n", num, time, temp);
	sleep(buffer[buffer_check].wait);	
	printf("CONSUMER%d: consuming number %d\n", num, temp);
	pthread_exit(0);	
}

struct buffer_item make_item(){
	struct buffer_item temp;
	temp.num = get_random_number(-1, -1);
	temp.wait = get_random_number(2, 9);
	return temp; 
}


int get_random_number(int min, int max){

	uint64_t arand;
	//retry for 10 times in case rdrand was inturrupted
			 	
	int success = 1;
	int attempt_limit_exceeded = -1;
	int attempt_limit = 10;

	int i;
	for(i = 0; i < attempt_limit; i++){
		//if rdrand64_step returned a usable random value
		if(rdrand64_step(&arand)){
			if(min == -1 && max == -1)
				return (int) arand;
			else
				return (arand % (uint64_t)(max - min + 1)) + min;
		}  
	}
	
	//this will return -1 if rdrand failed
	return attempt_limit_exceeded;
	
				
}//end get_random_number

//this function returns the index of an available buffer space
//or -1 if buffer is full
int check_buffer_open(){
	int i;
	for(i = 0; i < MAX_SIZE; i++){
		if(buffer[i].num == -1 && buffer[i].wait == -1)
			return i;  
	}
	return -1;
}
int check_buffer_for_item(){
	int i;
	for(i = 0; i < MAX_SIZE; i++){
		if(buffer[i].num != -1 && buffer[i].wait != -1)
			return i;  
	}
	return -1;
}


int check_rdrand(){

	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	eax = 0x01;

	__asm__ __volatile__(
				"cpuid;"
				: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
				: "a"(eax)
				);

	if(ecx & 0x40000000){
		return 1;	
	}
	else{
		return 0;
	}

}//end check_rdrand

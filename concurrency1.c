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

int main(){
	
	//intitallize buffer
	struct buffer_item empty;
	empty.num = -1;
	empty.wait = -1;
	int i;
	for(i = 0; i < MAX_SIZE; i++){
		buffer[i] = empty;
	}
	
	//check to make sure rdrand is supported 
	if(check_rdrand() == 1) rdrand_flag = 1;

	pthread_t producer_thread, consumer_thread;

	pthread_mutex_init(&my_mutex, NULL);
	

	pthread_create(&producer_thread, NULL, produce, NULL);
	pthread_create(&consumer_thread, NULL, consume, NULL);
	

	pthread_join(producer_thread, NULL);
	pthread_join(consumer_thread, NULL);

	
	return 0;
}	

//wait a random amount of time 3-7 seconds,
//then add item to buffer
//if buffer is full, block until consumer removes an item
void* produce(void *data){
	int i;
	for(i = 0; i < 10; i++){
		int wait_to_produce = get_random_number(3,7);
		
		printf("PRODUCER: Sleeping for %d seconds\n", wait_to_produce);	
		sleep(wait_to_produce);	
		
		int buffer_check = check_buffer_open();	
		while(buffer_check == -1){

			buffer_check = check_buffer_open();	
		}
		pthread_mutex_lock(&my_mutex);
		struct buffer_item tmp = make_item();
		printf("PRODUCER: addinf item with num: %d and wait: %d\n", tmp.num, tmp.wait);
		buffer[buffer_check] = tmp;	
		pthread_mutex_unlock(&my_mutex);	
	}
	pthread_exit(0);
}	

//Sleep for the amount of time in buffer_item->wait
//then print the number in buffer_item->num
//if buffer is empty, block until producer adds an item
void* consume(void *data){

	int i;
	for(i = 0; i < 10; i++){
		int buffer_check = check_buffer_for_item();
		while(buffer_check == -1){
			
			buffer_check = check_buffer_for_item();
		}
		pthread_mutex_lock(&my_mutex);
			printf("CONSUMER: consuming number %d\n", buffer[buffer_check].num);
			buffer[buffer_check].num = -1;
			printf("CONSUMER: consuming for %d seconds\n", buffer[buffer_check].wait);
			sleep(buffer[buffer_check].wait);	
		pthread_mutex_unlock(&my_mutex);	
	}
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

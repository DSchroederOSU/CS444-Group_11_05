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
int check_rdrand();
void produce();
void consume();
int get_random_number(uint64_t * rand);
uint64_t my_random_number;
int rdrand_flag;


int main(){
	

	//check to make sure rdrand is supported 
	if(check_rdrand() == 1) rdrand_flag = 1;
		
	int i;
	for(i = 0; i < 20; i++){
		get_random_number(&my_random_number);
		printf("Random number generated: %llu\n", my_random_number);
	} 
	
	return 0;
}	

//wait a random amount of time 3-7 seconds,
//then add item to buffer
//if buffer is full, block until consumer removes an item
void produce(){
	
}

//Sleep for the amount of time in buffer_item->wait
//then print the number in buffer_item->num
//if buffer is empty, block until producer adds an item
void consume(){

}

int get_random_number(uint64_t *arand){

	//retry for 10 times in case rdrand was inturrupted
			 	
	int success = 1;
	int attempt_limit_exceeded = 0;
	int attempt_limit = 10;

	int i;
	for(i = 0; i < attempt_limit; i++){
		if(rdrand64_step(arand)) return success; 
	}
	return attempt_limit_exceeded;
	
				
}//end get_random_number

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

}

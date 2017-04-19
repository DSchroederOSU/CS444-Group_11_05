/* Concurrency Assignemnt1
   Operating Systems II
   Daniel Schroeder, Brian Ozarowicz
   Spring 2017
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_SIZE 32

struct buffer_item{
	int num; //this is the number to print	
	int wait; //this is the number of seconds to wait
};

struct buffer_item buffer[MAX_SIZE];

//functions
void produce();
void consume();

int main(){



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

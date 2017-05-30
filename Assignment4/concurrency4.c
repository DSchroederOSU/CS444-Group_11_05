/*
THE SLEEPING BARBER PROBLEM

A barbershop consists of a waiting room with n chairs, and the
barber room containing the barber chair. If there are no customers
to be served, the barber goes to sleep. If a customer enters the
barbershop and all chairs are occupied, then the customer leaves
the shop. If the barber is busy, but chairs are available, then the
customer sits in one of the free chairs. If the barber is asleep, the
customer wakes up the barber. Write a program to coordinate the
barber and the customers.

To make the problem a little more concrete, I added the following information:
• Customer threads should invoke a function named getHairCut.
• If a customer thread arrives when the shop is full, it can invoke balk,
which does not return.
• The barber thread should invoke cutHair.
• When the barber invokes cutHair there should be exactly one thread
invoking getHairCut concurrently.
Write a solution that guarantees these constraints.
*/

#define _REENTRANT

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>
#include <immintrin.h>
#include <semaphore.h>

// The maximum number of customer threads.
#define MAX_CUSTOMERS 25
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

/* This calls assembler instruction rdrand and places the random number value into *arand */
int rdrand64_step (unsigned long long *arand)
{
        unsigned char ok;
        __asm__ __volatile__ ("rdrand %0; setc %1" : "=r" (*arand), "=qm" (ok));
        return (int) ok;
}

static unsigned long mt[N];
static int mti = N + 1;

// Function prototypes...
void *cust(void *num);
void *barb(void *);
void cutHair();
void getHairCut(int n);
int rdrand(int min, int max);
int barber_cut_time(int min, int max);
int mt19937(int min, int max);
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);

// total number of customers that can be in the shop
// 1 in barber chair, n - 1 in waiting room
int n = 4; 

int customers = 0; //number of customers in the shop
int allDone = 0;
//customers counts the number of customers 
//in the shop; it is protected by mutex.


//mutex for fullwaitroom
sem_t mutex;
// waitingRoom Limits the # of customers allowed 
// to enter the waiting room at one time.
sem_t waitingRoom;   

// barberChair ensures mutually exclusive access to
// the barber chair.
sem_t barberChair;

// barberPillow is used to allow the barber to sleep
// until a customer arrives.
sem_t barberSleep;

// seatBelt is used to make the customer to wait until
// the barber is done cutting his/her hair. 
sem_t customerWait;


int main(int argc, char *argv[]) {
		//initialize semaphores
		sem_init(&waitingRoom, 0, n);  
		sem_init(&barberChair, 0, 1); 
		sem_init(&barberSleep, 0, 0); 
		sem_init(&customerWait, 0, 0); 
		sem_init(&mutex, 0, 1); 
		
		
		pthread_t barber_thread;
		pthread_t customer_thread[n];
		
		// Create the barber.
    	pthread_create(&barber_thread, NULL, barb, NULL);

		// Create the customers.
		// This is to print the thread number (i.e. Customer# is...)
		int customers[10];
    	int i;
    	for (i=0; i<10; i++) {
			customers[i] = i;
    	}
		
		for (i=0; i<10; i++) {
		pthread_create(&customer_thread[i], NULL, cust, (void *)&customers[i]);
		}

		// Join each of the threads to wait for them to finish.
		for (i=0; i<10; i++) {
		pthread_join(customer_thread[i],NULL);
		}

		// When all of the customers are finished, kill the
		// barber thread.

		allDone = 1;
		sem_post(&barberSleep); //wake barber to allow exit
		pthread_join(barber_thread, NULL);
		
		return 0;
}

void *cust(void *number) {
		int num = *(int *)number;
		sem_wait(&mutex); 
	
		if(customers == n){
			printf("No seats available, leaving store.\n");
			fflush(stdout); 
			sem_post(&mutex); 
		}
		else{
		customers += 1;
		printf("Customer %d entering waiting room.\n", num);
		fflush(stdout); 
		sem_post(&mutex); 
		
		// Wait for the barber chair to become free.
		sem_wait(&barberChair); 
		// The chair is free so give up your spot in the
    	// waiting room.
		sem_post(&waitingRoom);
		
		// Wake up the barber...
  	    printf("Customer %d waking the barber.\n", num);
  		sem_post(&barberSleep);
  		
		// Wait for the barber to finish cutting your hair.
    	sem_wait(&customerWait); 
		// Give up the chair.
   		sem_post(&barberChair);
   		printf("Customer %d leaving barber shop.\n", num);
		sem_wait(&mutex); 
		customers -= 1;
		sem_post(&mutex); 
		}
}
void *barb(void *b) {
		while(!allDone){
				printf("Barber is sleeping...\n");
				fflush(stdout); 
				sem_wait(&barberSleep);

				if(!allDone){
					int time = barber_cut_time(3, 7);
					printf("Barber is cutting hair for %d seconds...\n", time);
					fflush(stdout); 
					sleep(time);
					
					printf("Barber done with haircut. Going to sleep...\n");
					fflush(stdout);  
					sem_post(&customerWait);
        		}
        		else
        			printf("All customers have been serviced...\n", time);	
        			fflush(stdout); 
		}
		
}

int barber_cut_time(int min, int max)
{
		int time;
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
        eax = 0x01;
        __asm__ __volatile__("cpuid;" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
        if (ecx & 0x40000000)
        		time = rdrand(min, max);
        else
                time = mt19937(min, max);
                
       return time;
}

int rdrand(int min, int max)
{
		unsigned long long arand;
		int success = 1;
		int attempt_limit_exceeded = -1;
		int attempt_limit = 10;
		int i;
		for (i = 0; i < attempt_limit; i++) {
				/* if rdrand64_step returned a usable random value */
				if (rdrand64_step(&arand)) {
						if (min == -1 && max == -1)
								return (int) arand;
						else
								return (arand % (unsigned long long)(max - min + 1)) + min;
				}
		}
		return attempt_limit_exceeded;
}

int mt19937(int min, int max)
{
		/* use mt19937 */
		if (min == -1 && max == -1)
				return (int) genrand_int32();
		else
				return (genrand_int32() % (unsigned long long)(max - min + 1)) + min;

}
void init_genrand(unsigned long s)
{
        mt[0]= s & 0xffffffffUL;
        for (mti=1; mti<N; mti++) {
                mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
                /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
                /* 2002/01/09 modified by Makoto Matsumoto             */
                mt[mti] &= 0xffffffffUL;
        }
}

void init_by_array(unsigned long init_key[], int key_length)
{
        int i;
        int j;
        int k;
        init_genrand(19650218UL);
        i = 1;
        j = 0;
        k = (N > key_length ? N : key_length);
        for (; k; k--) {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL)) + init_key[j] + j;
                mt[i] &= 0xffffffffUL;
                i++;
                j++;
                if (i>=N) {
                        mt[0] = mt[N-1];
                        i=1;
                }
                if (j>=key_length)
                        j=0;
        }
        for (k=N-1; k; k--) {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL)) - i;
                mt[i] &= 0xffffffffUL;
                i++;
                if (i>=N) {
                        mt[0] = mt[N-1];
                        i=1;
                }
        }
        mt[0] = 0x80000000UL;
}

unsigned long genrand_int32(void)
{
        unsigned long y;
        static unsigned long mag01[2]={0x0UL, MATRIX_A};
        if (mti >= N) {
                int kk;
                if (mti == N+1)
                        init_genrand(5489UL);
                for (kk=0;kk<N-M;kk++) {
                        y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                        mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                for (;kk<N-1;kk++) {
                        y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                        mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
                mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
                mti = 0;
        }
        y = mt[mti++];
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680UL;
        y ^= (y << 15) & 0xefc60000UL;
        y ^= (y >> 18);
        return y;
}
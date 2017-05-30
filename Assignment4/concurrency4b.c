/*
 * Concurrency Assignment 4
 * Operating Systems II
 * Luke Morrison, Daniel Schroeder, Brian Ozarowicz
 * Group 11-05
 * Spring 2017
 */

#define _REENTRANT
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <semaphore.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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
void cut_hair(int time);
void get_hair_cut(int n);
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


void usage()
{
	printf("Usage: ./concurrency4 [num_chairs num_customers]\n");
}

int main(int argc, char *argv[]) {

	int num_cust = 10;

	if (argc != 1 && argc != 3) {
		usage();
		exit(1);
	}
	if (argc == 3) {
		n = atoi(argv[1]);
		if (n <= 0) {
			usage();
			exit(1);
		}
		num_cust = atoi(argv[2]);
		if (num_cust < 2) {
			usage();
			exit(1);
		}
	}

	printf(ANSI_COLOR_YELLOW "Number of chairs: %d, Number of customers: %d\n" ANSI_COLOR_RESET, n, num_cust);

	//initialize semaphores
	sem_init(&waitingRoom, 0, n);  
	sem_init(&barberChair, 0, 1); 
	sem_init(&barberSleep, 0, 0); 
	sem_init(&customerWait, 0, 0); 
	sem_init(&mutex, 0, 1); 
	
	
	pthread_t barber_thread;
	pthread_t customer_thread[num_cust];
	
	// Create the barber.
    	pthread_create(&barber_thread, NULL, barb, NULL);
	sleep(1);

	// Create the customers.
	// This is to print the thread number (i.e. Customer# is...)
	int customers[num_cust];
    	int i;
    	for (i=0; i<num_cust; i++) {
		customers[i] = i;
    	}
		
	for (i=0 ; i<num_cust; i++) {
		pthread_create(&customer_thread[i], NULL, cust, (void *)&customers[i]);
	}
	// Join each of the threads to wait for them to finish.
	for (i=0 ; i<num_cust; i++) {
		pthread_join(customer_thread[i],NULL);
	}

	sleep(10);
	printf("--- Second round of customers ---\n");
	fflush(stdout);
	sleep(2);

	for (i=0 ; i<num_cust; i++) {
		pthread_create(&customer_thread[i], NULL, cust, (void *)&customers[i]);
	}
	// Join each of the threads to wait for them to finish.
	for (i=0 ; i<num_cust; i++) {
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

	if(customers >= n){
		printf(ANSI_COLOR_RED "Customer %d can't find a seat.\n" ANSI_COLOR_RESET, num);
		fflush(stdout); 
	}
	else{
		customers += 1;
		printf(ANSI_COLOR_RED "Customer %d entering waiting room.\n" ANSI_COLOR_RESET, num);
		fflush(stdout); 

		if (customers == 1){
			// Wake up the barber...
			printf(ANSI_COLOR_RED "Customer %d waking the barber.\n" ANSI_COLOR_RESET, num);
			sem_post(&barberSleep);
		}
		sem_post(&mutex); 
		
		// Wait for the barber chair to become free.
		sem_wait(&barberChair); 
		// The chair is free so give up your spot in the
		// waiting room.
		sem_post(&waitingRoom);
		
		// Wait for the barber to finish cutting your hair.
		get_hair_cut(num);
		sem_wait(&customerWait); 
		// Give up the chair.
		sem_wait(&mutex); 
		sem_post(&barberChair);
		customers -= 1;
	}

	printf(ANSI_COLOR_RED "Customer %d leaving barber shop.\n" ANSI_COLOR_RESET, num);
	sem_post(&mutex); 

	return NULL;
}

void *barb(void *b) {
	while(!allDone){
		if (customers == 0) {
			printf(ANSI_COLOR_GREEN "Barber is going to sleep...\n" ANSI_COLOR_RESET);
			fflush(stdout); 
			sem_wait(&barberSleep);
			printf(ANSI_COLOR_GREEN "Barber is woken up...\n" ANSI_COLOR_RESET);
		}
		else {
			int time = barber_cut_time(3, 7);
			printf(ANSI_COLOR_GREEN "Barber is cutting hair for %d seconds...\n" ANSI_COLOR_RESET, time);
			fflush(stdout); 
			cut_hair(time);
			printf(ANSI_COLOR_GREEN "Barber done with haircut...\n" ANSI_COLOR_RESET);
			fflush(stdout);  
			sem_post(&customerWait);
			sleep(1);	//IMPORTANT! do not remove
       		}
	}

	printf(ANSI_COLOR_GREEN "All customers have been serviced!\n" ANSI_COLOR_RESET);
	fflush(stdout);

	return NULL; 
}

void cut_hair(int time)
{
	sleep(time);
}

void get_hair_cut(int num)
{
	printf(ANSI_COLOR_RED "Customer %d is getting hair cut.\n" ANSI_COLOR_RESET, num);
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

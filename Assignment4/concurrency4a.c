/*
 * Concurrency Assignment 4a
 * Operating Systems II
 * Daniel Schroeder, Brian Ozarowicz, Luke Morrison
 * Group 11-05
 * Spring 2017
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <semaphore.h>

#define NUM_CONSUMER 15
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_CUSTOMERS 33
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


/* This calls assembler instruction rdrand and places the random number value into *arand */
int rdrand64_step (unsigned long long *arand)
{
        unsigned char ok;
        __asm__ __volatile__ ("rdrand %0; setc %1" : "=r" (*arand), "=qm" (ok));
        return (int) ok;
}

static unsigned long mt[N];
static int mti = N + 1;

struct shared{
	int value;
} my_shared;

sem_t mutex;
sem_t block;

int waiting, active;
int is_full;

void* consumer (void *number);
int get_random_sleep(int min, int max);
int mt19937(int min, int max);
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);
int rdrand(int min, int max); 

int main() {
	sem_init(&mutex, 0, 3);  //assign value of three
	sem_init(&block, 0, 0);  //assign value of three
	is_full = 0;
	
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
	return 0;
}



void* consumer (void *number)
{
	int num = *(int *)number;
	printf(ANSI_COLOR_RED "Customer %d is alive.\n" ANSI_COLOR_RESET, num);
	fflush(stdout);

	sem_wait(&mutex);
	if(is_full == 1){
		waiting +=1;
		sem_post(&mutex);
		sem_wait(&block);
		sem_wait(&mutex);
		waiting -=1;
	}
	active +=1;
	is_full = active == 3;
	sem_post(&mutex);

	printf(ANSI_COLOR_RED "Customer %d has the resource.\n" ANSI_COLOR_RESET, num);
	fflush(stdout);
	int sleeptime = get_random_sleep(3, 6);
	sleep(sleeptime);

	sem_wait(&mutex);
	active -= 1;
	printf(ANSI_COLOR_GREEN "Customer %d is leaving the resource.\n" ANSI_COLOR_RESET, num);
	fflush(stdout);
	if( active == 0 ){
		printf(ANSI_COLOR_BLUE "RESOURCE IS EMPTY.\n" ANSI_COLOR_RESET);
		fflush(stdout);
		int n;
		if( waiting < 3) 
			n = waiting;
		else 
			n = 3;
		
		while(n > 0 ){
			sem_post(&block);
			n -= 1;
		}
		is_full = 0; 
	}
	sem_post(&mutex);
}


int get_random_sleep(int min, int max)
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

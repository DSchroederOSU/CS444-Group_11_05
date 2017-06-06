/*
 * Concurrency Assignment 5
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

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

#define PAPER 0
#define TABACCO 1
#define MATCHES 1

/* This calls assembler instruction rdrand and places the random number value into *arand */
int rdrand64_step (unsigned long long *arand)
{
	unsigned char ok;
	__asm__ __volatile__ ("rdrand %0; setc %1" : "=r" (*arand), "=qm" (ok));
	return (int) ok;
}

static unsigned long mt[N];
static int mti = N + 1;

// 0 = Paper, 1 = Tabacco, 2 = Matches
sem_t smoker[3];
sem_t ingr[3];

void * smoker_func(void * arg);
void * enabler_func();
int random_func(int min, int max);

int main() {

	pthread_t smokers[3];
	pthread_t enabler;
	int arg[3], i;

	sem_init(&smoker[PAPER], 0, -1);
	sem_init(&smoker[TABACCO], 0, -1);
	sem_init(&smoker[MATCHES], 0, -1);
	sem_init(&ingr[PAPER], 0, -1);
	sem_init(&ingr[TABACCO], 0, -1);
	sem_init(&ingr[MATCHES], 0, -1);

	for (i = 0; i < 3; i++) {
		arg[i] = i;
		pthread_create(&smokers[i], NULL, smoker_func, (void*)&arg[i]);
	}
	pthread_create(&enabler, NULL, enabler_func, NULL);

	pthread_join(enabler, NULL);
	for (i = 0; i < 3; i++) {
		pthread_join(smokers[i], NULL);
	}

	return 0;
}

void *smoker_func(void * arg)
{
	int val = *((int*) arg);
	while (1) {
		sem_wait(&ingr[val]);
		printf(COLOR_RED "SMOKER 0: Waiting for tabacco and matches...\n" COLOR_RESET);
		printf(COLOR_YELLOW "SMOKER 1: Waiting for paper and matches...\n" COLOR_RESET);
		printf(COLOR_CYAN "SMOKER 2: Waiting for paper and tabacco...\n\n" COLOR_RESET);
		switch(val) {
			case 0:	printf(COLOR_RED "SMOKER %d: Gathering tabacco and matches. Smoking..." COLOR_RESET, val);
				fflush(stdout);
				sleep(5);
				printf(COLOR_RED " Done.\n\n" COLOR_RESET);
				sem_post(&smoker[val]);
				break;
			case 1:	printf(COLOR_YELLOW "SMOKER %d: Gathering paper and matches. Smoking..." COLOR_RESET, val);
				fflush(stdout);
				sleep(5);
				printf(COLOR_YELLOW " Done.\n\n" COLOR_RESET);
				sem_post(&smoker[val]);
				break;
			case 2:	printf(COLOR_CYAN "SMOKER %d: Gathering paper and tabacco. Smoking..." COLOR_RESET, val);
				fflush(stdout);
				sleep(5);
				printf(COLOR_CYAN " Done.\n\n" COLOR_RESET);
				sem_post(&smoker[val]);
				break;
		}
			}
	return NULL;
}

void *enabler_func()
{
	int ingr_val;
	while(1) {
		ingr_val = random_func(0,2);
		switch(ingr_val) {
			case 0:	printf(COLOR_GREEN "ENABLER: Putting tabacco and matches on the table. \n" COLOR_RESET);
				break;
			case 1:	printf(COLOR_GREEN "ENABLER: Putting paper and matches on the table. \n" COLOR_RESET);
				break;
			case 2:	printf(COLOR_GREEN "ENABLER: Putting paper and tabacco on the table. \n" COLOR_RESET);
				break;
		}
		fflush(stdout);
		sem_post(&ingr[ingr_val]);

		printf(COLOR_GREEN "ENABLER: Waiting for smoker to be done.\n\n" COLOR_RESET);
		sem_wait(&smoker[ingr_val]);
	}
	return NULL;
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
				return (int)arand;
			else
				return (arand % (unsigned long long)(max - min + 1)) + min;
		}
	}
	return attempt_limit_exceeded;
}

void init_genrand(unsigned long s)
{
	mt[0]= s & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		mt[mti] &= 0xffffffffUL;
	}
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

int mt19937(int min, int max)
{
	if (min == -1 && max == -1)
		return (int) genrand_int32();
	else
		return (genrand_int32() % (unsigned long long)(max - min + 1)) + min;
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

int random_func(int min, int max)
{
	int result;
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	eax = 0x01;
	__asm__ __volatile__("cpuid;" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	if (ecx & 0x40000000)
		result = rdrand(min, max);
	else
		result = mt19937(min, max);
	return result;
}

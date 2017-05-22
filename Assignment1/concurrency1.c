/*
 * Concurrency Assignment 1
 * Operating Systems II
 * Daniel Schroeder, Brian Ozarowicz
 * Group 11-05
 * Spring 2017
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <immintrin.h>
#include <stdint.h>

#define MAX_SIZE 32
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

/* This calls assembler instruction rdrand and places the random number value into *arand */
int rdrand64_step (uint64_t *arand)
{
        unsigned char ok;
        __asm__ __volatile__ ("rdrand %0; setc %1" : "=r" (*arand), "=qm" (ok));
        return (int) ok;
}

pthread_mutex_t my_mutex;
pthread_cond_t full;
pthread_cond_t empty;

/* This is the struct for the items in the buffer */
struct buffer_item {
        int num; /* The number to print	*/
        int wait; /* The number of seconds to wait */
};

struct buffer_item buffer[MAX_SIZE];

/* Functions */
struct buffer_item make_item();
int check_rdrand();
void* produce(void *);
void* consume(void *);
int get_random_number(int min, int max);
int rdrand_flag;
int check_buffer_open();
int check_buffer_for_item();
void print_buf();
unsigned long genrand_int32(void);
int ccounter = 0;
int pcounter = 0;
int buffer_size = 0;
static unsigned long mt[N];
static int mti = N + 1;

int main( int argc, char *argv[])
{
        if (argc > 2 || argc < 2) {
                printf ("Usage: [executable] [(int) NumThreads]\n");
                return 1;
        }

        int number_of_threads = atoi(argv[1]);
        pthread_t *prod_threads;
        prod_threads = malloc(sizeof(pthread_t)*number_of_threads);
        pthread_t *cons_threads;
        cons_threads = malloc(sizeof(pthread_t)*number_of_threads);

        int i;
        /* intitallize buffer */
        struct buffer_item none;
        none.num = -1;
        none.wait = -1;

        for (i = 0; i < MAX_SIZE; i++)
                buffer[i] = none;

        /* check if rdrand is supported */
        if (check_rdrand() == 1)
                rdrand_flag = 1;

        pthread_mutex_init(&my_mutex, NULL);

        pthread_cond_init(&full, NULL);
        pthread_cond_init(&empty, NULL);

        for (i = 0; i < number_of_threads; i++) {
                pthread_create(&prod_threads[i], NULL, produce, NULL);
                pthread_create(&cons_threads[i], NULL, consume, NULL);
        }

        for (i = 0; i < number_of_threads; i++) {
                pthread_join(prod_threads[i], NULL);
                pthread_join(cons_threads[i], NULL);
        }

        /* print function for debugging */
        //print_buf();

        return 0;
}

/* Wait a random amount of 3-7 seconds, then add item to buffer */
/* If buffer is full, block until consumer removes an item */
void* produce(void *d)
{
        int num = pcounter;
        pcounter++;
        int wait_to_produce = get_random_number(3,7);

        printf ("PRODUCER%d: Sleeping for %d seconds\n", num, wait_to_produce);
        sleep(wait_to_produce);

        pthread_mutex_lock(&my_mutex);
        int buffer_check = check_buffer_open();
        while (buffer_check == -1) {
                pthread_cond_wait(&full, &my_mutex);
                buffer_check = check_buffer_open();
        }
        struct buffer_item tmp = make_item();
        printf ("PRODUCER%d: adding item with num: %d and wait: %d\n", num, tmp.num, tmp.wait);
        buffer[buffer_check] = tmp;
        buffer_size++;
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&my_mutex);
        pthread_exit(0);
}

/* Sleep for the amount of time in buffer_item->wait then print the number in buffer_item->num */
/* If buffer is empty, block until producer adds an item */
void* consume(void *d)
{
        int num = ccounter;
        ccounter++;
        pthread_mutex_lock(&my_mutex);
        int buffer_check = check_buffer_for_item();
        while (buffer_check == -1) {
                pthread_cond_wait(&empty, &my_mutex);
                buffer_check = check_buffer_for_item();
        }
        int temp = buffer[buffer_check].num;
        int time = buffer[buffer_check].wait;
        buffer[buffer_check].num = -1;
        buffer[buffer_check].wait = -1;
        buffer_size--;
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&my_mutex);
        printf ("CONSUMER%d: waiting %d seconds to consume %d\n", num, time, temp);
        sleep(time);
        printf ("CONSUMER%d: consuming number %d\n", num, temp);
        pthread_exit(0);
}

struct buffer_item make_item() {
        struct buffer_item temp;
        temp.num = get_random_number(-1, -1);
        temp.wait = get_random_number(2, 9);
        return temp;
}

int get_random_number(int min, int max)
{
        if (check_rdrand() == 1) {
                uint64_t arand;
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
                                        return (arand % (uint64_t)(max - min + 1)) + min;
                        }
                }
                return attempt_limit_exceeded;
        }
        else {
        /* use mt19937 */
        if (min == -1 && max == -1)
                return (int) genrand_int32();
        else
                return (genrand_int32() % (uint64_t)(max - min + 1)) + min;
        }
}

/* This returns the index of an available buffer space or -1 if buffer is full */
int check_buffer_open()
{
        int i;
        for (i = 0; i < MAX_SIZE; i++) {
                if (buffer[i].num == -1 && buffer[i].wait == -1)
                        return i;
        }
        return -1;
}

int check_buffer_for_item()
{
        int i;
        for (i = 0; i < MAX_SIZE; i++) {
                if (buffer[i].num != -1 && buffer[i].wait != -1)
                        return i;
        }
        return -1;
}

int check_rdrand()
{
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
        eax = 0x01;
        __asm__ __volatile__("cpuid;" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
        if (ecx & 0x40000000)
                return 1;
        else
                return 0;
}

void print_buf()
{
        int i;
        for (i = 0; i < MAX_SIZE; i++)
                printf ("Buffer[%d]: num: %d wait: %d\n", i, buffer[i].num, buffer[i].wait);
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

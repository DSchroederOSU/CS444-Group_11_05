/*
THE SLEEPING BARBER PROBLEM

The barber has one barber chair and a waiting room with a number of chairs in it. 
When the barber finishes cutting a customer's hair, he dismisses the customer 
and then goes to the waiting room to see if there are other customers waiting. 
If there are, he brings one of them back to the chair and cuts his hair. 
If there are no other customers waiting, he returns to his chair and sleeps in it.

Each customer, when he arrives, looks to see what the barber is doing. 
If the barber is sleeping, then the customer wakes him up and sits in the chair. 
If the barber is cutting hair, then the customer goes to the waiting room. 
If there is a free chair in the waiting room, the customer sits in it and waits his turn. 
If there is no free chair, then the customer leaves.

# The first two are mutexes (only 0 or 1 possible)
Semaphore barberReady = 0
Semaphore accessWRSeats = 1     # if 1, the number of seats in the waiting room can be incremented or decremented
Semaphore custReady = 0         # the number of customers currently in the waiting room, ready to be served
int numberOfFreeWRSeats = N     # total number of seats in the waiting room

def Barber():
  while true:                   # Run in an infinite loop.
    wait(custReady)             # Try to acquire a customer - if none is available, go to sleep.
    wait(accessWRSeats)         # Awake - try to get access to modify # of available seats, otherwise sleep.
    numberOfFreeWRSeats += 1    # One waiting room chair becomes free.
    signal(barberReady)         # I am ready to cut.
    signal(accessWRSeats)       # Don't need the lock on the chairs anymore.
    # (Cut hair here.)

def Customer():
    wait(accessWRSeats)         # Try to get access to the waiting room chairs.
    if numberOfFreeWRSeats > 0: # If there are any free seats:
      numberOfFreeWRSeats -= 1  #   sit down in a chair
      signal(custReady)         #   notify the barber, who's waiting until there is a customer
      signal(accessWRSeats)     #   don't need to lock the chairs anymore
      wait(barberReady)         #   wait until the barber is ready
      # (Have hair cut here.)
    else:                       # otherwise, there are no free seats; tough luck --
      signal(accessWRSeats)     #   but don't forget to release the lock on the seats!
      # (Leave without a haircut.)

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
void *customer(void *num);
void *barber(void *);

int rdrand(int min, int max);
void barber_cut_time(int min, int max);
int mt19937(int min, int max);
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);

// Define semaphores.

//seats in waiting room
sem_t wait_seats;   

//only one person in barber chair
sem_t barber_chair;

// Allows barber to sleep when no customers waiting
sem_t barber_sleep;

// customer wait until barber is available
sem_t customer_wait;

// Flag to stop the barber thread when all customers
// have been serviced.
int allDone = 0;

int main(int argc, char *argv[]) {
    pthread_t barber_thread;
    pthread_t customer_thread[MAX_CUSTOMERS];
    long RandSeed;
    int i, numCustomers, numChairs;
    int customers[MAX_CUSTOMERS];
    
    
    /*    
    // Check to make sure there are the right number of
    // command line arguments.
    if (argc != 4) {
	printf("Use: SleepBarber <Num Customers> <Num Chairs> <rand seed>\n");
	exit(-1);
    }
    
    // Get the command line arguments and convert them
    // into integers.
    numCustomers = atoi(argv[1]);
    numChairs = atoi(argv[2]);
    RandSeed = atol(argv[3]);
    
    // Make sure the number of threads is less than the number of
    // customers we can support.
    if (numCustomers > MAX_CUSTOMERS) {
	printf("The maximum number of Customers is %d.\n", MAX_CUSTOMERS);
	exit(-1);
    }
    
    printf("\nSleepBarber.c\n\n");
    printf("A solution to the sleeping barber problem using semaphores.\n");
    
    // Initialize the random number generator with a new seed.
    srand48(RandSeed);
	*/
	
    // Initialize the numbers array.
    for (i=0; i<MAX_CUSTOMERS; i++) {
	customers[i] = i;
    }
    
		
    // Initialize the semaphores with initial values...
    sem_init(&wait_seats, 0, numChairs);
    sem_init(&barber_chair, 0, 1);
    sem_init(&barber_sleep, 0, 0);
    sem_init(&customer_wait, 0, 0);
    
    // Create the barber.
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Create the customers.
    for (i=0; i<numCustomers; i++) {
	pthread_create(&customer_thread[i], NULL, customer, i);
    }

    // Join each of the threads to wait for them to finish.
    for (i=0; i<numCustomers; i++) {
	pthread_join(customer_thread[i],NULL);
    }

    // When all of the customers are finished, kill the
    // barber thread.
    allDone = 1;
    sem_post(&barber_sleep);  // Wake the barber so he will exit.
    pthread_join(barber_thread,NULL);	
}

void *customer(int number) {
    

    /* Leave for the shop and take some random amount of
   	// time to arrive.
    printf("Customer %d leaving for barber shop.\n", num);
    randwait(5);
    printf("Customer %d arrived at barber shop.\n", num);
    */
    

    // Wait for space to open up in the waiting room...
    sem_wait(&wait_seats);
    printf("Customer %d entering waiting room.\n", number);

    // Wait for the barber chair to become free.
    sem_wait(&barber_chair);
    
    // The chair is free so give up your spot in the
    // waiting room.
    sem_post(&wait_seats);

    // Wake up the barber...
    printf("Customer %d waking the barber.\n", number);
    sem_post(&barber_sleep);

    // Wait for the barber to finish cutting your hair.
    sem_wait(&customer_wait);
    
    // Give up the chair.
    sem_post(&barber_chair);
    printf("Customer %d leaving barber shop.\n", number);
}

void *barber(void *junk) {
    // While there are still customers to be serviced...
    // Our barber is omnicient and can tell if there are 
    // customers still on the way to his shop.
    while (!allDone) {

	// Sleep until someone arrives and wakes you..
	printf("The barber is sleeping\n");
	sem_wait(&barber_sleep);

	// Skip this stuff at the end...
	if (!allDone) {

	    // Take a random amount of time to cut the
	    // customer's hair.
	   	barber_cut_time(3, 6);
	    printf("The barber has finished cutting hair.\n");

	    // Release the customer when done cutting...
	    sem_post(&customer_wait);
	}
	else {
	    printf("The barber is going home for the day.\n");
	}
    }
}

void barber_cut_time(int min, int max)
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
                
        printf("Barber is cutting hair for %d seconds...\n", time);
        sleep(time);
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





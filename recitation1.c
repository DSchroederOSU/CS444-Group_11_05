/*
 * Recitation Concurrency Assignemnt
 * Operating Systems II
 * Brian Ozarowicz, Daniel Schroeder
 * Spring 2017
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t philosopher0;
pthread_t philosopher1;
pthread_t philosopher2;
pthread_t philosopher3;
pthread_t philosopher4;
pthread_mutex_t forks[5];

void *waiter(int n)
{
        printf("Philosopher %d is thinking\n", n);
        pthread_mutex_lock(&forks[n]);
        pthread_mutex_lock(&forks[(n+1)%5]);
        printf("Philosopher %d is eating\n", n);
        sleep(2);
        pthread_mutex_unlock(&forks[n]);
        pthread_mutex_unlock(&forks[(n+1)%5]);
        printf("Philosopher %d is done eating\n", n);
        return NULL;
}

int main()
{
        int i;
        pthread_mutex_init(&forks[0], NULL);
        pthread_mutex_init(&forks[1], NULL);
        pthread_mutex_init(&forks[2], NULL);
        pthread_mutex_init(&forks[3], NULL);
        pthread_mutex_init(&forks[4], NULL);
        pthread_create(&philosopher0, NULL, (void *)waiter, (void *)0);
        pthread_create(&philosopher1, NULL, (void *)waiter, (void *)1);
        pthread_create(&philosopher2, NULL, (void *)waiter, (void *)2);
        pthread_create(&philosopher3, NULL, (void *)waiter, (void *)3);
        pthread_create(&philosopher4, NULL, (void *)waiter, (void *)4);
        pthread_join(philosopher0, NULL);
        pthread_join(philosopher1, NULL);
        pthread_join(philosopher2, NULL);
        pthread_join(philosopher3, NULL);
        pthread_join(philosopher4, NULL);
        return 0;
}

/*
 * Concurrency Assignment 5
 * Operating Systems II
 * Luke Morrison, Daniel Schroeder, Brian Ozarowicz
 * Group 11-05
 * Spring 2017
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <semaphore.h>

#define GENERALIZED_MODE 0

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

sem_t agentSem;
sem_t paper;
sem_t tabacco;
sem_t match;

sem_t paperSmoker;
sem_t tabaccoSmoker;
sem_t matchSmoker;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int PAPER, TABACCO, MATCH;

void * smokerA_func();
void * smokerB_func();
void * smokerC_func();
void * helperA_func();
void * helperB_func();
void * helperC_func();
void * agentA_func();
void * agentB_func();
void * agentC_func();


int main() {

	pthread_t agentA, agentB, agentC;
	pthread_t helperA, helperB, helperC;
	pthread_t smokerA, smokerB, smokerC;

	sem_init(&agentSem, 0, 1);
	sem_init(&paper, 0, 0);
	sem_init(&tabacco, 0, 0);
	sem_init(&match, 0, 0);
	sem_init(&paperSmoker, 0, 0);
	sem_init(&tabaccoSmoker, 0, 0);
	sem_init(&matchSmoker, 0, 0);

	PAPER = 0;
	TABACCO = 0;
	MATCH = 0;

	pthread_create(&smokerA, NULL, smokerA_func, NULL);
	pthread_create(&smokerB, NULL, smokerB_func, NULL);
	pthread_create(&smokerC, NULL, smokerC_func, NULL);

	pthread_create(&helperA, NULL, helperA_func, NULL);
	pthread_create(&helperB, NULL, helperB_func, NULL);
	pthread_create(&helperC, NULL, helperC_func, NULL);

	pthread_create(&agentA, NULL, agentA_func, NULL);
	pthread_create(&agentB, NULL, agentB_func, NULL);
	pthread_create(&agentC, NULL, agentC_func, NULL);

	//stuff infinitely happening in threads. Main ends here

	pthread_join(agentA, NULL);
	pthread_join(agentB, NULL);
	pthread_join(agentC, NULL);

	pthread_join(helperA, NULL);
	pthread_join(helperB, NULL);
	pthread_join(helperC, NULL);

	pthread_join(smokerA, NULL);
	pthread_join(smokerB, NULL);
	pthread_join(smokerC, NULL);

	return 0;
}

void *smokerA_func()
{
	while (1) {
		printf(COLOR_RED "SMOKER A: Waiting for tabacco and match.\n" COLOR_RESET);
		fflush(stdout);
		sem_wait(&paperSmoker);
		#if GENERALIZED_MODE!=0
		sem_post(&agentSem);
		#endif
		printf(COLOR_CYAN "SMOKER: Taking tabacco and match. Adding paper and smoking... \n" COLOR_RESET);
		fflush(stdout);
		sleep(5); //Smoking
		printf(COLOR_CYAN "Done.\n\n" COLOR_RESET);
		fflush(stdout);
		#if GENERALIZED_MODE==0
		sem_post(&agentSem);
		#endif
	}
	return NULL;
}

void *smokerB_func()
{
	while (1) {
		printf(COLOR_RED "SMOKER B: Waiting for paper and match.\n" COLOR_RESET);
		fflush(stdout);
		sem_wait(&tabaccoSmoker);
		#if GENERALIZED_MODE!=0
		sem_post(&agentSem);
		#endif
		printf(COLOR_CYAN "SMOKER: Taking paper and match. Adding tabacco and smoking... \n" COLOR_RESET);
		fflush(stdout);
		sleep(5); //Smoking
		printf(COLOR_CYAN "Done.\n\n" COLOR_RESET);
		fflush(stdout);
		#if GENERALIZED_MODE==0
		sem_post(&agentSem);
		#endif
	}
	return NULL;
}

void *smokerC_func()
{
	while (1) {
		printf(COLOR_RED "SMOKER C: Waiting for paper and tabacco.\n" COLOR_RESET);
		fflush(stdout);
		sem_wait(&matchSmoker);
		#if GENERALIZED_MODE!=0
		sem_post(&agentSem);
		#endif
		printf(COLOR_CYAN "SMOKER: Taking paper and tabacco. Adding match and smoking... \n" COLOR_RESET);
		fflush(stdout);
		sleep(5); //Smoking
		printf(COLOR_CYAN "Done.\n\n" COLOR_RESET);
		fflush(stdout);
		#if GENERALIZED_MODE==0
		sem_post(&agentSem);
		#endif
	}
	return NULL;
}

void *helperA_func()
{
	while (1) {
		sem_wait(&paper);
		pthread_mutex_lock(&mutex);
		if (MATCH) {
			MATCH -= 1;
			sem_post(&tabaccoSmoker);
			printf(COLOR_YELLOW "HELPER: Gathering paper and match. Signaling tabacco Smoker.\n" COLOR_RESET);
			fflush(stdout);
		}
		else if (TABACCO) {
			TABACCO -= 1;
			sem_post(&matchSmoker);
			printf(COLOR_YELLOW "HELPER: Gathering paper and tabacco. Signaling match Smoker.\n" COLOR_RESET);
			fflush(stdout);
		}
		else
			PAPER += 1;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *helperB_func()
{
	while (1) {
		sem_wait(&tabacco);
		pthread_mutex_lock(&mutex);
		if (MATCH) {
			MATCH -= 1;
			sem_post(&paperSmoker);
			printf(COLOR_YELLOW "HELPER: Gathering tabacco and match. Signaling paper Smoker.\n" COLOR_RESET);
			fflush(stdout);
		}
		else if (PAPER) {
			PAPER -= 1;
			sem_post(&matchSmoker);
			printf(COLOR_YELLOW "HELPER: Gathering paper and tabacco. Signaling match Smoker.\n" COLOR_RESET);
			fflush(stdout);
		}
		else
			TABACCO += 1;

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *helperC_func()
{
	while (1) {
		sem_wait(&match);
		pthread_mutex_lock(&mutex);

		if (PAPER) {
			PAPER -= 1;
			sem_post(&tabaccoSmoker);
			printf(COLOR_YELLOW "HELPER: Gathering paper and match. Signaling tabacco Smoker.\n" COLOR_RESET);
			fflush(stdout);
		}
		else if (TABACCO) {
			TABACCO -= 1;
			sem_post(&paperSmoker);
			printf(COLOR_YELLOW "HELPER: Gathering tabacco and match. Signaling paper Smoker.\n" COLOR_RESET);
			fflush(stdout);
		}
		else
			MATCH += 1;

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *agentA_func()
{
	while (1) {
		sem_wait(&agentSem);
		printf(COLOR_GREEN "AGENT: Putting paper and tabacco on the table. \n" COLOR_RESET);
		fflush(stdout);
		sem_post(&tabacco);
		sem_post(&paper);
		#if GENERALIZED_MODE!=0
		sleep(4);
		#endif
	}
	return NULL;
}

void *agentB_func()
{
	while (1) {
		sem_wait(&agentSem);
		printf(COLOR_GREEN "AGENT: Putting paper and a match on the table. \n" COLOR_RESET);
		fflush(stdout);
		sem_post(&paper);
		sem_post(&match);
		#if GENERALIZED_MODE!=0
		sleep(4);
		#endif
	}
	return NULL;
}

void *agentC_func()
{
	while (1) {
		sem_wait(&agentSem);
		printf(COLOR_GREEN "AGENT: Putting tabacco and a match on the table. \n" COLOR_RESET);
		fflush(stdout);
		sem_post(&tabacco);
		sem_post(&match);
		#if GENERALIZED_MODE!=0
		sleep(4);
		#endif
	}
	return NULL;
}

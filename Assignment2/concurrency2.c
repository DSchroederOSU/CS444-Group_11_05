#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>

#define THINK_MIN 1
#define THINK_MAX 20
#define EAT_MIN 2
#define EAT_MAX 9

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int forks[5];
int eating[5];

char * names[5] = {"Albert", "Beaufort", "Charles", "David", "Elliot"};
pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;

int counter;

struct thread_info {
	int right;
	int left;
	int name;
	int eating;
};

void print_forks ()
{
	pthread_mutex_lock(&p_mutex);
	printf(ANSI_COLOR_GREEN "%d %s %d %s %d %s %d %s %d %s\n" ANSI_COLOR_RESET,
		forks[0], names[0], forks[1], names[1], forks[2],
		names[2],forks[3], names[3], forks[4], names[4]);
	pthread_mutex_unlock(&p_mutex);
}
void print_status()
{ 
	pthread_mutex_lock(&p_mutex);
	printf("--------Round %d---------\n", counter);
	
	int i;
	for(i = 0; i < 5; i++){
		if(forks[i] == 1)
			printf("Fork %d: %s\n", i, "Taken");
		else
			printf("Fork %d: %s\n", i, "Available");
	}
	for(i = 0; i < 5; i++){
		if(eating[i] == 1)
			printf("%s: %s\n", names[i], "Eating");
		else if(eating[i] == 0)
			printf("%s: %s\n", names[i], "Waiting");
		else
			printf("%s: %s\n", names[i], "Finished");
	}
	pthread_mutex_unlock(&p_mutex);	
	
}
void * philosopher_func (void * p_void)
{
	struct thread_info *p = (struct thread_info *)p_void;
	int thinktime, eattime;
	 

	//THINK
	thinktime = (rand() % (THINK_MAX-THINK_MIN+1)) + THINK_MIN;	
	printf("%s is starting to THINK for %d seconds.\n", names[p->name], thinktime);
	sleep(thinktime);

	//GET FORK
	printf("%s is done thinking. Getting forks...\n", names[p->name]);
	while (1) {
		pthread_mutex_lock(&p_mutex);
		if (forks[p->left] == 0 && forks[p->right] == 0) {
			forks[p->left] = 1;
			forks[p->right]= 1;
			eating[p->name] = 1;
			pthread_mutex_unlock(&p_mutex);
			break;
		}
		else {
			pthread_mutex_unlock(&p_mutex);
			//sleep(50); //maybe?
		}
	}
	 
	//EAT
	eattime = (rand() % (EAT_MAX-EAT_MIN+1)) + EAT_MIN;	
	printf("%s is starting to EAT for %d seconds.\n", names[p->name], eattime);
	sleep(eattime);

	//PUT FORK
	eating[p->name] = 2;
	printf("%s is done eating. Putting down forks...\n", names[p->name]);
	pthread_mutex_lock(&p_mutex);
	forks[p->left] = 0;
	forks[p->right]= 0;
	pthread_mutex_unlock(&p_mutex);
	print_status();
	counter++;

	return NULL; 
}

int main()
{
	memset(eating, 0, sizeof(int)*5);
	pthread_t thread_ids[5];
	struct thread_info p[5];

	srand(time(NULL));
	counter = 1;

	for (int i = 0; i < 5; i++) {
		forks[i] = 0;
		p[i].name = i;

		p[i].left = i;
		if (i < 4)
			p[i].right = i+1;
		else
			p[i].right = 0;

		pthread_create(&thread_ids[i], NULL, philosopher_func,(void *) &p[i]);
	}

	for (int i = 0; i < 5; i++)
		pthread_join(thread_ids[i], NULL);

	return 0;
}


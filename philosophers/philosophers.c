#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#define N 5 // количество философов
#define K 50 // количество жизненных циклов
#define THINKING 0   
#define HUNGRY 1       
#define EATING 2   


pthread_mutex_t mtx[N];
pthread_mutex_t print = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t meta = PTHREAD_MUTEX_INITIALIZER;
pthread_t phtreads[N];
int state[N];
int a[N];

int left(int i)
{
	return (i + N - 1)%N;
}

int right(int i)
{
	return (i + 1)%N;
}

void test(int i)         
{ 
	if (state[i] == HUNGRY && state[left(i)] != EATING && state[right(i)] != EATING) {
		state[i] = EATING;
		pthread_mutex_unlock(mtx + i);
	}
}

void take_forks(int i) 
{ 
		pthread_mutex_lock(&meta);

		state[i] = HUNGRY;
		test(i);

		pthread_mutex_unlock(&meta);    
		pthread_mutex_lock(mtx + i);     
}

void put_forks(int i)       
{ 
		pthread_mutex_lock(&meta); 

		state[i] = THINKING;
		test(left(i));      
		test(right(i));    

		pthread_mutex_unlock(&meta); 
}



void eat(int i){
	pthread_mutex_lock(&print); 
	printf("%d eating\n", i);
	pthread_mutex_unlock(&print); 
}

void think(int i){
	pthread_mutex_lock(&print); 
	printf("%d thinking\n", i);
	pthread_mutex_unlock(&print); 
}

void * philosopher(void *arument)
{
	int index = *(int *)arument;

	int k;
	for (k = 0; k < K; k++) {  
		think(index);
		//usleep(1000);//thinking     
		take_forks(index);   
		eat(index);
		//usleep(1000);//eating       
		put_forks(index);   
	}

	return NULL;
}

int main(int argc, char ** argv)
{
	int i;

	for (i = 0; i < N; ++i){
		mtx[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(mtx + i);
		a[i] = i;
		state[i] = THINKING;
	}

	for (i = 0; i < N; ++i)
		if (pthread_create(phtreads + i, NULL, philosopher, a + i)){
			perror("creation failed");
			exit(1);
		}


	
	for (i = 0; i < N; ++i)
		pthread_join(phtreads[i], NULL);
	


	return 0;
}
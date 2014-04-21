#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>


#define N 5

struct job {
  struct job* next; 
  int a, b, c;
};

struct queue
{
  struct job* begin;
  struct job* end; 
  int count; 
  int destroyed;
};

struct queue job_queue;

pthread_cond_t queue_flag;
pthread_mutex_t queue_mutex;

void process_job (struct job *t)
{
  static pthread_mutex_t print = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&print);
  printf("%d\n", t->a);
  pthread_mutex_unlock(&print);

}

void initialize_job_queue ()
{
  job_queue.begin = NULL;
  job_queue.end = NULL;
  pthread_mutex_init (&queue_mutex, NULL);
  pthread_cond_init (&queue_flag, NULL);
  job_queue.count = 0;
  job_queue.destroyed =0;

}

void destroy_queue()
{
  pthread_mutex_lock (&queue_mutex);
  job_queue.destroyed = 1;
  pthread_cond_signal (&queue_flag);
  pthread_mutex_unlock (&queue_mutex);

}

void* thread_function (void* thread_arg)
{
    while (1) {
        struct job* current_job;

        pthread_mutex_lock (&queue_mutex);

        while (job_queue.count == 0){ 
          if (job_queue.destroyed != 0){
            pthread_mutex_unlock (&queue_mutex);
            return NULL;
          }
          pthread_cond_wait (&queue_flag, &queue_mutex);
          if (job_queue.destroyed != 0 && job_queue.count == 0){
            pthread_mutex_unlock (&queue_mutex);
            return NULL;
          }
        }

        current_job = job_queue.begin;
        if (job_queue.count == 1)
          job_queue.end = NULL;
        job_queue.count--;
        job_queue.begin = current_job->next;

        pthread_mutex_unlock (&queue_mutex);

        process_job (current_job);
        free (current_job);
    }
    return NULL;
}



void enqueue_job (int a, int b, int c)
{
  struct job* new_job;
  new_job = (struct job*) malloc (sizeof (struct job));
  new_job->a = a;
  new_job->b = b;
  new_job->c = c;

  pthread_mutex_lock (&queue_mutex);
  new_job->next = NULL;
  if(job_queue.count == 0)
    job_queue.begin = new_job;
  else
    job_queue.end->next = new_job;
  job_queue.end = new_job;
  job_queue.count++;
  pthread_cond_signal (&queue_flag);
  pthread_mutex_unlock (&queue_mutex);
}

int main(int argc, char ** argv)
{
  int i;
  pthread_t phtreads[N];
  initialize_job_queue();

  for (i = 0; i < N; ++i)
    if (pthread_create(phtreads + i, NULL, thread_function, NULL)){
      perror("creation failed");
      exit(1);
    }

  for(i = 0; i < 100; i++)
    enqueue_job(i, 0, 0);

  destroy_queue();//задания больше не будут поступать
  
  for (i = 0; i < N; ++i)
    pthread_join(phtreads[i], NULL);
  


  return 0;
}
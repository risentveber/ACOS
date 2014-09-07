#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>


#define N 5
#define TIMES 2

struct job {
  struct job* next; 
  mpz_t a, b, c;
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
pthread_mutex_t print = PTHREAD_MUTEX_INITIALIZER;

void process_job (struct job *t)
{
  mpz_t res, i;
  mpz_init(i);
  //mpz_init_set_ui(tmp, 1U);
  mpz_init_set_ui(res, 1U); 
  
  while (1){
    mpz_add_ui(i, i, 1U);
    mpz_mul(res, res, t->a);
    mpz_mod(res, res, t->b);
    if (mpz_cmp(res, t->c) == 0){
      pthread_mutex_lock(&print);
      gmp_printf ("%Zd\n", i);
      pthread_mutex_unlock(&print);
      break;

    }

  }
  mpz_clears(i, res, NULL);
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
  pthread_cond_broadcast (&queue_flag);
  pthread_mutex_unlock (&queue_mutex);

}

void queue_elem_free(struct job *t)
{
  mpz_clear(t->a);
  mpz_clear(t->b);
  mpz_clear(t->c);
  free(t);
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
        queue_elem_free (current_job);
    }
    return NULL;
}



void enqueue_job (mpz_t a, mpz_t b, mpz_t c)
{
  struct job* new_job;
  new_job = (struct job*) malloc (sizeof (struct job));
  mpz_init(new_job->a);
  mpz_init(new_job->b);
  mpz_init(new_job->c);

  mpz_set(new_job->a, a);
  mpz_set(new_job->b, b);
  mpz_set(new_job->c, c);

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

  mpz_t a, b, c;
  mpz_inits(a, b, c, NULL);

  for(i = 0; i < TIMES; i++){
    gmp_scanf("%Zd %Zd %Zd", a, b, c);
    enqueue_job(a, b, c);
  }

  //adding into queue

  destroy_queue();//задания больше не будут поступать
  
  for (i = 0; i < N; ++i)
    pthread_join(phtreads[i], NULL);
  


  return 0;
}
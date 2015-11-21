#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 10
#define STANDBY_SIZE 10

typedef struct {
    void (*function)(void *);
    void *argument;
} pool_task_t;


struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
  pthread_t *threads;
  pool_task_t *queue;
  int thread_count;
  int queue_used;
  int task_queue_size_limit;
  int queue_front;
  int queue_rear;
  int close;
};

static void *thread_do_work(void *pool);


/*
 * Create a threadpool, initialize variables, etc
 *
 */
pool_t *pool_create(int queue_size, int num_threads)
{
  pool_t *pl = (pool_t*)malloc(sizeof(pool_t));
  pthread_mutex_init(&(pl->lock), NULL);
  pthread_cond_init (&(pl->not_empty), NULL);
  pthread_cond_init (&(pl->not_full), NULL);
  pl->thread_count = 0;
  pl->queue_front = 0;
  pl->queue_rear = 0;
  pl->queue_used = 0;
  pl->task_queue_size_limit = 20;
  pl->threads = (pthread_t *)malloc(sizeof(pthread_t) * MAX_THREADS);
  pl->queue = (pool_task_t *)malloc(sizeof(pool_task_t) * pl->task_queue_size_limit);
  for(int i = 0; i < MAX_THREADS; i++)
  {
    pthread_create(&(pl->threads[i]),NULL, thread_do_work, (void*)pl);
  }
  return pl;
}


/*
 * Add a task to the threadpool
 *
 */
int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
  int err = 0;
  pthread_mutex_lock(&(pool->lock));
  while(pool->queue_used == pool->task_queue_size_limit)
  {
    pthread_cond_wait(&(pool->not_full),&(pool->lock));
  }
  (pool->queue[pool->queue_rear]).function = function;
  (pool->queue[pool->queue_rear]).argument = argument;
  pool->queue_rear = (pool->queue_rear+1) % pool->task_queue_size_limit;
  pool->queue_used++;
  pthread_cond_signal(&(pool->not_empty));
  pthread_mutex_unlock(&(pool->lock));     
  return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;
    
    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *pool)
{ 

    while(1) {
        
    }

    pthread_exit(NULL);
    return(NULL);
}

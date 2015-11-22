#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "semaphore.h"

#define STANDBY_SIZE 10

int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);
int sem_check(m_sem_t *s);


m_sem_t *sem_init(m_sem_t *st, int value)
{

  	 st = (m_sem_t*)malloc(sizeof(m_sem_t)); //we do this in seat.c
	pthread_mutex_init(&(st->mutex), NULL); //initializing the mutex and condition variables
	pthread_cond_init(&(st->cond), NULL);

	st->value = value; //this is going ot be 10

	///////////Queue parts - must impliment a queue to keep an order of which person came first
	st->queue_front = 0;
	st->queue_rear = 0;
	st->task_queue_size_limit = STANDBY_SIZE; //can only have 10 threads in standby list at a time

	return st;
}

//will return 1 if there are no longer any spots
int sem_wait(m_sem_t *s)
{

	int temp = 0; //All have an index which is unique because threads have unique stacks therefore unique local variables

	pthread_mutex_lock(&(s->mutex)); //we have to lock this so other threads using semaphore can't make same changes to state of semaphore
	
	s->queue_rear = (s->queue_rear+1) % s->task_queue_size_limit; //This increases the queue
	temp = (s->queue_rear) - 1; //this will give the thread a unique ID
	
	--(s->value); //our standby list shrinks by one

	while(s->value <= 10) //Put this thread to sleep until
	{
		while((s->queue_front) != temp) //If threads unique ID equals front of queue, then we can leave while loop
		{
		pthread_cond_wait(&(s->cond), &(s->mutex)); //all threads will be awaken, but we only want the front thread to run
		}
	}
	
	s->queue_front = (s->queue_front + 1) % s->task_queue_size_limit; //increment the queueFront by one to the next one in the Queue

	pthread_mutex_unlock(&(s->mutex)); 
	return 1; //we want to return to callin function
}

int sem_post(m_sem_t *s)
{
	//TODO
  pthread_mutex_lock(&(s->mutex));

  if((s->value) < 10) //if threads get put on standby list, this will be less than 10, so we must incriment it and notify the waiting threads
  {
  ++(s->value);
  }
  pthread_mutex_unlock(&(s->mutex));
  pthread_cond_broadcast(&(s->cond)); //this will wake up all standby threads. 
  //NOTE: ONLY the most recently added one will get executed
  return 1; //we must return to calling function
}


int sem_check(m_sem_t *s)
{
	if((s->value) <= 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//todo - keep track of all occupied and empty seats
// If all seats are locked and a thread calls sem_wait (only if value>0)
//impliment the queue +
//Must add BIG lock so this immedietly returns to the cancel function and puts the seat in stadby
//come together and add the usr ID to the new open seat +

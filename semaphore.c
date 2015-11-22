#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "semaphore.h"
#include "seats.h"

#define STANDBY_SIZE 10


m_sem_t *sem_init(m_sem_t *st, int value)
{

  	 st = (m_sem_t*)malloc(sizeof(m_sem_t)); //we do this in seat.c
	pthread_mutex_init(&(st->mutex), NULL); //initializing the mutex and condition variables
	pthread_mutex_init(&(st->mutex1), NULL);
	pthread_cond_init(&(st->cond), NULL);

	st->value = value; //this is going ot be 10

	///////////Queue parts - must impliment a queue to keep an order of which person came first
	st->queue_front = 0;
	st->queue_rear = 0;
	st->task_queue_size_limit = STANDBY_SIZE; //can only have 10 threads in standby list at a time
	st -> usrID = 0;
	st -> used = 0; //this is to do indicate that 1 thread has left the standby list

	return st;
}

//will return 1 if there are no longer any spots
void sem_wait(m_sem_t *s)
{
	printf("IN semaphore \n");
	int temp = 0; //All have an index which is unique because threads have unique stacks therefore unique local variables
	int temp2 = (s-> usrID); //we want the user ID that is calluing the function to be stored on the stack temproarily 
	//this is the user id for the waiting list

	pthread_mutex_lock(&(s->mutex)); //we have to lock this so other threads using semaphore can't make same changes to state of semaphore
	
	s->queue_rear = (s->queue_rear+1) % s->task_queue_size_limit; //This increases the queue
	temp = (s->queue_rear) - 1; //this will give the thread a unique ID
	
	--(s->value); //our standby list shrinks by one

		//printf("your b4temp %d \n", temp);
		//printf("your b4queue front %d \n", s->queue_front);
		//printf("your value %d \n", s->value);
		
		int firstTime = 1;

		while((s->queue_front != temp) || firstTime)
		{
		while (s->used == 0)
		{
		pthread_cond_wait(&(s->cond), &(s->mutex)); //all threads will be awaken, but we only want the front thread to run
		printf("your WHILEtemp %d \n", temp);
		printf("your WHILEqueue front %d \n", s->queue_front);
		printf("your WHILEVALUE  %d \n", s->value);
		firstTime = 0;
		}
		}

	s->used = 0; //now we need to reset the used value so other threads cannot make the same mistake
	
	printf("your AFTERqueue front %d \n", s->queue_front);	
	printf("Semaphore AWAKE \n");
	printf("Your USR value in queued being awoken is: %d \n", temp2);	
	pthread_mutex_unlock(&(s->mutex)); 	
	s->queue_front = (s->queue_front + 1) % s->task_queue_size_limit; //increment the queueFront by one to the next one in the Queue
	confirm_seat1(temp2);
}

int sem_post(m_sem_t *s)
{
	printf("inside SEMPOST \n");
	//TODO
  pthread_mutex_lock(&(s->mutex));

  if((s->value) < 10) //if threads get put on standby list, this will be less than 10, so we must incriment it and notify the waiting threads
  {
  ++(s->value);
  }
  s -> used = 1; //this is to allow one thread to leave the while loop
  pthread_mutex_unlock(&(s->mutex));
  pthread_cond_broadcast(&(s->cond)); //this will wake up all standby threads. 
  //NOTE: ONLY the most recently added one will get executed
  return 1; //we must return to calling function
}


//todo - keep track of all occupied and empty seats
// If all seats are locked and a thread calls sem_wait (only if value>0)
//impliment the queue +
//Must add BIG lock so this immedietly returns to the cancel function and puts the seat in stadby
//come together and add the usr ID to the new open seat +

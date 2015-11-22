#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "semaphore.h"

//typedef struct m_sem_t { //moved this into a header file for access from other parts of the program

int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);

//
void sem_init(m_sem_t *s, int value)
{
  pthread_mutex_init(&(s->mutex), NULL);
  pthread_cond_init(&(s->cond), NULL);
  s->value = value;
}

void sem_wait(m_sem_t *s)
{
    //TODO
  pthread_mutex_lock(&(s->mutex)); //we have to lock this so other threads using semaphore can't make same changes to state of semaphore
  
  while(s->value <= 0) //Put this thread to sleep until
  {
    pthread_cond_wait(&(s->cond), &(s->mutex));
  }
  --(s->value);
  pthread_mutex_unlock(&(s->mutex)); 
  //return 0; //there is no need to have this return an integer
}

void sem_post(m_sem_t *s)
{
    //TODO
  pthread_mutex_lock(&(s->mutex));
  ++(s->value);
  pthread_mutex_unlock(&(s->mutex));
  pthread_cond_broadcast(&(s->cond));
  //return 0;
}
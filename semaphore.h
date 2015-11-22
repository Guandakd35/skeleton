#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

//must create another header file so we can access semphore from outside of the program

//we are creating a sem structure
typedef struct sem_t {

  int value; //classic value
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  //the standby list will have a queue to determine what requests came in first [basically same as threadpool.c]
  int task_queue_size_limit;
  int queue_front;
  int queue_rear;
  
} m_sem_t;


int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);
int sem_check(m_sem_t *s); //this checks if there are already 10 threads in the standby list
m_sem_t *sem_init(m_sem_t *s, int value); //initializes the global semaphore in the seats.c 

#endif

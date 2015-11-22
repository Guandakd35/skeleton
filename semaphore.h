#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

//must create another header file so we can access semphore from outside of the program
typedef struct sem_t {

  int value;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} m_sem_t;

void sem_wait(m_sem_t *s);
void sem_post(m_sem_t *s);
void sem_init(m_sem_t *s, int value);

#endif

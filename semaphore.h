#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

//must create another header file so we can access semphore from outside of the program
typedef struct sem_t;

int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);
int sem_check(m_sem_t *s); //this checks if there are already 10 threads in the standby list
m_sem_t *sem_init(m_sem_t *s, int value); //initializes the global semaphore in the seats.c 

#endif

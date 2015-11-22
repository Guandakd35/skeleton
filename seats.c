//This file includes some of the main functions that handle function calls made by the client

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "seats.h"
#include "semaphore.h"

seat_t* seat_header = NULL;
char seat_state_to_char(seat_state_t);

m_sem_t* sem = NULL; //we will create a GLOBAL pointer to a semaphore

 seat_t* OpenSeat = NULL; //all threads get access to this. This tell what the most recent open seat was. Racing conditions ok here

int FULL(); //this function tells iterates through all seats. If all are O or P then this returns 1
//int openseat(); //this will find the open seat for the thread that has returned off of standby mode

///////////////////
//NOTE: You are not actually changing the prexisting buf, only tagging stuff on the end and changing length through snprintf
//bufsize is the length of the buffer
////////////////////////

void list_seats(char* buf, int bufsize) //If an action in the form of an event call is made the buffer is overwritten here
{
	//since the buffer is unique to the thread, we aren't worrying about locks
    seat_t* curr = seat_header;
    int index = 0;
    while(curr != NULL && index < bufsize+ strlen("%d %c,")) // for integer and then a char
    {
        int length = snprintf(buf+index, bufsize-index, 
                "%d %c,", curr->id, seat_state_to_char(curr->state));
        if (length > 0)
            index = index + length;
        curr = curr->next; //we just iterate through the seats until there are non left
    }
    if (index > 0)
        snprintf(buf+index-1, bufsize-index-1, "\n");
    else
        snprintf(buf, bufsize, "No seats not found\n\n"); //the no seats found is additional stuff that is appended onto the back
}

//if we are changing the contents of the specific seat structure we will place a lock on it
void view_seat(char* buf, int bufsize,  int seat_id, int customer_id, int customer_priority)
{
    seat_t* curr = seat_header;
    while(curr != NULL)  //basically you are iterating through linked list until you stumble across right one
    {
        if(curr->id == seat_id)

        {
            if(curr->state == AVAILABLE || (curr->state == PENDING && curr->customer_id == customer_id))
            {
				pthread_mutex_lock(&(curr->lock)); //if we found a match we wanna lock down this seat while we make a change to memory

                snprintf(buf, bufsize, "Confirm seat: %d %c ?\n\n",
								curr->id, seat_state_to_char(curr->state));
                curr->state = PENDING;
                curr->customer_id = customer_id;

				pthread_mutex_unlock(&(curr->lock)); //we're done changin the critical space so we are going to unlcok
			}

			if(curr->state == PENDING || (curr->state == OCCUPIED)) //if the targeted seat iseither occupied or pending we are going to see if plane is full
			{
				//int ful = FULL(); //we can do extra messages if we have time
				//int room = sem_check(sem)

				if(FULL() && sem_check(sem)) //If plane is full and there is room on standby list, add user request to it
				{

					snprintf(buf, bufsize, "All Seats Full. Your User ID automatically placed in Standby. please wait... %d %c ?\n\n",
						curr->id, seat_state_to_char(curr->state));
					int random = sem_wait(sem); //now we step our thread into standby mode until a seat becomes avaible

					//When a seat becomes available we return to this point

					//Must LOCKDOWN all threads in Thread pool while we run this
					//pthread_mutex_lock(&(pool->lock)); //must lock EVERYTHING up while we reassign

					curr = openseat(); //we are reassinging the current seat to the new open one
					pthread_mutex_lock(&(curr->lock));  //LOCK

					snprintf(buf, bufsize, "Seat confirmed: %d %c\n\n",
						curr->id, seat_state_to_char(curr->state));
					curr->state = OCCUPIED;
					curr->customer_id = customer_id;

					pthread_mutex_unlock(&(curr->lock)); //UNLOCK
					//pthread_mutex_unlock(&(pool->lock)); //we are g
					return; //now we exit
				}

			}

            else
            {
                snprintf(buf, bufsize, "Seat unavailable\n\n");
            }

            return;
        }
        curr = curr->next;
    }
    snprintf(buf, bufsize, "Requested seat not found\n\n");
    return;
}

//will return 1 if all the seats on the plane are full
int FULL() 
{
    seat_t* curr = seat_header;
    while(curr != NULL)  //basically you are iterating through linked list until you stumble across right one
    {
        
            if(curr->state == AVAILABLE)
            {
				return 0; //this means we found a seat that is available therfore no standby list neeeded and we'll leave function with 0
            }
           
        }
        curr = curr->next;
    }
    snprintf("All seats on Plane are Full \n\n");
    return 1;
}

//This will iterate through al of the seats and find the open seat
 seat_t* openseat() 
{
	int seatNum = 0;
    seat_t* curr = seat_header;
	seat_t* seatOpen = seat_header;
    while(curr != NULL)  //basically you are iterating through linked list until you stumble across right one
    {
        
            if(curr->state == AVAILABLE)
            {
				return  curr; //this is the seat that got openned
            }
           
        }
        curr = curr->next;
    }
    snprintf("Another user snatched your seat, we are kicking out the first seat (victim)"); //we cannot protect against racing condition and block ALL threads yet
    return  seatOpen; //this is just the first seat
}

void confirm_seat(char* buf, int bufsize, int seat_id, int customer_id, int customer_priority)
{
    seat_t* curr = seat_header;
    while(curr != NULL)
    {
        if(curr->id == seat_id)
        {
            if(curr->state == PENDING && curr->customer_id == customer_id )
            {
				pthread_mutex_lock(&(curr->lock));  //LOCK
                snprintf(buf, bufsize, "Seat confirmed: %d %c\n\n",
                        curr->id, seat_state_to_char(curr->state));
                curr->state = OCCUPIED;
				pthread_mutex_unlock(&(curr->lock)); //UNLOCK
            }
            else if(curr->customer_id != customer_id ) //if the current customer is trying to reserver a seat that is not reservered fr him
            {
                snprintf(buf, bufsize, "Permission denied - seat held by another user\n\n");
            }
            else if(curr->state != PENDING)
            {
                snprintf(buf, bufsize, "No pending request\n\n");
            }

            return;
        }
        curr = curr->next;
    }
    snprintf(buf, bufsize, "Requested seat not found\n\n");
    
    return;
}


//This also is called from the Seat Confirmation page to cancel a pending seat reservation. 
//This can only be called when the same user has placed the seat into the pending (P) state, and returns the seat to the available (A) state.

void cancel(char* buf, int bufsize, int seat_id, int customer_id, int customer_priority)
{
    seat_t* curr = seat_header;
    while(curr != NULL)
    {
        if(curr->id == seat_id)// if the seat ID from the current one being iterated matches the input one
        {
            if(curr->state == PENDING && curr->customer_id == customer_id )
            {

				pthread_mutex_lock(&(curr->lock));  //LOCK
                snprintf(buf, bufsize, "Seat request cancelled: %d %c\n\n",
                        curr->id, seat_state_to_char(curr->state));
                curr->state = AVAILABLE;
                curr->customer_id = -1;
				pthread_mutex_unlock(&(curr->lock)); //UNLOCK

				OpenSeat = curr; //this will set the open seat to the one that just got openned
				int random =  sem_post(m_sem_t *s); //must awaken all of the the threads in the standby list if there are any
            }
            else if(curr->customer_id != customer_id )
            {
                snprintf(buf, bufsize, "Permission denied - seat held by another user\n\n");
            }
            else if(curr->state != PENDING)
            {
                snprintf(buf, bufsize, "No pending request\n\n");
            }

            return;
        }
        curr = curr->next;
    }
    snprintf(buf, bufsize, "Seat not found\n\n");
    
    return;
}

//This is called at the very beginning and therefore needs not to have any locks since nothing is being writtten to data
void load_seats(int number_of_seats)
{
    seat_t* curr = NULL;
    int i;
    for(i = 0; i < number_of_seats; i++)
    {   
        seat_t* temp = (seat_t*) malloc(sizeof(seat_t));
        temp->id = i;
        temp->customer_id = -1;
        temp->state = AVAILABLE;
        temp->next = NULL;
        pthread_mutex_init((&temp->lock),NULL);
        
        if (seat_header == NULL)
        {
            seat_header = temp;
        }
        else
        {
            curr-> next = temp;
        }
        curr = temp;
    }
	sem = sem_init(sem, 10); //We are initializing the GLOBAL semaphore here
}

void unload_seats()
{
    seat_t* curr = seat_header;
    while(curr != NULL)
    {
        seat_t* temp = curr;
        curr = curr->next;
        free(temp);
    }
}

char seat_state_to_char(seat_state_t state)
{
    switch(state)
    {
        case AVAILABLE:
            return 'A';
        case PENDING:
            return 'P';
        case OCCUPIED:
            return 'O';
    }
    return '?';
}

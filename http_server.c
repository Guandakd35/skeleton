#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include "thread_pool.h"
#include "seats.h"
#include "util.h"
#include <sys/timeb.h>
#define BUFSIZE 1024
#define FILENAMESIZE 100

void shutdown_server(int);

double duration;
int listenfd;


void dowork(int fd)
{
        //int connfd = *((int*)fd);
        struct timeb end;
        int connfd = fd;
        struct request *req = (struct request*)malloc(sizeof(struct request));
        //struct request req;
        pthread_mutex_init(&(req->lock),NULL);
        // parse_request fills in the req struct object
        pthread_mutex_lock(&(req->lock));
        parse_request(connfd, req);
        // process_request reads the req struct and processes the command
        process_request(connfd, req);
        ftime(&end);
        duration += (end.time*1000 +end.millitm) ;
        pthread_mutex_unlock(&(req->lock));
        close(connfd);
        free(req);
        //return NULL;

}

// TODO: Declare your threadpool!
pool_t *threadpool;

int main(int argc,char *argv[])
{
    pthread_t check_p;
    int flag, num_seats = 20;
    int connfd = 0;
    struct sockaddr_in serv_addr;

    char send_buffer[BUFSIZE];
    
    listenfd = 0; 

    int server_port = 8080;

    if (argc > 1)
    {
        num_seats = atoi(argv[1]);
    } 

    if (server_port < 1500)
    {
        fprintf(stderr,"INVALID PORT NUMBER: %d; can't be < 1500\n",server_port);
        exit(-1);
    }
    
    if (signal(SIGINT, shutdown_server) == SIG_ERR) 
        printf("Issue registering SIGINT handler");

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( listenfd < 0 ){
        perror("Socket");
        exit(errno);
    }
    printf("Established Socket: %d\n", listenfd);
    flag = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag) );

    // Load the seats;
    load_seats(num_seats);

    // set server address 
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(send_buffer, '0', sizeof(send_buffer));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(server_port);

    // bind to socket
    if ( bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0)
    {
        perror("socket--bind");
        exit(errno);
    }

    // listen for incoming requests
    listen(listenfd, 100);

    // TODO: Initialize your threadpool!
    threadpool = pool_create(5000,100);
    struct timeb start;
    pthread_create(&check_p,NULL,check_pend,NULL);

    // This while loop "forever", handling incoming connections
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        printf("the file D this time is %d\n",connfd);
        /*********************************************************************
            You should not need to modify any of the code above this comment.
            However, you will need to add lines to declare and initialize your 
            threadpool!

            The lines below will need to be modified! Some may need to be moved
            to other locations when you make your server multithreaded.
        *********************************************************************/
        // pthread_t tid;
        // pthread_create(&tid,NULL,dowork,(void *)(&connfd));
        // struct request req;
        // // parse_request fills in the req struct object
        // parse_request(connfd, &req);
        // // process_request reads the req struct and processes the command
        // process_request(connfd, &req);
        // close(connfd);
        ftime(&start);
        duration-=(start.time*1000 + start.millitm);
        while(pool_add_task(threadpool, (void*)dowork, (void*)connfd));
    }
}



void shutdown_server(int signo){
    printf("Shutting down the server...\n");
    int count;
    // TODO: Teardown your threadpool
    count = pool_destroy(threadpool);
    duration = duration / count;
    printf("The average respond time is %f ms \n",duration);
    // TODO: Print stats about your ability to handle requests.
    unload_seats();
    close(listenfd);
    exit(0);
}

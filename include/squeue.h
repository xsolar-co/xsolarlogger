#ifndef __SQUEUE_H__
#define __SQUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE          100
#define MAX_QUEUE_DATA_SIZE     2048

typedef struct {
    char data[MAX_QUEUE_SIZE][MAX_QUEUE_DATA_SIZE];
    int front;
    int rear;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;

} Queue;

typedef struct {
    int total;
    int current;
    Queue** queue;

} Channel;

void initQueue(Queue* q);
int isEmpty(Queue* q);
int isFull(Queue* q);
void enqueue(Queue* q, const char* data);
int dequeue(Queue* q, char* data);
int wait_dequeue(Queue* q, char* data);

int initChannel(Channel* c, int size);

#endif

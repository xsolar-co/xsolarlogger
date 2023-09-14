#include "squeue.h"

void initQueue(Queue* q) {
    q->front = q->rear = -1;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

int isEmpty(Queue* q) {
    return q->front == -1;
}

int isFull(Queue* q) {
    return (q->rear + 1) % MAX_QUEUE_SIZE == q->front;
}

void enqueue(Queue* q, const char* data) {
    pthread_mutex_lock(&q->lock);

    if (isFull(q)) {
        printf("Queue is full. Cannot enqueue.\n");
        pthread_mutex_unlock(&q->lock);
        return;
    }

    if (isEmpty(q)) {
        q->front = q->rear = 0;
    } else {
        q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    }

    memset(q->data[q->rear],0, MAX_QUEUE_DATA_SIZE);
    strcpy(q->data[q->rear], data);

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int dequeue(Queue* q, char* data) {
    pthread_mutex_lock(&q->lock);
    int ret = 0;

    if (isEmpty(q)) {
        pthread_mutex_unlock(&q->lock);
        return ret;
    }

    ret = 1;
    strcpy(data, q->data[q->front]);

    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    }

    pthread_mutex_unlock(&q->lock);
    return ret;
}

int wait_dequeue(Queue* q, char* data) {
    pthread_mutex_lock(&q->lock);
    
    // wait until it's not empty
    // while (!q->not_empty)
        pthread_cond_wait(&q->not_empty, &q->lock);
   
    
    strcpy(data, q->data[q->front]);

    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    }

    pthread_mutex_unlock(&q->lock);
    return 1;
}

int initChannel(Channel* c, int size) {
    
    c->total    = size;
    c->current  = 0;
    c->queue    = malloc(size * sizeof(Queue*));

    return size;
}

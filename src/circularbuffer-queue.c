/**
 * @file circularbuffer-queue.c
 * @brief Queue implementation using a circular buffer.
 *
 * The ring arithmetic keeps enqueue O(1) with no data movement.
 * Full/empty detection uses the "one slot reserved" convention:
 * the queue is full when advancing rear would make it equal to front.
 */

#include "circularbuffer-queue.h"
#include <stdio.h>

void cb_enqueue(CircularBuffer *cb, int data)
{
    if (cb->front == -1 && cb->rear == -1) {
        cb->front         = 0;
        cb->rear          = 0;
        cb->arr[cb->rear] = data;
        printf("Enqueued %d\n", data);
    }
    else if ((cb->rear + 1) % cb->size == cb->front) {
        printf("Queue is full\n");
    }
    else {
        cb->rear          = (cb->rear + 1) % cb->size;
        cb->arr[cb->rear] = data;
        printf("Enqueued %d\n", data);
    }
}

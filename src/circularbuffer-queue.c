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

void cb_enqueue(CircularBuffer *cb, int data) {
    if (cb == NULL || cb->arr == NULL) return;
    
    if (cb->count >= cb->size) {
        printf("Queue is full\n");
        return;
    }
    
    if (cb->count == 0) {
        cb->front = cb->rear = 0;
    } else {
        cb->rear = (cb->rear + 1) % cb->size;
    }
    
    cb->arr[cb->rear] = data;
    cb->count++;
    printf("Enqueued %d\n", data);
}

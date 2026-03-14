/**
 * @file circularbuffer-queue.h
 * @brief Queue implementation using a circular buffer.
 *
 * The CircularBuffer struct uses a pointer member for its storage ring.
 * Callers must perform two allocations and initialise all fields:
 *
 *   CircularBuffer *cb = malloc(sizeof(CircularBuffer));
 *   cb->arr            = malloc(size * sizeof(int));
 *
 * and issue two matching frees (arr first, then the control block).
 */

#ifndef CIRCULARBUFFER_QUEUE_H
#define CIRCULARBUFFER_QUEUE_H

/**
 * @brief Control block and storage for a circular-buffer queue.
 *
 * @note @c arr must point to a heap-allocated array of @c size integers.
 *       The caller is responsible for allocating and freeing @c arr
 *       separately from the CircularBuffer itself.
 */
typedef struct {
    int  size;  /**< Capacity of the queue (number of slots). */
    int  front; /**< Index of the front element; -1 when empty.  */
    int  rear;  /**< Index of the rear element;  -1 when empty.  */
    int *arr;   /**< Pointer to the heap-allocated storage ring. */
} CircularBuffer;

/**
 * @brief Enqueue an integer into the circular-buffer queue.
 *
 * Insertion policy (as specified):
 *  - If @c front == -1 and @c rear == -1 the queue is empty: set
 *    front = rear = 0 and store @p data at arr[rear].
 *  - Else if @c (rear + 1) % size == front the queue is full: print
 *    "Queue is full" and return without modifying the queue.
 *  - Otherwise advance rear = (rear + 1) % size and store @p data.
 *
 * @param[in,out] cb   Pointer to an initialised CircularBuffer.
 * @param[in]     data Integer value to enqueue.
 */
void cb_enqueue(CircularBuffer *cb, int data);

/**
 * @brief Dequeue the front element from the circular-buffer queue.
 *
 * Removal policy:
 *  - If @c front == -1 and @c rear == -1 the queue is empty: print
 *    "Queue is empty" and return without modifying the queue.
 *  - Else if @c front == rear only one element remains: print the
 *    dequeued value and reset front = rear = -1 (sentinel empty state).
 *  - Otherwise print the dequeued value and advance
 *    front = (front + 1) % size.
 *
 * @param[in,out] cb Pointer to an initialised CircularBuffer.
 */
void cb_dequeue(CircularBuffer *cb);

#endif /* CIRCULARBUFFER_QUEUE_H */

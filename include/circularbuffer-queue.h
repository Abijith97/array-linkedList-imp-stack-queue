/**
 * @file circularbuffer-queue.h
 * @brief Queue implementation using a circular buffer.
 *
 * The CircularBuffer struct uses a C99 flexible array member so that a single
 * heap allocation covers both the control block and the data array.  Callers
 * must allocate with:
 *
 *   CircularBuffer *cb = (CircularBuffer *)malloc(sizeof(CircularBuffer));
 *
 * and initialise all fields before calling any function.
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
    int  count;  // Number of elements
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

#endif /* CIRCULARBUFFER_QUEUE_H */

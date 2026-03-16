/**
 * @file singlylinkedlist-queue.h
 * @brief Queue implementation using a singly linked list.
 *
 * Elements are enqueued at the tail and dequeued from the head, giving
 * O(n) enqueue and O(1) dequeue.  The list grows dynamically on the heap;
 * callers must initialise head to NULL before the first call.
 */

#ifndef SINGLYLINKEDLIST_QUEUE_H
#define SINGLYLINKEDLIST_QUEUE_H

/**
 * @brief A node in the singly linked list queue.
 */
typedef struct SinglyLinkedList {
    int                   data; /**< Integer payload.            */
    struct SinglyLinkedList *next; /**< Pointer to the next node.   */
} SinglyLinkedList;

/**
 * @brief Enqueue an integer at the tail of the linked-list queue.
 *
 * Allocates a new node on the heap and appends it to the tail.  If
 * @c *head is NULL the new node becomes the sole element and @c *head is
 * updated accordingly.
 *
 * @param[in,out] head Pointer to the caller's head pointer.
 * @param[in]     data Integer value to enqueue.
 */
void sll_enqueue(SinglyLinkedList **head, int data);

#endif /* SINGLYLINKEDLIST_QUEUE_H */

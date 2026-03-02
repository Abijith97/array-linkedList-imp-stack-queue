/**
 * @file linkedlist-stack.h
 * @brief Stack implementation using a singly linked list.
 *
 * Each node owns its heap-allocated memory. The caller holds a pointer to the
 * top node; push/pop receive a pointer-to-pointer so they can update the
 * caller's top without relying on a return value.
 */

#ifndef LINKEDLIST_STACK_H
#define LINKEDLIST_STACK_H

/** @brief Singly-linked list node used as a stack element. */
struct node {
    int          data; /**< Integer payload stored in this node. */
    struct node *next; /**< Pointer to the node below in the stack, or NULL. */
};

/**
 * @brief Push a new integer onto the top of the linked-list stack.
 *
 * Allocates a new node, sets its data to @p new_data, and links it as the
 * new top.  If @p *top is NULL the stack is treated as empty.
 *
 * @param[in,out] top      Address of the caller's top pointer.
 * @param[in]     new_data Integer value to push.
 */
void ll_push(struct node **top, int new_data);

/**
 * @brief Pop the top element from the linked-list stack.
 *
 * Removes the top node, frees its memory, and updates the caller's top
 * pointer.  Prints "Stack Underflow" when the stack is empty.
 *
 * @param[in,out] top Address of the caller's top pointer.
 */
void ll_pop(struct node **top);

/**
 * @brief Display all elements of the linked-list stack from top to bottom.
 *
 * Prints "Stack Underflow" when the stack is empty.
 *
 * @param[in] top Pointer to the top node (read-only traversal).
 */
void ll_display(struct node *top);

#endif /* LINKEDLIST_STACK_H */

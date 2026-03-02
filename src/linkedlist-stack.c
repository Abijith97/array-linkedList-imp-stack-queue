/**
 * @file linkedlist-stack.c
 * @brief Stack implementation using a singly linked list.
 *
 * New nodes are prepended (O(1) push).  The top pointer lives in the caller;
 * every mutating function receives its address to keep the API side-effect-free
 * with respect to global state.
 */

#include "linkedlist-stack.h"
#include <stdio.h>
#include <stdlib.h>

void ll_push(struct node **top, int new_data)
{
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    new_node->data = new_data;
    new_node->next = NULL;

    if (*top == NULL) {
        *top = new_node;
    }
    else {
        new_node->next = *top;
        *top = new_node;
    }

    printf("Pushed %d to stack\n", new_data);
}

void ll_pop(struct node **top)
{
    /* TODO: implement pop */
    (void)top;
}

void ll_display(struct node *top)
{
    /* TODO: implement display */
    (void)top;
}

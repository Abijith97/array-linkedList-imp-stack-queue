/**
 * @file singlylinkedlist-queue.c
 * @brief Queue implementation using a singly linked list.
 */

#include "singlylinkedlist-queue.h"
#include <stdio.h>
#include <stdlib.h>

void sll_enqueue(SinglyLinkedList **head, int data)
{
    SinglyLinkedList *new_node = (SinglyLinkedList *)malloc(sizeof(SinglyLinkedList));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    }
    else {
        SinglyLinkedList *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    printf("Enqueued %d\n", data);
}

void sll_dequeue(SinglyLinkedList **head)
{
    if (*head == NULL) {
        printf("Queue is empty\n");
        return;
    }

    SinglyLinkedList *temp = *head;
    *head = (*head)->next;
    printf("Dequeued %d from queue\n", temp->data);
    free(temp);
}

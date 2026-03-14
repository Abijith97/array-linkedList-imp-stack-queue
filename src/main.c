#include "array-stack.h"
#include "circularbuffer-queue.h"
#include "linkedlist-stack.h"
#include <stdio.h>
#include <stdlib.h>

static void run_array_stack(void)
{
    int arr_size;
    printf("Enter size of stack: ");
    if (scanf("%d", &arr_size) != 1 || arr_size <= 0) {
        printf("Invalid input\n");
        return;
    }
    int *arr_stack = (int *)malloc(arr_size * sizeof(int));
    if (arr_stack == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    int top = -1;
    while (1) {
        int choice, value;
        printf("1. Push\n2. Pop\n3. Display\n4. Exit\nEnter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("Enter value to push: ");
                scanf("%d", &value);
                push(arr_stack, arr_size, value, &top);
                break;
            case 2:
                pop(arr_stack, &top);
                break;
            case 3:
                display(arr_stack, &top);
                break;
            case 4:
                free(arr_stack);
                return;
            default:
                printf("Invalid choice\n");
        }
    }
}

static void run_circularbuffer_queue(void)
{
    int cb_size;
    printf("Enter size of queue: ");
    if (scanf("%d", &cb_size) != 1 || cb_size <= 0) {
        printf("Invalid input\n");
        return;
    }

    CircularBuffer *cb = (CircularBuffer *)malloc(sizeof(CircularBuffer));
    if (cb == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    cb->arr = (int *)malloc((size_t)cb_size * sizeof(int));
    if (cb->arr == NULL) {
        printf("Memory allocation failed\n");
        free(cb);
        return;
    }
    cb->size  = cb_size;
    cb->front = -1;
    cb->rear  = -1;

    while (1) {
        int choice, value;
        printf("1. Enqueue\n2. Dequeue\n3. Exit\nEnter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("Enter value to enqueue: ");
                scanf("%d", &value);
                cb_enqueue(cb, value);
                break;
            case 2:
                cb_dequeue(cb);
                break;
            case 3:
                free(cb->arr);
                free(cb);
                return;
            default:
                printf("Invalid choice\n");
        }
    }
}

static void run_linkedlist_stack(void)
{
    struct node *top = NULL;
    while (1) {
        int choice, value;
        printf("1. Push\n2. Pop\n3. Display\n4. Exit\nEnter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("Enter value to push: ");
                scanf("%d", &value);
                ll_push(&top, value);
                break;
            case 2:
                ll_pop(&top);
                break;
            case 3:
                ll_display(top);
                break;
            case 4:
                return;
            default:
                printf("Invalid choice\n");
        }
    }
}

int main(void)
{
    int impl_choice;
    printf("Select implementation:\n1. Array Stack\n2. Linked List Stack\n3. Circular Buffer Queue\nEnter your choice: ");
    if (scanf("%d", &impl_choice) != 1) {
        printf("Invalid input\n");
        return 1;
    }
    switch (impl_choice) {
        case 1:
            run_array_stack();
            break;
        case 2:
            run_linkedlist_stack();
            break;
        case 3:
            run_circularbuffer_queue();
            break;
        default:
            printf("Invalid choice\n");
            return 1;
    }
    return 0;
}

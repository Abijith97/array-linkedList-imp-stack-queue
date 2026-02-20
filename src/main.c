#include "array-stack.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr_size;
    printf("Enter size of stack: ");
    if (scanf("%d", &arr_size) != 1 || arr_size <= 0) {
        printf("Invalid input\n");
        return 1;
    }
    int *arr_stack = (int *)malloc(arr_size * sizeof(int));
    if (arr_stack == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    int top = -1; // Initialize top to indicate empty stack
    while(1){
        int choice, value;
        printf("1. Push\n2. Pop\n3. Display\n4. Exit\nEnter your choice: ");
        scanf("%d", &choice);
        switch(choice){
            case 1:
                printf("Enter value to push: ");
                scanf("%d", &value);
                push(arr_stack, arr_size, value, &top);
                break;
            case 2:
                pop(arr_stack, &top);
                break;
            case 3:
                display(arr_stack, arr_size, &top);
                break;
            case 4:
                free(arr_stack);
                return 0;
            default:
                printf("Invalid choice\n");
        }
    }
    return 0;
}
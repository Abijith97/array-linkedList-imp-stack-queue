#include "include/array-stack.h"

int main() {
    int arr_size;
    scanf("Enter size of stack: %d", &arr_size);
    int *arr_stack = (int *)malloc(arr_size * sizeof(int));
    int *top = arr_size - 1; // Initialize top to indicate empty stack
    while(1){
        int choice, value;
        printf("1. Push\n2. Pop\n3. Display\n4. Exit\nEnter your choice: ");
        scanf("%d", &choice);
        switch(choice){
            case 1:
                printf("Enter value to push: ");
                scanf("%d", &value);
                push(arr_stack, arr_size, value, top);
                break;
            case 2:
                pop(arr_stack, arr_size, top);
                break;
            case 3:
                display(arr_stack, arr_size, top);
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
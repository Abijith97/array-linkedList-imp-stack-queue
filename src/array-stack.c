#include "include/array-stack.h"

int push(int *arr_stack, int arr_size, int value, int *top) {
    if (*top == -1) {
        printf("Stack Overflow\n");
        return 0;
    }
    arr_stack[(*top)--] = value;
    printf("Pushed %d to stack\n", value);
    return 0;
}

int pop(int *arr_stack, int arr_size, int *top) {
    if (*top == arr_size - 1) {
        printf("Stack Underflow\n");
        return 0;
    }
    int popped_value = arr_stack[(*top)++];
    printf("Popped %d from stack\n", popped_value);
    return 0;
}

void display(int *arr_stack, int arr_size, int *top) {
    if (*top == arr_size - 1) {
        printf("Stack is empty\n");
    }
    printf("Stack elements: ");
    for (int i = *top; i < arr_size; i++) {
        printf("%d ", arr_stack[i]);
    }
    printf("\n");
}
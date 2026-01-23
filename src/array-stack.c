#include "include/array-stack.h"

int push(int *arr_stack, int arr_size, int value, int *top) {
    if (*top == -1) {
        printf("Stack Overflow\n");
        return -1;
    }
    arr_stack[++(*top)] = value;
    printf("Pushed %d to stack\n", value);
    return 0;
}
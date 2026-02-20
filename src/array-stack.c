#include "array-stack.h"
#include <stdio.h>
#include <stdlib.h>

void push(int *arr_stack, int arr_size, int value, int *top) {
    if (*top == (arr_size - 1)) {
        printf("Stack Overflow\n");
    }
    else{
        arr_stack[++(*top)] = value;
        printf("Pushed %d to stack\n", value);
    }
}

void pop(int *arr_stack, int *top) {
    if(*top == -1){
        printf("Stack Underflow\n");
    }
    else{
        int popped_value = arr_stack[(*top)--];
        printf("Popped %d from stack\n", popped_value);
    }
}

void display(int *arr_stack, int arr_size, int *top) {
    if(*top == -1){
        printf("Stack Underflow\n");
    }
    else{
        printf("Stack elements: ");
        for(int i = *top; i >= 0; i--){
            printf("%d ", arr_stack[i]);
        }
        printf("\n");
    }
}
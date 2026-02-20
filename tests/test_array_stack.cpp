#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

// Extern C to link with C code
extern "C" {
    #include "array-stack.h"
}

class ArrayStackTest : public ::testing::Test {
protected:
    int* arr_stack;
    int* top;
    int arr_size;

    void SetUp() override {
        // Common setup for each test
        arr_size = 5;
        arr_stack = new int[arr_size];
        top = new int;
        *top = -1; // Empty stack
    }

    void TearDown() override {
        delete top;
        delete[] arr_stack;
    }
};

// Test 1: Insert 1 element and count should be 1
TEST_F(ArrayStackTest, PushOneElement) {
    // Capture stdout to verify the print message
    testing::internal::CaptureStdout();

    push(arr_stack, arr_size, 10, top);

    std::string output = testing::internal::GetCapturedStdout();

    // Verify count (top + 1 = count)
    EXPECT_EQ(*top + 1, 1) << "Count should be 1 after pushing 1 element";
    EXPECT_EQ(*top, 0) << "Top should be 0 after pushing 1 element";
    EXPECT_EQ(arr_stack[0], 10) << "First element should be 10";

    // Verify output message
    EXPECT_NE(output.find("Pushed 10 to stack"), std::string::npos)
        << "Should print success message";
}

// Test 2: Insert arr_size-1 elements and count should be arr_size-1
TEST_F(ArrayStackTest, PushArrSizeMinusOneElements) {
    testing::internal::CaptureStdout();

    // Push arr_size - 1 = 4 elements
    for (int i = 0; i < arr_size - 1; i++) {
        push(arr_stack, arr_size, i * 10, top);
    }

    std::string output = testing::internal::GetCapturedStdout();

    // Verify count
    EXPECT_EQ(*top + 1, arr_size - 1)
        << "Count should be " << (arr_size - 1) << " after pushing " << (arr_size - 1) << " elements";
    EXPECT_EQ(*top, arr_size - 2)
        << "Top should be " << (arr_size - 2) << " after pushing " << (arr_size - 1) << " elements";

    // Verify elements
    for (int i = 0; i < arr_size - 1; i++) {
        EXPECT_EQ(arr_stack[i], i * 10) << "Element at index " << i << " should be " << (i * 10);
    }

    // Verify no overflow message
    EXPECT_EQ(output.find("Stack Overflow"), std::string::npos)
        << "Should not have overflow when pushing " << (arr_size - 1) << " elements";
}

// Test 3: Insert arr_size+1 elements and it should print "Stack Overflow"
TEST_F(ArrayStackTest, PushArrSizeElementsCausesOverflow) {
    testing::internal::CaptureStdout();

    // Push arr_size = 6 elements (indices 0-4 are valid, 5th element causes overflow)
    for (int i = 0; i < arr_size+1; i++) {
        push(arr_stack, arr_size, i * 10, top);
    }

    std::string output = testing::internal::GetCapturedStdout();

    // After pushing arr_size elements:
    // - First arr_size-1 pushes succeed
    // - Last push should fail with overflow
    EXPECT_EQ(*top, arr_size - 1)
        << "Count should be " << (arr_size - 1) << " (last push should fail)";

    // Verify overflow message appears
    EXPECT_NE(output.find("Stack Overflow"), std::string::npos)
        << "Should print 'Stack Overflow' when trying to push to full stack";

    // Verify first arr_size-1 elements are correctly stored
    for (int i = 0; i < arr_size - 1; i++) {
        EXPECT_EQ(arr_stack[i], i * 10) << "Element at index " << i << " should be " << (i * 10);
    }
}

// Additional test: Verify stack overflow message exactly
TEST_F(ArrayStackTest, OverflowMessageExact) {
    // Fill the stack to capacity
    for (int i = 0; i < arr_size; i++) {
        push(arr_stack, arr_size, i, top);
    }

    // Now try to push when stack is full
    testing::internal::CaptureStdout();
    push(arr_stack, arr_size, 999, top);
    std::string output = testing::internal::GetCapturedStdout();

    // Verify exact overflow message
    EXPECT_NE(output.find("Stack Overflow"), std::string::npos)
        << "Should print 'Stack Overflow' message";

    // Verify the element was NOT added
    EXPECT_EQ(*top, arr_size - 1) << "Top should not change on overflow";
}

// Test: Pop 1 from full stack, popped element should match last pushed
TEST_F(ArrayStackTest, PopFromFullStackMatchesLastPushed) {
    // Push arr_size elements: 0, 10, 20, 30, 40
    for (int i = 0; i < arr_size; i++) {
        push(arr_stack, arr_size, i * 10, top);
    }

    int last_pushed = (arr_size - 1) * 10;

    testing::internal::CaptureStdout();
    pop(arr_stack, top);
    std::string output = testing::internal::GetCapturedStdout();

    // Verify the popped element matches the last pushed
    std::string expected = "Popped " + std::to_string(last_pushed) + " from stack";
    EXPECT_NE(output.find(expected), std::string::npos)
        << "Popped element should be " << last_pushed;

    // Verify top decremented
    EXPECT_EQ(*top, arr_size - 2) << "Top should decrement by 1 after pop";
}

// Test: Fill, empty, push 1, pop 1 — popped element matches last pushed
TEST_F(ArrayStackTest, PopAfterRefillMatchesLastPushed) {
    // Fill the stack
    for (int i = 0; i < arr_size; i++) {
        push(arr_stack, arr_size, i * 10, top);
    }

    // Empty the stack
    for (int i = 0; i < arr_size; i++) {
        pop(arr_stack, top);
    }

    // Push one new element
    int new_value = 99;
    push(arr_stack, arr_size, new_value, top);

    testing::internal::CaptureStdout();
    pop(arr_stack, top);
    std::string output = testing::internal::GetCapturedStdout();

    // Verify the popped element matches the newly pushed value
    std::string expected = "Popped " + std::to_string(new_value) + " from stack";
    EXPECT_NE(output.find(expected), std::string::npos)
        << "Popped element should be " << new_value;

    // Stack should be empty again
    EXPECT_EQ(*top, -1) << "Stack should be empty after popping the only element";
}

// Test: Push max capacity, pop max capacity + 1, last pop should print underflow
TEST_F(ArrayStackTest, PopBeyondEmptyPrintsUnderflow) {
    // Fill the stack
    for (int i = 0; i < arr_size; i++) {
        push(arr_stack, arr_size, i * 10, top);
    }

    // Pop arr_size + 1 times (one extra beyond empty)
    testing::internal::CaptureStdout();
    for (int i = 0; i < arr_size + 1; i++) {
        pop(arr_stack, top);
    }
    std::string output = testing::internal::GetCapturedStdout();

    // Verify underflow message appears
    EXPECT_NE(output.find("Stack Underflow"), std::string::npos)
        << "Should print 'Stack Underflow' when popping from empty stack";

    // Top should remain -1 (underflow should not corrupt state)
    EXPECT_EQ(*top, -1) << "Top should stay -1 after underflow";
}

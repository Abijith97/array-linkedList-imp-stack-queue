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
        top = (int*)malloc(sizeof(int));
        *top = -1; // Empty stack
    }

    void TearDown() override {
        free(top);
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

// Test 3: Insert arr_size elements and it should print "Stack Overflow"
TEST_F(ArrayStackTest, PushArrSizeElementsCausesOverflow) {
    testing::internal::CaptureStdout();

    // Push arr_size = 5 elements (indices 0-4 are valid, 5th element causes overflow)
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

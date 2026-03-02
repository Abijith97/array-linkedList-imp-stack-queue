#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <cstdlib>

// Extern C to link with C code
extern "C" {
    #include "linkedlist-stack.h"
}

/**
 * @brief Test fixture for ll_push.
 *
 * Initialises top to nullptr before each test and frees every remaining
 * node in TearDown to prevent memory leaks (ll_pop is not yet implemented).
 */
class LLPushTest : public ::testing::Test {
protected:
    struct node *top{nullptr};

    void TearDown() override {
        struct node *cur = top;
        while (cur != nullptr) {
            struct node *next = cur->next;
            free(cur);
            cur = next;
        }
        top = nullptr;
    }
};

// Test 1: Before any push, top must be NULL
TEST_F(LLPushTest, InitialTopIsNull) {
    EXPECT_EQ(top, nullptr) << "top should be NULL before any push";
}

// Test 2: Single push — top->data equals the pushed value; top->next is NULL
TEST_F(LLPushTest, SinglePushUpdatesTopCorrectly) {
    testing::internal::CaptureStdout();

    ll_push(&top, 42);

    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_NE(top, nullptr)        << "top should not be NULL after one push";
    EXPECT_EQ(top->data, 42)       << "top->data should equal the pushed value";
    EXPECT_EQ(top->next, nullptr)  << "top->next should be NULL after one push";

    EXPECT_NE(output.find("Pushed 42 to stack"), std::string::npos)
        << "Should print push confirmation message";
}

// Test 3: After 100 pushes (values 0..99), the 49th node from the top
//         (0-indexed) must hold value 50.
//         Rationale: top (index 0) holds 99; traversing k steps yields 99 - k.
//         For index 49: expected value = 99 - 49 = 50.
TEST_F(LLPushTest, HundredPushes49thNodeFromTopCorrect) {
    testing::internal::CaptureStdout();

    for (int i = 0; i < 100; ++i) {
        ll_push(&top, i);
    }

    testing::internal::GetCapturedStdout(); // discard push messages

    struct node *cur = top;
    for (int step = 0; step < 49; ++step) {
        ASSERT_NE(cur, nullptr)
            << "List is shorter than expected; failed at step " << step;
        cur = cur->next;
    }

    ASSERT_NE(cur, nullptr) << "Node at index 49 from top must not be NULL";
    EXPECT_EQ(cur->data, 50)
        << "Node at index 49 from top should hold value 50 (99 - 49)";
}

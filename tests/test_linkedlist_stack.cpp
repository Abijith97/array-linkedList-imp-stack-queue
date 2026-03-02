#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <cstdlib>

// Extern C to link with C code
extern "C" {
    #include "linkedlist-stack.h"
}

// ─── malloc injection ─────────────────────────────────────────────────────────
// The linker replaces every malloc() call in this binary with __wrap_malloc().
// __real_malloc() resolves to the original libc symbol.
// Enabled via: target_link_options(... -Wl,--wrap=malloc)
extern "C" {
    void *__real_malloc(size_t size);

    static bool g_malloc_fail = false;

    void *__wrap_malloc(size_t size)
    {
        if (g_malloc_fail) {
            return nullptr;
        }
        return __real_malloc(size);
    }
}

/** @brief RAII guard: activates malloc failure on construction, restores on destruction.
 *
 *  Guarantees g_malloc_fail is reset even if an assertion throws, preventing
 *  test-state leakage into subsequent tests.
 */
struct MallocFailGuard {
    MallocFailGuard()  { g_malloc_fail = true;  }
    ~MallocFailGuard() { g_malloc_fail = false; }
};

constexpr int TOTAL_PUSHES = 100;
constexpr int TARGET_INDEX = 49;
constexpr int EXPECTED_VALUE = TOTAL_PUSHES - TARGET_INDEX - 1; // 50

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

// Test 3: After pushing values 0..99, they appear in reverse order (LIFO)
// Index 0 (top) = 99, Index 1 = 98, ..., Index 49 = 50
TEST_F(LLPushTest, HundredPushes49thNodeFromTopCorrect) {
    testing::internal::CaptureStdout();

    for (int i = 0; i < TOTAL_PUSHES; ++i) {
        ll_push(&top, i);
    }

    testing::internal::GetCapturedStdout(); // discard push messages

    struct node *cur = top;
    for (int step = 0; step < TARGET_INDEX; ++step) {
        ASSERT_NE(cur, nullptr)
            << "List is shorter than expected; failed at step " << step;
        cur = cur->next;
    }

    ASSERT_NE(cur, nullptr) << "Node at index 49 from top must not be NULL";
    EXPECT_EQ(cur->data, EXPECTED_VALUE)
        << "Node at index 49 from top should hold value 50 (99 - 49)";
}

// Test 4: Test with negative numbers
TEST_F(LLPushTest, PushNegativeValues) {
    ll_push(&top, -42);
    ASSERT_NE(top, nullptr);
    EXPECT_EQ(top->data, -42);
}

// Test 5: Test with zero
TEST_F(LLPushTest, PushZero) {
    ll_push(&top, 0);
    ASSERT_NE(top, nullptr);
    EXPECT_EQ(top->data, 0);
}

// Test 6: Test order after multiple pushes
TEST_F(LLPushTest, VerifyLIFOOrder) {
    ll_push(&top, 1);
    ll_push(&top, 2);
    ll_push(&top, 3);
    
    EXPECT_EQ(top->data, 3);
    EXPECT_EQ(top->next->data, 2);
    EXPECT_EQ(top->next->next->data, 1);
}

// Test 7: malloc failure — top unchanged, error message printed
TEST_F(LLPushTest, PushOnMallocFailureLeavesTopUnchanged)
{
    ASSERT_EQ(top, nullptr);  // precondition: empty stack

    {
        MallocFailGuard guard;   // malloc returns nullptr for this scope only
        testing::internal::CaptureStdout();

        ll_push(&top, 99);

        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(top, nullptr)
            << "top must remain NULL when malloc fails";
        EXPECT_NE(output.find("Memory allocation failed"), std::string::npos)
            << "Should print 'Memory allocation failed' on allocation error";
    }
    // guard destructor fires here — malloc restored before GTest cleanup
}
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

// ─── LLPop Tests ─────────────────────────────────────────────────────────────

/**
 * @brief Test fixture for ll_pop.
 *
 * Provides a node_count() helper and frees all heap memory in TearDown
 * to prevent leaks regardless of how many pops each test performed.
 */
class LLPopTest : public ::testing::Test {
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

    /** @brief Return the number of nodes currently in the stack. */
    int node_count() const
    {
        int count = 0;
        for (const struct node *cur = top; cur != nullptr; cur = cur->next) {
            ++count;
        }
        return count;
    }
};

// Test 8: Pop from empty stack — prints "Stack underflow", top stays NULL
TEST_F(LLPopTest, PopEmptyStackPrintsUnderflow)
{
    testing::internal::CaptureStdout();
    ll_pop(&top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Stack is empty"), std::string::npos)
        << "Expected 'Stack is empty' when popping an empty stack";
    EXPECT_EQ(top, nullptr) << "top must remain NULL after underflow";
}

// Test 9: Push 1, pop 1 — output reports the pushed value, stack is empty after
TEST_F(LLPopTest, PopSingleElementMatchesPushed)
{
    testing::internal::CaptureStdout();
    ll_push(&top, 42);
    testing::internal::GetCapturedStdout();  // discard push output

    testing::internal::CaptureStdout();
    ll_pop(&top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Popped 42 from stack"), std::string::npos)
        << "Pop message must report the pushed value (42)";
    EXPECT_EQ(top, nullptr) << "Stack must be empty after popping the sole element";
}

// Test 10: Push 1, 2, 3; pop once — LIFO: value 3 (last pushed) removed, top becomes 2
TEST_F(LLPopTest, PopReturnsLastPushed)
{
    testing::internal::CaptureStdout();
    ll_push(&top, 1);
    ll_push(&top, 2);
    ll_push(&top, 3);
    testing::internal::GetCapturedStdout();  // discard push output

    testing::internal::CaptureStdout();
    ll_pop(&top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Popped 3 from stack"), std::string::npos)
        << "Third pushed value (3) must be the first popped (LIFO)";
    ASSERT_NE(top, nullptr);
    EXPECT_EQ(top->data, 2) << "After popping 3, new top must be 2";
    EXPECT_EQ(node_count(), 2);
}

// Test 11: Push 3 elements, pop 4 — 4th pop must print "Stack underflow"
TEST_F(LLPopTest, ExcessivePopTriggersUnderflow)
{
    testing::internal::CaptureStdout();
    ll_push(&top, 1);
    ll_push(&top, 2);
    ll_push(&top, 3);
    testing::internal::GetCapturedStdout();  // discard push output

    testing::internal::CaptureStdout();
    ll_pop(&top);
    ll_pop(&top);
    ll_pop(&top);
    testing::internal::GetCapturedStdout();  // discard three valid pops

    testing::internal::CaptureStdout();
    ll_pop(&top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Stack is empty"), std::string::npos)
        << "4th pop on a 3-element stack must report 'Stack is empty'";
    EXPECT_EQ(top, nullptr);
}

// Test 12: Push 100 elements (0–99), pop 4 times — 4th pop yields value 96
//          (the 97th pushed element, 1-indexed), 96 nodes remain.
//
// Stack layout after 100 pushes (top → bottom): 99, 98, 97, 96, 95, …, 0
//   pop 1 → 99  (100th pushed)
//   pop 2 → 98  ( 99th pushed)
//   pop 3 → 97  ( 98th pushed)
//   pop 4 → 96  ( 97th pushed)  ← the target
TEST_F(LLPopTest, Pop97thPushedElementFromHundredElementStack)
{
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        ll_push(&top, i);
    }
    testing::internal::GetCapturedStdout();  // discard push output

    // Discard first 3 pops (values 99, 98, 97).
    testing::internal::CaptureStdout();
    ll_pop(&top);
    ll_pop(&top);
    ll_pop(&top);
    testing::internal::GetCapturedStdout();

    // 4th pop must yield value 96 — the 97th pushed element.
    testing::internal::CaptureStdout();
    ll_pop(&top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Popped 96 from stack"), std::string::npos)
        << "4th pop must yield 96, the 97th pushed element (1-indexed)";
    EXPECT_EQ(node_count(), 96)
        << "96 nodes must remain after 4 pops on a 100-element stack";
    ASSERT_NE(top, nullptr);
    EXPECT_EQ(top->data, 95) << "New top must be 95 after popping 96";
}

// ─── LLDisplay Tests ─────────────────────────────────────────────────────────

/**
 * @brief Test fixture for ll_display.
 *
 * Identical setup/teardown to LLPopTest; kept separate to isolate display
 * concerns and allow independent evolution of each fixture.
 */
class LLDisplayTest : public ::testing::Test {
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

// Test 13: Display empty stack — prints "Stack is empty"
TEST_F(LLDisplayTest, DisplayEmptyStackPrintsEmpty)
{
    testing::internal::CaptureStdout();
    ll_display(top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Stack is empty"), std::string::npos)
        << "Expected 'Stack is empty' when displaying an empty stack";
}

// Test 14: Push 1, pop 1, display — stack is empty, prints "Stack is empty"
TEST_F(LLDisplayTest, DisplayAfterPushPopPrintsEmpty)
{
    testing::internal::CaptureStdout();
    ll_push(&top, 42);
    ll_pop(&top);
    testing::internal::GetCapturedStdout();  // discard push/pop output

    testing::internal::CaptureStdout();
    ll_display(top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Stack is empty"), std::string::npos)
        << "Expected 'Stack is empty' after pushing and popping the sole element";
}

// Test 15: Push 1, 2, 3; pop 1 — display shows 2 then 1 (top to bottom)
TEST_F(LLDisplayTest, DisplayThreePushOnePopShowsTopToBottom)
{
    testing::internal::CaptureStdout();
    ll_push(&top, 1);
    ll_push(&top, 2);
    ll_push(&top, 3);
    ll_pop(&top);
    testing::internal::GetCapturedStdout();  // discard push/pop output

    // Stack: 2(top) → 1 → NULL
    testing::internal::CaptureStdout();
    ll_display(top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "2\n1\n")
        << "Display must print top-to-bottom: 2, then 1";
}

// Test 16: Push 100 elements (0–99), pop 4 — display 96 elements (95 down to 0)
//
// Stack after 4 pops (top → bottom): 95, 94, …, 1, 0
TEST_F(LLDisplayTest, DisplayAfterHundredPushFourPop)
{
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        ll_push(&top, i);
    }
    ll_pop(&top);  // removes 99
    ll_pop(&top);  // removes 98
    ll_pop(&top);  // removes 97
    ll_pop(&top);  // removes 96
    testing::internal::GetCapturedStdout();  // discard push/pop output

    // Build expected output: "95\n94\n…\n1\n0\n"
    std::string expected;
    for (int i = 95; i >= 0; --i) {
        expected += std::to_string(i) + "\n";
    }

    testing::internal::CaptureStdout();
    ll_display(top);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, expected)
        << "Display must print 96 elements from 95 (top) down to 0 (bottom)";
}

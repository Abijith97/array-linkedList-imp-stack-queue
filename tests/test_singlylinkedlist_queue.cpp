#include <gtest/gtest.h>
#include <climits>
#include <cstdlib>
#include <string>

extern "C" {
    #include "singlylinkedlist-queue.h"
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

/** @brief RAII guard: activates malloc failure on construction, restores on destruction. */
struct MallocFailGuard {
    MallocFailGuard()  { g_malloc_fail = true;  }
    ~MallocFailGuard() { g_malloc_fail = false; }
};

// ─── Test fixture ─────────────────────────────────────────────────────────────

/**
 * @brief Test fixture for sll_enqueue.
 *
 * Initialises head to nullptr before each test.  TearDown traverses the full
 * list and frees every node — correct regardless of how many enqueues a test
 * performed and whether any of them failed.
 */
class SLLEnqueueTest : public ::testing::Test {
protected:
    SinglyLinkedList *head{nullptr};

    void TearDown() override {
        SinglyLinkedList *cur = head;
        while (cur != nullptr) {
            SinglyLinkedList *next = cur->next;
            free(cur);
            cur = next;
        }
        head = nullptr;
    }

    /** @brief Return the number of nodes currently in the list. */
    int node_count() const
    {
        int count = 0;
        for (const SinglyLinkedList *cur = head; cur != nullptr; cur = cur->next) {
            ++count;
        }
        return count;
    }

    /**
     * @brief Return pointer to the node at 0-indexed @p index, or nullptr if
     *        the list is shorter than @p index + 1.
     */
    const SinglyLinkedList *node_at(int index) const
    {
        const SinglyLinkedList *cur = head;
        for (int i = 0; i < index && cur != nullptr; ++i) {
            cur = cur->next;
        }
        return cur;
    }
};

// ─── Initial state ────────────────────────────────────────────────────────────

// Test 1: Before any enqueue, head must be NULL
TEST_F(SLLEnqueueTest, InitialHeadIsNull)
{
    EXPECT_EQ(head, nullptr) << "head must be NULL before any enqueue";
}

// ─── Happy path ───────────────────────────────────────────────────────────────

// Test 2: Enqueue into empty list — head points to new node, data correct,
//         next is NULL, confirmation message printed
TEST_F(SLLEnqueueTest, EnqueueIntoEmptyList_HeadPointsToNewNode)
{
    // Arrange: empty list (head == nullptr)
    testing::internal::CaptureStdout();

    // Act
    sll_enqueue(&head, 42);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_NE(head, nullptr)       << "head must not be NULL after first enqueue";
    EXPECT_EQ(head->data, 42)      << "head->data must equal the enqueued value";
    EXPECT_EQ(head->next, nullptr) << "head->next must be NULL after a single enqueue";
    EXPECT_NE(output.find("Enqueued 42"), std::string::npos)
        << "Must print 'Enqueued 42' on success";
}

// Test 3: Enqueue second element — FIFO: appended at tail, head unchanged
TEST_F(SLLEnqueueTest, EnqueueSecondElement_AppendedAtTail_HeadUnchanged)
{
    // Arrange
    testing::internal::CaptureStdout();
    sll_enqueue(&head, 10);

    // Act
    sll_enqueue(&head, 20);
    testing::internal::GetCapturedStdout();  // discard output

    // Assert
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, 10)             << "head must still hold the first enqueued value";
    ASSERT_NE(head->next, nullptr);
    EXPECT_EQ(head->next->data, 20)       << "tail must hold the second enqueued value";
    EXPECT_EQ(head->next->next, nullptr)  << "tail->next must be NULL";
}

// Test 4: Enqueue three elements — FIFO insertion order preserved head → tail
TEST_F(SLLEnqueueTest, EnqueueThreeElements_PreservesFIFOOrder)
{
    // Arrange + Act
    testing::internal::CaptureStdout();
    sll_enqueue(&head, 1);
    sll_enqueue(&head, 2);
    sll_enqueue(&head, 3);
    testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data,                   1) << "1st node must hold 1";
    EXPECT_EQ(head->next->data,             2) << "2nd node must hold 2";
    EXPECT_EQ(head->next->next->data,       3) << "3rd node must hold 3";
    EXPECT_EQ(head->next->next->next, nullptr) << "tail->next must be NULL";
}

// Test 5: Node count matches the number of successful enqueues
TEST_F(SLLEnqueueTest, EnqueueFiveElements_NodeCountIsFive)
{
    testing::internal::CaptureStdout();
    for (int i = 0; i < 5; ++i) {
        sll_enqueue(&head, i * 10);
    }
    testing::internal::GetCapturedStdout();

    EXPECT_EQ(node_count(), 5)
        << "node_count() must equal the number of successful enqueues";
}

// ─── Boundary / edge values ───────────────────────────────────────────────────

// Test 6: Enqueue a negative value — stored and confirmed correctly
TEST_F(SLLEnqueueTest, EnqueueNegativeValue_StoredCorrectly)
{
    testing::internal::CaptureStdout();
    sll_enqueue(&head, -42);
    const std::string output = testing::internal::GetCapturedStdout();

    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, -42) << "Negative value must be stored correctly";
    EXPECT_NE(output.find("Enqueued -42"), std::string::npos)
        << "Must print 'Enqueued -42'";
}

// Test 7: Enqueue zero — stored correctly
TEST_F(SLLEnqueueTest, EnqueueZero_StoredCorrectly)
{
    testing::internal::CaptureStdout();
    sll_enqueue(&head, 0);
    testing::internal::GetCapturedStdout();

    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, 0) << "Zero must be stored correctly";
}

// Test 8: Enqueue INT_MAX then INT_MIN — both boundary values stored correctly
TEST_F(SLLEnqueueTest, EnqueueIntMaxThenIntMin_BothStoredCorrectly)
{
    testing::internal::CaptureStdout();
    sll_enqueue(&head, INT_MAX);
    sll_enqueue(&head, INT_MIN);
    testing::internal::GetCapturedStdout();

    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, INT_MAX)        << "First node must hold INT_MAX";
    ASSERT_NE(head->next, nullptr);
    EXPECT_EQ(head->next->data, INT_MIN)  << "Second node must hold INT_MIN";
    EXPECT_EQ(head->next->next, nullptr)  << "tail->next must be NULL";
}

// Test 9: Enqueue 100 elements (values 0–99) — total count is 100,
//         node at 0-indexed position 49 holds value 49.
//         Queue layout: head(0) → 1 → 2 → … → 49 → … → 99(tail)
TEST_F(SLLEnqueueTest, Enqueue100Elements_NodeAtIndex49HoldsValue49)
{
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        sll_enqueue(&head, i);
    }
    testing::internal::GetCapturedStdout();

    EXPECT_EQ(node_count(), 100) << "List must contain 100 nodes after 100 enqueues";

    const SinglyLinkedList *target = node_at(49);
    ASSERT_NE(target, nullptr) << "Node at index 49 must exist";
    EXPECT_EQ(target->data, 49)
        << "Node at 0-indexed position 49 from head must hold value 49";
}

// ─── Error conditions ─────────────────────────────────────────────────────────

// Test 10: malloc failure on empty list — head remains NULL, error message printed
TEST_F(SLLEnqueueTest, EnqueueOnMallocFailure_EmptyList_HeadRemainsNull)
{
    ASSERT_EQ(head, nullptr);  // precondition

    {
        MallocFailGuard guard;
        testing::internal::CaptureStdout();

        sll_enqueue(&head, 99);

        const std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(head, nullptr)
            << "head must remain NULL when malloc fails on an empty list";
        EXPECT_NE(output.find("Memory allocation failed"), std::string::npos)
            << "Must print 'Memory allocation failed' on allocation failure";
    }
}

// Test 11: malloc failure on non-empty list — existing nodes untouched,
//          count unchanged, error message printed
TEST_F(SLLEnqueueTest, EnqueueOnMallocFailure_NonEmptyList_ListUnchanged)
{
    // Arrange: two nodes already in the list
    testing::internal::CaptureStdout();
    sll_enqueue(&head, 1);
    sll_enqueue(&head, 2);
    testing::internal::GetCapturedStdout();

    ASSERT_EQ(node_count(), 2);  // precondition

    {
        MallocFailGuard guard;
        testing::internal::CaptureStdout();

        // Act
        sll_enqueue(&head, 3);

        const std::string output = testing::internal::GetCapturedStdout();

        // Assert: list structure unchanged
        EXPECT_EQ(node_count(), 2)
            << "Node count must not change when malloc fails";
        EXPECT_EQ(head->data, 1)
            << "head must remain unchanged on allocation failure";
        EXPECT_EQ(head->next->data, 2)
            << "tail must remain unchanged on allocation failure";
        EXPECT_NE(output.find("Memory allocation failed"), std::string::npos)
            << "Must print 'Memory allocation failed' on allocation failure";
    }
}

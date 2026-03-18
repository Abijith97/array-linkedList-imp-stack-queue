#include <gtest/gtest.h>
#include <climits>
#include <cstdlib>
#include <initializer_list>
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

// ─── SLLDequeue fixture ───────────────────────────────────────────────────────

/**
 * @brief Test fixture for sll_dequeue.
 *
 * Provides enqueue_values() as a test-data builder for expressive AAA setup.
 * TearDown frees all remaining nodes so tests that dequeue partially do not
 * leak memory.
 */
class SLLDequeueTest : public ::testing::Test {
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
     * @brief Test-data builder: enqueue a braced list of values in order.
     *
     * Discards stdout so callers only capture output produced by the Act step.
     * Example: enqueue_values({1, 2, 3}) → head(1) → 2 → 3(tail)
     */
    void enqueue_values(std::initializer_list<int> values)
    {
        testing::internal::CaptureStdout();
        for (int v : values) {
            sll_enqueue(&head, v);
        }
        testing::internal::GetCapturedStdout();
    }
};

// ─── Error condition ──────────────────────────────────────────────────────────

// Test 12: Dequeue from empty queue — prints "Queue is empty"
TEST_F(SLLDequeueTest, DequeueFromEmptyQueue_PrintsQueueIsEmpty)
{
    // Arrange: head == nullptr (fixture default)
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_NE(output.find("Queue is empty"), std::string::npos)
        << "Must print 'Queue is empty' when dequeuing from an empty queue";
}

// Test 13: Dequeue from empty queue — head remains NULL (no state corruption)
TEST_F(SLLDequeueTest, DequeueFromEmptyQueue_HeadRemainsNull)
{
    // Arrange
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(head, nullptr)
        << "head must remain NULL after dequeuing from an empty queue";
}

// ─── Single-element queue ─────────────────────────────────────────────────────

// Test 14: Dequeue sole element — head becomes NULL (queue is empty)
TEST_F(SLLDequeueTest, DequeueSoleElement_HeadBecomesNull)
{
    // Arrange
    enqueue_values({42});
    ASSERT_NE(head, nullptr);  // precondition

    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(head, nullptr)
        << "head must be NULL after dequeuing the only element";
}

// Test 15: Dequeue sole element — prints confirmation message with correct value
TEST_F(SLLDequeueTest, DequeueSoleElement_PrintsDequeueMessage)
{
    // Arrange
    enqueue_values({42});
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_NE(output.find("Dequeued 42 from queue"), std::string::npos)
        << "Must print 'Dequeued 42 from queue' when dequeuing value 42";
}

// Test 16: Dequeue sole element — node count drops to zero
TEST_F(SLLDequeueTest, DequeueSoleElement_NodeCountIsZero)
{
    // Arrange
    enqueue_values({99});
    ASSERT_EQ(node_count(), 1);

    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(node_count(), 0)
        << "Node count must be 0 after dequeuing the only element";
}

// ─── Two-element queue ────────────────────────────────────────────────────────

// Test 17: Dequeue from two-element queue — removes the first enqueued value (FIFO)
TEST_F(SLLDequeueTest, DequeueFromTwoElements_RemovesFirstEnqueuedValue)
{
    // Arrange: head(10) → 20(tail)
    enqueue_values({10, 20});
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_NE(output.find("Dequeued 10 from queue"), std::string::npos)
        << "FIFO: first enqueued value (10) must be dequeued first";
}

// Test 18: Dequeue from two-element queue — head advances to second node
TEST_F(SLLDequeueTest, DequeueFromTwoElements_HeadAdvancesToSecondNode)
{
    // Arrange: head(10) → 20(tail)
    enqueue_values({10, 20});
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_NE(head, nullptr)   << "head must not be NULL after one dequeue from two-element queue";
    EXPECT_EQ(head->data, 20)  << "head must now point to the second enqueued element (20)";
}

// Test 19: Dequeue from two-element queue — new tail->next is NULL
TEST_F(SLLDequeueTest, DequeueFromTwoElements_NewTailNextIsNull)
{
    // Arrange
    enqueue_values({10, 20});
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->next, nullptr)
        << "Remaining single node's next must be NULL after dequeue";
}

// Test 20: Dequeue from two-element queue — node count decrements by one
TEST_F(SLLDequeueTest, DequeueFromTwoElements_NodeCountDecrementsByOne)
{
    // Arrange
    enqueue_values({10, 20});
    ASSERT_EQ(node_count(), 2);

    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(node_count(), 1)
        << "Node count must decrement from 2 to 1 after one dequeue";
}

// ─── FIFO order across multiple dequeues ─────────────────────────────────────

// Test 21: Dequeue three times from three-element queue — strict FIFO order
//          Enqueue: 1, 2, 3  →  Dequeue order must be: 1, 2, 3
TEST_F(SLLDequeueTest, DequeueThreeElements_PreservesFIFOOrder)
{
    // Arrange
    enqueue_values({1, 2, 3});

    for (int expected : {1, 2, 3}) {
        testing::internal::CaptureStdout();

        // Act
        sll_dequeue(&head);
        const std::string output = testing::internal::GetCapturedStdout();

        // Assert
        const std::string msg = "Dequeued " + std::to_string(expected) + " from queue";
        EXPECT_NE(output.find(msg), std::string::npos)
            << "Expected '" << msg << "' but got: " << output;
    }
}

// ─── Drain to empty ───────────────────────────────────────────────────────────

// Test 22: Dequeue all elements — queue becomes empty (head == NULL)
TEST_F(SLLDequeueTest, DequeueAllElements_QueueBecomesEmpty)
{
    // Arrange
    enqueue_values({5, 10, 15});
    ASSERT_EQ(node_count(), 3);

    testing::internal::CaptureStdout();

    // Act: drain the queue
    sll_dequeue(&head);
    sll_dequeue(&head);
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(head, nullptr)    << "head must be NULL after dequeuing all elements";
    EXPECT_EQ(node_count(), 0)  << "Node count must be 0 after draining the queue";
}

// Test 23: Dequeue one beyond empty — prints "Queue is empty" after draining
TEST_F(SLLDequeueTest, DequeueOneBeyondEmpty_PrintsQueueIsEmpty)
{
    // Arrange: drain a two-element queue
    enqueue_values({7, 8});
    testing::internal::CaptureStdout();
    sll_dequeue(&head);
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    ASSERT_EQ(head, nullptr);  // precondition: queue is empty

    testing::internal::CaptureStdout();

    // Act: one extra dequeue
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_NE(output.find("Queue is empty"), std::string::npos)
        << "Must print 'Queue is empty' when dequeuing from an already-drained queue";
}

// ─── Boundary values ─────────────────────────────────────────────────────────

// Test 24: Dequeue INT_MAX — confirmation message reports correct value
TEST_F(SLLDequeueTest, DequeueIntMaxValue_PrintsCorrectMessage)
{
    // Arrange
    enqueue_values({INT_MAX});
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    const std::string expected = "Dequeued " + std::to_string(INT_MAX) + " from queue";
    EXPECT_NE(output.find(expected), std::string::npos)
        << "Must report INT_MAX correctly in dequeue message";
}

// Test 25: Dequeue INT_MIN — confirmation message reports correct value
TEST_F(SLLDequeueTest, DequeueIntMinValue_PrintsCorrectMessage)
{
    // Arrange
    enqueue_values({INT_MIN});
    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    const std::string expected = "Dequeued " + std::to_string(INT_MIN) + " from queue";
    EXPECT_NE(output.find(expected), std::string::npos)
        << "Must report INT_MIN correctly in dequeue message";
}

// ─── Scale ────────────────────────────────────────────────────────────────────

// Test 26: Enqueue 100 elements (0–99), dequeue once — removes value 0 (FIFO head)
TEST_F(SLLDequeueTest, DequeueFromHundredElements_RemovesFirstEnqueued)
{
    // Arrange: head(0) → 1 → … → 99(tail)
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        sll_enqueue(&head, i);
    }
    testing::internal::GetCapturedStdout();

    ASSERT_EQ(node_count(), 100);

    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_NE(output.find("Dequeued 0 from queue"), std::string::npos)
        << "First enqueued element (0) must be dequeued first from 100-element queue";
    EXPECT_EQ(node_count(), 99) << "99 nodes must remain after one dequeue";
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, 1)    << "New head must be 1 after dequeuing 0";
}

// Test 27: Enqueue 100 elements, dequeue all 100 — queue is empty, count is zero
TEST_F(SLLDequeueTest, DequeueAllHundredElements_QueueBecomesEmpty)
{
    // Arrange
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        sll_enqueue(&head, i);
    }
    testing::internal::GetCapturedStdout();

    // Act
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        sll_dequeue(&head);
    }
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(head, nullptr)    << "head must be NULL after dequeuing all 100 elements";
    EXPECT_EQ(node_count(), 0)  << "Node count must be 0 after draining a 100-element queue";
}

// ─── Interleaved enqueue / dequeue ───────────────────────────────────────────

// ─── SLLDisplay fixture ───────────────────────────────────────────────────────

/**
 * @brief Test fixture for sll_display.
 *
 * Provides enqueue_values() builder, node_count() structural helper, and
 * build_expected_output() to generate the exact stdout string expected for a
 * given sequence of values.  TearDown frees all nodes regardless of whether
 * any dequeue was performed.
 */
class SLLDisplayTest : public ::testing::Test {
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

    /** @brief Test-data builder: enqueue values in order, discarding stdout. */
    void enqueue_values(std::initializer_list<int> values)
    {
        testing::internal::CaptureStdout();
        for (int v : values) {
            sll_enqueue(&head, v);
        }
        testing::internal::GetCapturedStdout();
    }

    /**
     * @brief Build the exact stdout string expected from sll_display for the
     *        given ordered sequence of values (head → tail).
     *
     * Each value is formatted as "%d\n", matching printf("%d\n", temp->data).
     */
    static std::string build_expected_output(std::initializer_list<int> values)
    {
        std::string expected;
        for (int v : values) {
            expected += std::to_string(v) + "\n";
        }
        return expected;
    }
};

// ─── Empty queue ──────────────────────────────────────────────────────────────

// Test 29: Display on empty queue — prints "Queue is empty"
TEST_F(SLLDisplayTest, DisplayEmptyQueue_PrintsQueueIsEmpty)
{
    // Arrange: head == nullptr (fixture default)
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, "Queue is empty\n")
        << "Exact output must be 'Queue is empty\\n' for an empty queue";
}

// Test 30: Display on empty queue — head remains NULL (display is non-destructive)
TEST_F(SLLDisplayTest, DisplayEmptyQueue_HeadRemainsNull)
{
    // Arrange
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(head, nullptr)
        << "head must remain NULL after displaying an empty queue";
}

// ─── Single-element queue ─────────────────────────────────────────────────────

// Test 31: Display single element — output is exactly "<value>\n"
TEST_F(SLLDisplayTest, DisplaySingleElement_OutputExactValue)
{
    // Arrange
    enqueue_values({42});
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, "42\n")
        << "Single-element display must output exactly '42\\n'";
}

// Test 32: Display single element — list structure unchanged (non-destructive)
TEST_F(SLLDisplayTest, DisplaySingleElement_ListUnchangedAfterDisplay)
{
    // Arrange
    enqueue_values({42});
    const int count_before = node_count();
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(node_count(), count_before)
        << "Node count must be unchanged after display";
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, 42)
        << "head->data must be unchanged after display";
}

// ─── Multi-element queue — FIFO output order ──────────────────────────────────

// Test 33: Display two elements — output is head-to-tail (FIFO) order
TEST_F(SLLDisplayTest, DisplayTwoElements_OutputInFIFOOrder)
{
    // Arrange: head(10) → 20(tail)
    enqueue_values({10, 20});
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, build_expected_output({10, 20}))
        << "Two-element display must print head first: '10\\n20\\n'";
}

// Test 34: Display three elements — strict FIFO output order (1 → 2 → 3)
TEST_F(SLLDisplayTest, DisplayThreeElements_StrictFIFOOutputOrder)
{
    // Arrange
    enqueue_values({1, 2, 3});
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, build_expected_output({1, 2, 3}))
        << "Three-element display must print in enqueue order: '1\\n2\\n3\\n'";
}

// Test 35: Display three elements — list structure unchanged after display
TEST_F(SLLDisplayTest, DisplayThreeElements_ListUnchangedAfterDisplay)
{
    // Arrange
    enqueue_values({1, 2, 3});
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    testing::internal::GetCapturedStdout();

    // Assert: verify head, middle, and tail are intact
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data,             1) << "head->data must be unchanged";
    EXPECT_EQ(head->next->data,       2) << "middle node must be unchanged";
    EXPECT_EQ(head->next->next->data, 3) << "tail->data must be unchanged";
    EXPECT_EQ(head->next->next->next, nullptr) << "tail->next must remain NULL";
}

// ─── Boundary values ─────────────────────────────────────────────────────────

// Test 36: Display negative value — printed correctly
TEST_F(SLLDisplayTest, DisplayNegativeValue_PrintedCorrectly)
{
    enqueue_values({-42});
    testing::internal::CaptureStdout();

    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "-42\n")
        << "Negative value must be printed correctly";
}

// Test 37: Display zero — printed correctly
TEST_F(SLLDisplayTest, DisplayZero_PrintedCorrectly)
{
    enqueue_values({0});
    testing::internal::CaptureStdout();

    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "0\n")
        << "Zero must be printed correctly";
}

// Test 38: Display INT_MAX then INT_MIN — both boundary values in correct order
TEST_F(SLLDisplayTest, DisplayIntMaxThenIntMin_BothPrintedInOrder)
{
    enqueue_values({INT_MAX, INT_MIN});
    testing::internal::CaptureStdout();

    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    const std::string expected = std::to_string(INT_MAX) + "\n"
                               + std::to_string(INT_MIN) + "\n";
    EXPECT_EQ(output, expected)
        << "INT_MAX and INT_MIN must both appear in enqueue order";
}

// ─── Non-destructive / idempotent behaviour ───────────────────────────────────

// Test 39: Display called twice — both calls produce identical output
TEST_F(SLLDisplayTest, DisplayCalledTwice_OutputIsIdentical)
{
    // Arrange
    enqueue_values({5, 10, 15});

    testing::internal::CaptureStdout();
    sll_display(head);
    const std::string first_output = testing::internal::GetCapturedStdout();

    testing::internal::CaptureStdout();

    // Act: second display call
    sll_display(head);
    const std::string second_output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(first_output, second_output)
        << "sll_display must be idempotent: repeated calls produce identical output";
}

// Test 40: Display after dequeue — shows remaining elements only
TEST_F(SLLDisplayTest, DisplayAfterDequeue_ShowsOnlyRemainingElements)
{
    // Arrange: head(1) → 2 → 3, dequeue removes 1
    enqueue_values({1, 2, 3});
    testing::internal::CaptureStdout();
    sll_dequeue(&head);
    testing::internal::GetCapturedStdout();

    // Queue is now: head(2) → 3(tail)
    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, build_expected_output({2, 3}))
        << "Display must reflect post-dequeue state: only '2\\n3\\n'";
    EXPECT_EQ(output.find("1"), std::string::npos)
        << "Dequeued value (1) must not appear in display output";
}

// ─── Scale ────────────────────────────────────────────────────────────────────

// Test 41: Display 100 elements (0–99) — all values appear in exact FIFO order
TEST_F(SLLDisplayTest, Display100Elements_ExactFIFOOutput)
{
    // Arrange: build expected string before enqueuing to keep it readable
    std::string expected;
    for (int i = 0; i < 100; ++i) {
        expected += std::to_string(i) + "\n";
    }

    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        sll_enqueue(&head, i);
    }
    testing::internal::GetCapturedStdout();

    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, expected)
        << "100-element display must print values 0–99 in enqueue (FIFO) order";
}

// Test 42: Display 100 elements — list count unchanged after display (non-destructive)
TEST_F(SLLDisplayTest, Display100Elements_NodeCountUnchangedAfterDisplay)
{
    testing::internal::CaptureStdout();
    for (int i = 0; i < 100; ++i) {
        sll_enqueue(&head, i);
    }
    testing::internal::GetCapturedStdout();

    ASSERT_EQ(node_count(), 100);  // precondition

    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(node_count(), 100)
        << "Node count must be 100 after displaying a 100-element queue";
}

// ─── Interleaved operations ───────────────────────────────────────────────────

// Test 43: Enqueue 5, dequeue 2, enqueue 2 more, display — reflects exact state
//
// Timeline:
//   enqueue 10,20,30,40,50  →  head(10) → 20 → 30 → 40 → 50
//   dequeue                 →  removes 10, head(20) → 30 → 40 → 50
//   dequeue                 →  removes 20, head(30) → 40 → 50
//   enqueue 60, 70          →  head(30) → 40 → 50 → 60 → 70
//   display                 →  must print: 30, 40, 50, 60, 70
TEST_F(SLLDisplayTest, DisplayAfterInterleavedOps_ReflectsCurrentQueueState)
{
    // Arrange
    enqueue_values({10, 20, 30, 40, 50});
    testing::internal::CaptureStdout();
    sll_dequeue(&head);  // removes 10
    sll_dequeue(&head);  // removes 20
    testing::internal::GetCapturedStdout();

    enqueue_values({60, 70});
    // Queue: head(30) → 40 → 50 → 60 → 70(tail)
    ASSERT_EQ(node_count(), 5);

    testing::internal::CaptureStdout();

    // Act
    sll_display(head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_EQ(output, build_expected_output({30, 40, 50, 60, 70}))
        << "Display must reflect current queue state after interleaved ops";
}

// Test 28: Enqueue 3, dequeue 2, enqueue 2 more — FIFO order maintained throughout
//
// Timeline:
//   enqueue 10, 20, 30  →  head(10) → 20 → 30
//   dequeue              →  removes 10,  head(20) → 30
//   dequeue              →  removes 20,  head(30)
//   enqueue 40, 50       →  head(30) → 40 → 50
//   dequeue              →  must remove 30 (oldest surviving element)
TEST_F(SLLDequeueTest, InterleavedEnqueueDequeue_MaintainsFIFOOrder)
{
    // Arrange
    enqueue_values({10, 20, 30});
    testing::internal::CaptureStdout();
    sll_dequeue(&head);  // removes 10
    sll_dequeue(&head);  // removes 20
    testing::internal::GetCapturedStdout();

    enqueue_values({40, 50});
    // Queue now: head(30) → 40 → 50(tail)
    ASSERT_EQ(node_count(), 3);

    testing::internal::CaptureStdout();

    // Act
    sll_dequeue(&head);
    const std::string output = testing::internal::GetCapturedStdout();

    // Assert
    EXPECT_NE(output.find("Dequeued 30 from queue"), std::string::npos)
        << "After interleaved operations, oldest surviving element (30) must dequeue next";
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->data, 40) << "New head must be 40 after dequeuing 30";
    EXPECT_EQ(node_count(), 2);
}

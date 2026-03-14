/**
 * @file test_circularbuffer_queue.cpp
 * @brief GoogleTest suite for cb_enqueue and cb_dequeue (circular-buffer queue).
 *
 * Allocation/free of the CircularBuffer follows the two-malloc contract
 * established in main.c: one malloc for the control block, one for arr.
 * TearDown mirrors the matching two-free sequence so Valgrind stays clean.
 */

#include <gtest/gtest.h>

extern "C" {
    #include "circularbuffer-queue.h"
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

/**
 * @brief Compute the number of elements currently stored in the queue.
 *
 * Handles the sentinel-based empty state (front == rear == -1) and
 * the wrap-around case using the ring formula.
 *
 * @param cb Read-only pointer to an initialised CircularBuffer.
 * @return   Number of elements in [0, cb->size].
 */
static int queue_count(const CircularBuffer *cb)
{
    if (cb->front == -1 && cb->rear == -1) {
        return 0;
    }
    return (cb->rear - cb->front + cb->size) % cb->size + 1;
}

// ─── Fixture ─────────────────────────────────────────────────────────────────

/**
 * @brief Test fixture for cb_enqueue.
 *
 * allocate() initialises a CircularBuffer of a given capacity.
 * TearDown frees arr then the control block, matching the two-malloc contract.
 */
class CBEnqueueTest : public ::testing::Test {
protected:
    CircularBuffer *cb{nullptr};

    /**
     * @brief Allocate and initialise a CircularBuffer with @p size slots.
     *
     * Must be called explicitly inside each test rather than in SetUp so
     * that each test can choose its own capacity.
     *
     * @param size Desired queue capacity (number of slots).
     */
    void allocate(int size)
    {
        cb = static_cast<CircularBuffer *>(malloc(sizeof(CircularBuffer)));
        ASSERT_NE(cb, nullptr) << "Control-block allocation failed";

        cb->arr = static_cast<int *>(malloc(static_cast<size_t>(size) * sizeof(int)));
        ASSERT_NE(cb->arr, nullptr) << "Array allocation failed";

        cb->size  = size;
        cb->front = -1;
        cb->rear  = -1;
    }

    void TearDown() override
    {
        if (cb != nullptr) {
            free(cb->arr);
            free(cb);
            cb = nullptr;
        }
    }
};

// ─── Tests ───────────────────────────────────────────────────────────────────

// Test 1: size=5, enqueue 1 element — queue holds exactly 1 element
//
// After the first enqueue the sentinel branch sets front = rear = 0.
// queue_count() must return 1.
TEST_F(CBEnqueueTest, SingleEnqueueCountIsOne)
{
    allocate(5);

    testing::internal::CaptureStdout();
    cb_enqueue(cb, 42);
    testing::internal::GetCapturedStdout();  // discard "Enqueued 42"

    EXPECT_EQ(queue_count(cb), 1)
        << "Queue must hold exactly 1 element after a single enqueue";
    EXPECT_EQ(cb->front, 0) << "front must be 0 after first enqueue";
    EXPECT_EQ(cb->rear,  0) << "rear must be 0 after first enqueue";
    EXPECT_EQ(cb->arr[0], 42) << "arr[0] must store the enqueued value";
}

// Test 2: size=5, fill to capacity, then enqueue one more — prints "Queue is full"
//
// With size=5 and sentinel-based empty detection the ring can hold 5 elements
// (indices 0..4).  The 6th enqueue triggers (rear+1)%size == front and must
// print "Queue is full" without modifying the queue state.
TEST_F(CBEnqueueTest, EnqueueBeyondCapacityPrintsQueueFull)
{
    constexpr int CAPACITY = 5;
    allocate(CAPACITY);

    // Fill all slots — discard confirmation output
    testing::internal::CaptureStdout();
    for (int i = 1; i <= CAPACITY; ++i) {
        cb_enqueue(cb, i);
    }
    testing::internal::GetCapturedStdout();

    // Attempt one more enqueue — must print "Queue is full"
    testing::internal::CaptureStdout();
    cb_enqueue(cb, 99);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Queue is full"), std::string::npos)
        << "Expected 'Queue is full' when enqueuing into a full queue";

    // Queue state must be unchanged
    EXPECT_EQ(queue_count(cb), CAPACITY)
        << "Queue count must remain at capacity after a rejected enqueue";
}

// ─── CBDequeueTest ────────────────────────────────────────────────────────────

/**
 * @brief Test fixture for cb_dequeue.
 *
 * Identical allocation contract to CBEnqueueTest: two mallocs in allocate(),
 * two matching frees in TearDown().  Kept as a separate fixture so dequeue
 * tests can evolve independently of enqueue tests (SRP).
 */
class CBDequeueTest : public ::testing::Test {
protected:
    CircularBuffer *cb{nullptr};

    /** @brief Allocate and initialise a CircularBuffer with @p size slots. */
    void allocate(int size)
    {
        cb = static_cast<CircularBuffer *>(malloc(sizeof(CircularBuffer)));
        ASSERT_NE(cb, nullptr) << "Control-block allocation failed";

        cb->arr = static_cast<int *>(malloc(static_cast<size_t>(size) * sizeof(int)));
        ASSERT_NE(cb->arr, nullptr) << "Array allocation failed";

        cb->size  = size;
        cb->front = -1;
        cb->rear  = -1;
    }

    void TearDown() override
    {
        if (cb != nullptr) {
            free(cb->arr);
            free(cb);
            cb = nullptr;
        }
    }
};

// Test 3: size=5, dequeue on empty queue — prints "Queue is empty"
TEST_F(CBDequeueTest, DequeueEmptyPrintsQueueEmpty)
{
    allocate(5);

    testing::internal::CaptureStdout();
    cb_dequeue(cb);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Queue is empty"), std::string::npos)
        << "Expected 'Queue is empty' when dequeuing from an empty queue";
}

// Test 4: size=5, enqueue 1 element, dequeue 1 — output matches enqueued value
//
// FIFO guarantee: the single enqueued value (42) must be the dequeued value.
TEST_F(CBDequeueTest, DequeueAfterOneEnqueueMatchesValue)
{
    allocate(5);

    testing::internal::CaptureStdout();
    cb_enqueue(cb, 42);
    testing::internal::GetCapturedStdout();  // discard enqueue output

    testing::internal::CaptureStdout();
    cb_dequeue(cb);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "Dequeued 42\n")
        << "Dequeued value must match the single enqueued element";
}

// Test 5: size=5, enqueue 1, dequeue twice — second dequeue prints "Queue is empty"
TEST_F(CBDequeueTest, DequeueMoreThanEnqueuedPrintsEmpty)
{
    allocate(5);

    testing::internal::CaptureStdout();
    cb_enqueue(cb, 10);
    cb_dequeue(cb);  // removes the sole element
    testing::internal::GetCapturedStdout();  // discard

    testing::internal::CaptureStdout();
    cb_dequeue(cb);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Queue is empty"), std::string::npos)
        << "Expected 'Queue is empty' on second dequeue from a one-element queue";
}

// Test 6: size=5, enqueue 5 elements, dequeue 1 — prints first enqueued element
//
// FIFO: elements enqueued 1..5 occupy indices 0..4.  First dequeue must
// return arr[front=0] == 1.
TEST_F(CBDequeueTest, DequeueFromFullQueueReturnsFirstEnqueued)
{
    allocate(5);

    testing::internal::CaptureStdout();
    for (int i = 1; i <= 5; ++i) {
        cb_enqueue(cb, i);
    }
    testing::internal::GetCapturedStdout();  // discard

    testing::internal::CaptureStdout();
    cb_dequeue(cb);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "Dequeued 1\n")
        << "First dequeue must return the first enqueued element (FIFO)";
}

// Test 7: size=5, enqueue 4 elements, dequeue 2 — queue count is 2
TEST_F(CBDequeueTest, DequeueReducesCountCorrectly)
{
    allocate(5);

    testing::internal::CaptureStdout();
    for (int i = 1; i <= 4; ++i) {
        cb_enqueue(cb, i);
    }
    cb_dequeue(cb);
    cb_dequeue(cb);
    testing::internal::GetCapturedStdout();  // discard

    EXPECT_EQ(queue_count(cb), 2)
        << "Queue count must be 2 after 4 enqueues and 2 dequeues";
}

// Test 8: size=5, enqueue 5, dequeue 6 — sixth dequeue prints "Queue is empty"
TEST_F(CBDequeueTest, ExcessiveDequeuePrintsEmpty)
{
    allocate(5);

    testing::internal::CaptureStdout();
    for (int i = 1; i <= 5; ++i) {
        cb_enqueue(cb, i);
    }
    for (int i = 0; i < 5; ++i) {
        cb_dequeue(cb);
    }
    testing::internal::GetCapturedStdout();  // discard

    testing::internal::CaptureStdout();
    cb_dequeue(cb);  // 6th dequeue — queue is empty
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Queue is empty"), std::string::npos)
        << "Expected 'Queue is empty' on 6th dequeue from a 5-element queue";
}

// Test 9: size=5, enqueue 5, dequeue 5, enqueue 2, dequeue 1 — returns 6th overall
//
// After draining the queue (front = rear = -1), the ring resets to its sentinel
// state.  The next enqueue restarts at index 0.  Dequeuing must return the
// first of the two new elements, i.e. the 6th element enqueued overall.
TEST_F(CBDequeueTest, DequeueAfterRefillReturnsSixthEnqueuedElement)
{
    allocate(5);

    constexpr int SIXTH_ELEMENT  = 6;
    constexpr int SEVENTH_ELEMENT = 7;

    // Fill and drain the queue
    testing::internal::CaptureStdout();
    for (int i = 1; i <= 5; ++i) {
        cb_enqueue(cb, i);
    }
    for (int i = 0; i < 5; ++i) {
        cb_dequeue(cb);
    }
    testing::internal::GetCapturedStdout();  // discard

    // Refill with two new elements
    testing::internal::CaptureStdout();
    cb_enqueue(cb, SIXTH_ELEMENT);
    cb_enqueue(cb, SEVENTH_ELEMENT);
    testing::internal::GetCapturedStdout();  // discard

    // Dequeue — must return the 6th enqueued element (FIFO)
    testing::internal::CaptureStdout();
    cb_dequeue(cb);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "Dequeued 6\n")
        << "After refill, first dequeue must return the 6th enqueued element";
}

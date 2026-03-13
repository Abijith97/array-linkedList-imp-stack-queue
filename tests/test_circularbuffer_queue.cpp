/**
 * @file test_circularbuffer_queue.cpp
 * @brief GoogleTest suite for cb_enqueue (circular-buffer queue).
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

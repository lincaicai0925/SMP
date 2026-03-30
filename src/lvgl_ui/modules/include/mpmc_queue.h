/**
 * @file mpmc_queue.h
 * @brief Lock-free bounded Multi-Producer Multi-Consumer (MPMC) queue
 *
 * Based on Dmitry Vyukov's bounded MPMC queue algorithm.
 * Each cell carries a per-cell sequence counter that acts as a state machine,
 * enabling lock-free coordination via a single CAS per enqueue/dequeue.
 *
 * Requirements:
 *   - C11 or later (uses <stdatomic.h>, _Alignas, aligned_alloc)
 *   - GCC 4.9+ / Clang 3.5+ / MSVC 2019+ (with C11 support)
 *
 * Thread safety:
 *   - MPMC_Queue_Enqueue: safe to call from multiple threads concurrently
 *   - MPMC_Queue_Dequeue: safe to call from multiple threads concurrently
 *   - MPMC_Queue_Init / MPMC_Queue_Destroy: NOT thread-safe, call from one thread
 */

#ifndef MPMC_QUEUE_H
#define MPMC_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 *  Configuration
 * --------------------------------------------------------------------------- */

/** Cache line size in bytes. Override at compile time if your platform differs. */
#ifndef MPMC_CACHE_LINE_SIZE
#define MPMC_CACHE_LINE_SIZE 64
#endif

/* ---------------------------------------------------------------------------
 *  Opaque handle
 * --------------------------------------------------------------------------- */

/** Opaque queue handle. Internal layout is hidden in the .c file. */
typedef struct MPMC_Queue MPMC_Queue;

/* ---------------------------------------------------------------------------
 *  Lifecycle
 * --------------------------------------------------------------------------- */

/**
 * @brief Create and initialize an MPMC queue.
 *
 * @param capacity  Max number of elements. MUST be a power of 2 and >= 2.
 * @return          Pointer to the queue, or NULL on failure.
 *
 * The caller owns the returned pointer and must eventually call
 * MPMC_Queue_Destroy() to release all resources.
 */
MPMC_Queue *MPMC_Queue_Init(uint64_t capacity);

/**
 * @brief Destroy the queue and free all associated memory.
 *
 * @param q  Pointer to the queue pointer. *q is set to NULL after free.
 *           If q is NULL or *q is NULL, this is a no-op.
 *
 * The caller must ensure no other threads are accessing the queue when
 * this function is called.
 */
void MPMC_Queue_Destroy(MPMC_Queue **q);

/* ---------------------------------------------------------------------------
 *  Core operations
 * --------------------------------------------------------------------------- */

/**
 * @brief Enqueue (produce) an item.  Lock-free, wait-free bounded.
 *
 * @param q     The queue.
 * @param data  Pointer to enqueue.  NULL is allowed but discouraged.
 * @return      true  – item was successfully enqueued.
 *              false – queue is full (try again later or apply backpressure).
 */
bool MPMC_Queue_Enqueue(MPMC_Queue *q, void *data);

/**
 * @brief Dequeue (consume) an item.  Lock-free, wait-free bounded.
 *
 * @param q     The queue.
 * @param data  [out] On success, receives the dequeued pointer.
 *              On failure the value is unchanged.
 * @return      true  – an item was dequeued into *data.
 *              false – queue is empty.
 */
bool MPMC_Queue_Dequeue(MPMC_Queue *q, void **data);

/* ---------------------------------------------------------------------------
 *  Status queries  (approximate – inherently racy in MPMC context)
 * --------------------------------------------------------------------------- */

/** Return the queue capacity (constant after init). */
uint64_t MPMC_Queue_Capacity(const MPMC_Queue *q);

/** Approximate number of items currently in the queue. */
uint64_t MPMC_Queue_Size(const MPMC_Queue *q);

/** Whether the queue appears empty right now. */
bool MPMC_Queue_IsEmpty(const MPMC_Queue *q);

/** Whether the queue appears full right now. */
bool MPMC_Queue_IsFull(const MPMC_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* MPMC_QUEUE_H */

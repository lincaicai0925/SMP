/**
* @file mpmc_queue.c
* @brief Lock-free bounded MPMC queue – implementation
*
* Cross-platform: supports MSVC 2015+ (C89), GCC 4.9+, Clang 3.5+, and C11.
*/

#include "modules/include/mpmc_queue.h"
#include "clabez/include/catomic.h"
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
*  Cross-platform alignment macro
*
*  Replaces C11 _Alignas() for compilers that don't support it (e.g. VS2015).
*  Works as a prefix on both struct members and variable declarations.
* --------------------------------------------------------------------------- */

#if defined(_MSC_VER)
/* MSVC: __declspec(align(N)) works on struct members and variables. */
#define MPMC_ALIGNAS(n)  __declspec(align(n))
#elif defined(__GNUC__) || defined(__clang__)
/* GCC / Clang: __attribute__((aligned(N))) as a prefix also works. */
#define MPMC_ALIGNAS(n)  __attribute__((aligned(n)))
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/* Standard C11 */
#define MPMC_ALIGNAS(n)  _Alignas(n)
#else
#error "Unsupported compiler: no alignment primitive available."
#endif

/* ---------------------------------------------------------------------------
*  Internal types
* --------------------------------------------------------------------------- */

/**
* A single slot in the ring buffer.
*
* Each cell owns an atomic `sequence` that encodes the cell's state:
*   sequence == pos          →  slot is free for a producer claiming `pos`
*   sequence == pos + 1      →  slot holds data for a consumer claiming `pos`
*   sequence == pos + capacity →  slot recycled, ready for next-round producer
*
* The cell is padded to a full cache line so that adjacent cells never share
* a cache line (prevents false sharing when different threads operate on
* neighbouring slots).
*/
typedef struct {
	MPMC_ALIGNAS(MPMC_CACHE_LINE_SIZE) catomic_int64 sequence;
	void *data;
	/* Padding to fill to cache line boundary is implicit thanks to alignment
	* on the *next* cell in the array (the compiler inserts trailing padding
	* so that sizeof(MPMC_Cell) is a multiple of MPMC_CACHE_LINE_SIZE). */
	char _pad[MPMC_CACHE_LINE_SIZE - sizeof(catomic_int64) - sizeof(void *)];
} MPMC_Cell;

/**
* Queue control structure.
*
* enqueue_pos and dequeue_pos live on separate cache lines so that
* producers and consumers don't bounce each other's cache lines.
*/
struct MPMC_Queue {
	/* Read-only after init ------------------------------------------------- */
	MPMC_Cell *buffer;
	uint64_t   capacity;
	uint64_t   mask;          /* capacity - 1, for fast modulo via & */

							  /* Producer-hot --------------------------------------------------------- */
	MPMC_ALIGNAS(MPMC_CACHE_LINE_SIZE) catomic_int64 enqueue_pos;

	/* Consumer-hot --------------------------------------------------------- */
	MPMC_ALIGNAS(MPMC_CACHE_LINE_SIZE) catomic_int64 dequeue_pos;
};

/* ---------------------------------------------------------------------------
*  Helpers
* --------------------------------------------------------------------------- */

/** Check that v is a power of 2 and >= 2. */
static bool is_power_of_two(uint64_t v)
{
	return v >= 2 && (v & (v - 1)) == 0;
}

/**
* Portable aligned allocation.
*
* C11 aligned_alloc requires `size` to be a multiple of `alignment`.
* We round up to satisfy that constraint.
*/
static void *alloc_aligned(size_t alignment, size_t size)
{
	/* Round size up to a multiple of alignment. */
	size = (size + alignment - 1) & ~(alignment - 1);

#if defined(_WIN32)
	return _aligned_malloc(size, alignment);
#else
	return aligned_alloc(alignment, size);
#endif
}

static void free_aligned(void *ptr)
{
#if defined(_WIN32)
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

/* ---------------------------------------------------------------------------
*  Lifecycle
* --------------------------------------------------------------------------- */

MPMC_Queue *MPMC_Queue_Init(uint64_t capacity)
{
	MPMC_Queue *q;
	size_t buf_bytes;
	uint64_t i;   /* VS2015 C89: declare all variables at block top */

	if (!is_power_of_two(capacity)) {
		return NULL;
	}

	/* Allocate the queue struct itself, cache-line aligned. */
	q = (MPMC_Queue *)alloc_aligned(
		MPMC_CACHE_LINE_SIZE, sizeof(MPMC_Queue));
	if (!q) {
		return NULL;
	}
	memset(q, 0, sizeof(*q));

	/* Allocate the cell array, cache-line aligned. */
	buf_bytes = (size_t)capacity * sizeof(MPMC_Cell);
	q->buffer = (MPMC_Cell *)alloc_aligned(MPMC_CACHE_LINE_SIZE, buf_bytes);
	if (!q->buffer) {
		free_aligned(q);
		return NULL;
	}
	memset(q->buffer, 0, buf_bytes);

	q->capacity = capacity;
	q->mask = capacity - 1;

	/* Initialise per-cell sequences: cell[i].sequence = i
	* This marks every cell as "available for the producer whose
	* enqueue_pos == i". */
	for (i = 0; i < capacity; i++) {
		catomic_store_int64(&q->buffer[i].sequence, (int64_t)i, MEM_ORDER_RELAXED);
		q->buffer[i].data = NULL;
	}

	catomic_store_int64(&q->enqueue_pos, 0, MEM_ORDER_RELAXED);
	catomic_store_int64(&q->dequeue_pos, 0, MEM_ORDER_RELAXED);

	return q;
}

void MPMC_Queue_Destroy(MPMC_Queue **q)
{
	MPMC_Queue *ptr;

	if (!q || !*q) {
		return;
	}

	ptr = *q;

	if (ptr->buffer) {
		free_aligned(ptr->buffer);
		ptr->buffer = NULL;
	}

	free_aligned(ptr);
	*q = NULL;
}

/* ---------------------------------------------------------------------------
*  Enqueue  (producer path)
* --------------------------------------------------------------------------- */

bool MPMC_Queue_Enqueue(MPMC_Queue *q, void *data)
{
	int64_t pos;

	if (!q) {
		return false;
	}

	pos = catomic_load_int64(&q->enqueue_pos, MEM_ORDER_RELAXED);

	for (;;) {
		MPMC_Cell *cell = &q->buffer[pos & q->mask];

		int64_t seq = catomic_load_int64(&cell->sequence, MEM_ORDER_ACQUIRE);

		int64_t diff = (int64_t)seq - (int64_t)pos;

		if (diff == 0) {
			/* Slot is free for this position.  Try to claim it by
			* advancing enqueue_pos.  CAS acts as our mutual exclusion
			* among competing producers. */
			if (catomic_compare_exchange_weak_int64(
				&q->enqueue_pos, &pos, pos + 1,
				MEM_ORDER_RELAXED, MEM_ORDER_RELAXED)) {
				/* Claimed.  Write the data, then publish by bumping
				* the cell's sequence to pos+1.  The release fence
				* ensures the data write is visible before the
				* sequence update. */
				cell->data = data;
				catomic_store_int64(&cell->sequence, pos + 1, MEM_ORDER_RELEASE);
				return true;
			}
			/* CAS failed – `pos` has been updated to the current value
			* of enqueue_pos by the CAS.  Loop and retry. */
		}
		else if (diff < 0) {
			/* The cell's sequence is *behind* our position, meaning the
			* consumer hasn't released this slot yet.  Queue is full. */
			return false;
		}
		else {
			/* diff > 0: another producer already advanced enqueue_pos
			* past `pos`.  Loop and retry. */
			pos = catomic_load_int64(&q->enqueue_pos, MEM_ORDER_RELAXED);
		}
	}
}

/* ---------------------------------------------------------------------------
*  Dequeue  (consumer path)
* --------------------------------------------------------------------------- */

bool MPMC_Queue_Dequeue(MPMC_Queue *q, void **data)
{
	int64_t pos;

	if (!q || !data) {
		return false;
	}

	pos = catomic_load_int64(&q->dequeue_pos, MEM_ORDER_RELAXED);

	for (;;) {
		MPMC_Cell *cell = &q->buffer[pos & q->mask];

		int64_t seq = catomic_load_int64(&cell->sequence, MEM_ORDER_ACQUIRE);
		int64_t diff = (int64_t)seq - (int64_t)(pos + 1);

		if (diff == 0) {
			/* seq == pos + 1 → this cell was published by a producer
			* for position `pos`.  Try to claim it. */
			if (catomic_compare_exchange_weak_int64(
				&q->dequeue_pos, &pos, pos + 1,
				MEM_ORDER_RELAXED, MEM_ORDER_RELAXED)) {
				/* Claimed.  Read the data, then recycle the slot by
				* setting sequence to pos + capacity.  This tells
				* a future producer (at enqueue_pos == pos + capacity)
				* that the slot is available again.  */
				*data = cell->data;
				cell->data = NULL;    /* Clear to avoid dangling ref */
				catomic_store_int64(&cell->sequence, pos + q->mask + 1, MEM_ORDER_RELEASE);
				return true;
			}
		}
		else if (diff < 0) {
			/* The producer hasn't published data here yet.
			* Queue is empty. */
			return false;
		}
		else {
			/* Another consumer already advanced dequeue_pos past `pos`.
			* Reload. */
			pos = catomic_load_int64(&q->dequeue_pos, MEM_ORDER_RELAXED);
		}
	}
}

/* ---------------------------------------------------------------------------
*  Status queries
* --------------------------------------------------------------------------- */

uint64_t MPMC_Queue_Capacity(const MPMC_Queue *q)
{
	return q ? q->capacity : 0;
}

uint64_t MPMC_Queue_Size(const MPMC_Queue *q)
{
	int64_t e, d;
	if (!q) {
		return 0;
	}
	e = catomic_load_int64((catomic_int64*)&q->enqueue_pos, MEM_ORDER_RELAXED);
	d = catomic_load_int64((catomic_int64*)&q->dequeue_pos, MEM_ORDER_RELAXED);
	return (e >= d) ? (uint64_t)(e - d) : 0;
}

bool MPMC_Queue_IsEmpty(const MPMC_Queue *q)
{
	return MPMC_Queue_Size(q) == 0;
}

bool MPMC_Queue_IsFull(const MPMC_Queue *q)
{
	return q ? (MPMC_Queue_Size(q) >= q->capacity) : true;
}

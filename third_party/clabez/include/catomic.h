#ifndef ATOMIC_C99_FULL_H
#define ATOMIC_C99_FULL_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef CC_EXPORTS
#ifdef CC_STATIC
#define CC_API
#else
#define CC_API __declspec(dllimport)
#endif
#else
#define CC_API __declspec(dllexport)
#endif
#define CC_CALL __cdecl
#elif defined(__unix) || defined(__linux)
#ifndef CC_API
#define CC_API __attribute__((visibility("default")))
#endif
#define CC_CALL
#endif
#include <stdbool.h>
#include <stdint.h>
    /* ---------- memory order ---------- */
    typedef enum catomic_mem_order
    {
        MEM_ORDER_RELAXED,
        MEM_ORDER_ACQUIRE,
        MEM_ORDER_RELEASE,
        MEM_ORDER_ACQ_REL,
        MEM_ORDER_SEQ_CST
    } catomic_mem_order;

    /* ---------- atomic int32 ---------- */
    typedef struct catomic_int32
    {
        volatile int32_t v;
    } catomic_int32;

    typedef struct catomic_int64
    {
        volatile int64_t v;
    } catomic_int64;

    typedef struct catomic_ptr
    {
        volatile void *v;
    } catomic_ptr;

    typedef struct catomic_bool
    {
        volatile int v;
    } catomic_bool;

    typedef struct catomic_flag
    {
        catomic_bool flag;
    } catomic_flag;



    /* ---------------- load/store ---------------- */
    CC_API int32_t CC_CALL catomic_load_int32(const catomic_int32 *a, catomic_mem_order mo);

    CC_API void CC_CALL catomic_store_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);

    CC_API int64_t CC_CALL catomic_load_int64(const catomic_int64 *a, catomic_mem_order mo);

    CC_API void CC_CALL catomic_store_int64(catomic_int64 *a, int64_t val, catomic_mem_order mo);

    CC_API void *CC_CALL catomic_load_ptr(const catomic_ptr *a, catomic_mem_order mo);
    CC_API void CC_CALL catomic_store_ptr(catomic_ptr *a, void *val, catomic_mem_order mo);

    CC_API bool CC_CALL catomic_load_bool(const catomic_bool *a, catomic_mem_order mo);

    CC_API void CC_CALL catomic_store_bool(catomic_bool *a, bool val, catomic_mem_order mo);

    /* ---------------- fetch_add / fetch_sub / fetch_and / fetch_or / fetch_xor / fetch_nand ---------------- */
    CC_API int32_t CC_CALL catomic_fetch_add_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);

    CC_API int32_t CC_CALL catomic_fetch_sub_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);
    
    CC_API int64_t CC_CALL catomic_fetch_add_int64(catomic_int64 *a, int64_t val, catomic_mem_order mo);

    CC_API int64_t CC_CALL catomic_fetch_sub_int64(catomic_int64 *a, int64_t val, catomic_mem_order mo);
    CC_API int32_t CC_CALL catomic_fetch_and_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);

    CC_API int32_t CC_CALL catomic_fetch_or_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);

    CC_API int32_t CC_CALL catomic_fetch_xor_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);

    CC_API int32_t CC_CALL catomic_fetch_nand_int32(catomic_int32 *a, int32_t val, catomic_mem_order mo);

    /* ---------------- compare_exchange ---------------- */
    CC_API bool CC_CALL catomic_compare_exchange_strong_int32(catomic_int32 *a, int32_t *expected, int32_t desired,
                                                              catomic_mem_order mo_succ, catomic_mem_order mo_fail);

    CC_API bool CC_CALL catomic_compare_exchange_weak_int32(catomic_int32 *a, int32_t *expected, int32_t desired,
                                                            catomic_mem_order mo_succ, catomic_mem_order mo_fail);
    
    CC_API bool CC_CALL catomic_compare_exchange_strong_int64(catomic_int64 *a, int64_t *expected, int64_t desired,
                                                              catomic_mem_order mo_succ, catomic_mem_order mo_fail);

    CC_API bool CC_CALL catomic_compare_exchange_weak_int64(catomic_int64 *a, int64_t *expected, int64_t desired,
                                                            catomic_mem_order mo_succ, catomic_mem_order mo_fail);
    
    CC_API bool CC_CALL catomic_compare_exchange_strong_ptr(catomic_ptr *a, void **expected, void *desired,
                                                            catomic_mem_order mo_succ, catomic_mem_order mo_fail);

    CC_API bool CC_CALL catomic_compare_exchange_weak_ptr(catomic_ptr *a, void **expected, void *desired,
                                                          catomic_mem_order mo_succ, catomic_mem_order mo_fail);

    /* ---------------- catomic_flag ---------------- */
    CC_API bool CC_CALL catomic_flagest_and_set(catomic_flag *f, catomic_mem_order mo);

    CC_API void CC_CALL catomic_flag_clear_explicit(catomic_flag *f, catomic_mem_order mo);

#ifdef __cplusplus
}
#endif
#endif

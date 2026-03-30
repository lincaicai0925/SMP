/**
 * @file cvector_allocator.h
 * @brief CVVector 自定义内存分配器接口
 * @details 提供灵活的内存分配器接口，支持运行时切换不同的分配器
 * 
 * @note 使用场景：
 *       - 默认使用标准库 malloc/realloc/free
 *       - 可切换到 TLSF 内存池分配器
 *       - 可切换到 FreeRTOS heap
 *       - 可切换到 DMA 安全内存池
 *       - 支持用户自定义分配器
 * 
 * @warning 线程安全：
 *          cvector_set_allocator() 不是线程安全的，应在初始化阶段调用
 * 
 * @example
 * @code
 * // 使用 TLSF 分配器
 * cmp_t my_tlsf = cmp_create_with_pool(...);
 * cvector_use_cmp_allocator(my_tlsf);
 * 
 * // 使用标准库分配器（默认）
 * cvector_use_stdlib_allocator();
 * 
 * // 使用自定义分配器
 * cvector_allocator_t custom_alloc = {
 *     .malloc_fn = my_malloc,
 *     .realloc_fn = my_realloc,
 *     .free_fn = my_free
 * };
 * cvector_set_allocator(&custom_alloc);
 * @endcode
 */

#ifndef CVECTOR_ALLOCATOR_H
#define CVECTOR_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 内存分配器函数指针结构
 * @details 定义了三个必需的内存管理函数指针
 */
typedef struct cvector_allocator_t {
    /**
     * @brief 分配内存函数指针
     * @param size 要分配的字节数
     * @return 成功返回内存指针，失败返回 NULL
     * @note 语义与标准库 malloc() 相同
     */
    void* (*malloc_fn)(size_t size);
    
    /**
     * @brief 重新分配内存函数指针
     * @param ptr 原内存指针（可以为 NULL）
     * @param size 新的大小
     * @return 成功返回新内存指针，失败返回 NULL
     * @note 语义与标准库 realloc() 相同
     * @warning 失败时不会释放原指针
     */
    void* (*realloc_fn)(void* ptr, size_t size);
    
    /**
     * @brief 释放内存函数指针
     * @param ptr 要释放的内存指针（可以为 NULL）
     * @note 语义与标准库 free() 相同
     */
    void  (*free_fn)(void* ptr);
    
    /**
     * @brief 获取可用内存大小（可选）
     * @return 可用内存字节数，如果不支持返回 0
     * @note 可以为 NULL，表示不支持此功能
     */
    size_t (*available_fn)(void);
    
} cvector_allocator_t;

/* ==================== 分配器设置接口 ==================== */

/**
 * @brief 设置全局内存分配器
 * @param allocator 分配器函数指针结构，传入 NULL 恢复默认分配器
 * @note 非线程安全，应在初始化阶段调用
 * @warning 切换分配器前，确保所有使用旧分配器的 vector 已销毁
 * @example
 * @code
 * cvector_allocator_t my_alloc = {
 *     .malloc_fn = my_malloc,
 *     .realloc_fn = my_realloc,
 *     .free_fn = my_free,
 *     .available_fn = NULL
 * };
 * cvector_set_allocator(&my_alloc);
 * @endcode
 */
void cvector_set_allocator(const cvector_allocator_t* allocator);

/**
 * @brief 获取当前使用的分配器
 * @return 当前分配器指针（不会返回 NULL）
 */
const cvector_allocator_t* cvector_get_allocator(void);

/**
 * @brief 使用标准库分配器（malloc/realloc/free）
 * @note 这是默认分配器
 */
void cvector_use_stdlib_allocator(void);

/* ==================== TLSF (cmempool) 分配器支持 ==================== */

#ifdef USE_TLSF_ALLOCATOR
#include "cmempool.h"

/**
 * @brief 使用 cmempool (TLSF) 分配器
 * @param tlsf cmempool 分配器句柄
 * @note 需要先调用 cmempool_create_with_pool() 初始化
 * @warning tlsf 句柄不能为 NULL
 * 
 * @example STM32H743 使用示例：
 * @code
 * // 分配 64KB 内存池（推荐在 SRAM1/SRAM2 区域）
 * static uint8_t tlsf_pool[65536] __attribute__((aligned(8)));
 * cmempool_t my_tlsf = cmempool_create_with_pool(tlsf_pool, sizeof(tlsf_pool));
 * 
 * if (my_tlsf != NULL) {
 *     // 设置 cvector 使用此内存池
 *     cvector_use_cmempool_allocator(my_tlsf);
 *     
 *     // 之后创建的所有 cvector 都从此池分配
 *     cvector_int* vec = cvector_int_create();
 *     cvector_int_push_back(vec, 42);
 *     cvector_int_destroy(&vec);
 * }
 * @endcode
 * 
 * @example 多内存池场景：
 * @code
 * // 快速内存池（小对象）
 * static uint8_t fast_pool[16384] __attribute__((aligned(8)));
 * cmempool_t fast_tlsf = cmempool_create_with_pool(fast_pool, sizeof(fast_pool));
 * 
 * // 通用内存池（大对象）
 * static uint8_t general_pool[131072] __attribute__((aligned(8)));
 * cmempool_t general_tlsf = cmempool_create_with_pool(general_pool, sizeof(general_pool));
 * 
 * // 切换到快速池
 * cvector_use_cmempool_allocator(fast_tlsf);
 * cvector_int* small_vec = cvector_int_create();
 * 
 * // 切换到通用池
 * cvector_use_cmempool_allocator(general_tlsf);
 * cvector_int* large_vec = cvector_int_create_with_capacity(10000);
 * @endcode
 */
void cvector_use_cmempool_allocator(cmempool_t tlsf);

/**
 * @brief 获取当前 cmempool 句柄
 * @return cmempool_t 句柄，如果未使用 cmempool 则返回 NULL
 * 
 * @example
 * @code
 * cmempool_t current = cvector_get_cmempool_handle();
 * if (current != NULL) {
 *     // 获取内存池统计信息
 *     cmempool_stats_t stats;
 *     cmempool_get_stats(current, &stats);
 *     printf("Memory usage: %zu bytes\n", stats.current_allocated);
 * }
 * @endcode
 */
cmempool_t cvector_get_cmempool_handle(void);

#endif /* USE_TLSF_ALLOCATOR */

/* ==================== 内存统计接口 ==================== */

/**
 * @brief 获取可用内存大小
 * @return 可用内存字节数，如果不支持返回 0
 * @note 依赖于当前分配器是否实现 available_fn
 */
size_t cvector_available_memory(void);

#ifdef __cplusplus
}
#endif

#endif /* CVECTOR_ALLOCATOR_H */


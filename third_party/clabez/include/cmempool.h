#ifndef CMEMPOOL_
#define CMEMPOOL_



#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* ==================== STM32H743 特定配置 ==================== */
#ifdef STM32H743xx

/**
 * @brief TLSF 错误码定义
 */
typedef enum {
    TLSF_OK = 0,                    /**< 操作成功 */
    TLSF_ERR_OUT_OF_MEMORY,         /**< 内存耗尽 */
    TLSF_ERR_INVALID_POINTER,       /**< 无效指针 */
    TLSF_ERR_DOUBLE_FREE,           /**< 重复释放 */
    TLSF_ERR_CORRUPTION,            /**< 内存结构损坏 */
    TLSF_ERR_ALIGNMENT,             /**< 对齐错误 */
    TLSF_ERR_INVALID_SIZE           /**< 无效大小 */
} cmempool_error_t;

/**
 * @brief 错误处理回调函数类型
 * 
 * @param error 错误码
 * @param context 上下文信息（可能是 cmempool_t 或内存地址）
 * @param file 发生错误的文件名（可选）
 * @param line 发生错误的行号（可选）
 * 
 * @note 此回调在中断禁用状态下调用，应该尽快返回
 * @warning 不要在此回调中调用任何 TLSF 函数，可能导致死锁
 */
typedef void (*cmempool_error_callback)(cmempool_error_t error, void* context, const char* file, int line);

/**
 * @brief 设置错误处理回调函数
 * 
 * @param callback 错误回调函数指针，传入 NULL 则取消回调
 * 
 * 使用示例：
 * @code
 * void my_error_handler(cmempool_error_t error, void* context, const char* file, int line) {
 *     // 记录错误到日志
 *     error_log(error);
 *     // 严重错误时复位系统
 *     if (error == TLSF_ERR_CORRUPTION) {
 *         NVIC_SystemReset();
 *     }
 * }
 * 
 * cmempool_set_error_callback(my_error_handler);
 * @endcode
 */
void cmempool_set_error_callback(cmempool_error_callback callback);

/**
 * @brief 分配 DMA 专用内存（32 字节对齐，适合 D-Cache）
 * 
 * @param tlsf TLSF 实例
 * @param bytes 请求的字节数
 * @return 指向对齐内存的指针，失败返回 NULL
 * 
 * @note STM32H743 的 D-Cache line 为 32 字节，DMA 缓冲区必须对齐到 Cache line
 * @note 建议将 DMA 内存池创建在 Non-Cacheable 区域，避免 Cache 一致性问题
 * @warning 返回的内存地址保证 32 字节对齐，但仍需注意 Cache 维护操作
 * 
 * 使用示例：
 * @code
 * uint8_t *dma_buffer = cmempool_malloc_dma(dma_tlsf, 1024);
 * if (dma_buffer != NULL) {
 *     SCB_CleanDCache_by_Addr(dma_buffer, 1024);  // 发送前清理 Cache
 *     HAL_UART_Transmit_DMA(&huart1, dma_buffer, 1024);
 * }
 * @endcode
 */
void* cmempool_malloc_dma(cmempool_t tlsf, size_t bytes);

/**
 * @brief 释放 DMA 内存
 * 
 * @param tlsf TLSF 实例
 * @param ptr 要释放的指针
 * @param bytes 内存块大小（用于 Cache 无效化）
 * 
 * @note 如果在 Cacheable 区域，此函数会自动进行 Cache 无效化
 * 
 * 使用示例：
 * @code
 * // DMA 接收完成后
 * cmempool_free_dma(dma_tlsf, dma_buffer, 1024);
 * @endcode
 */
void cmempool_free_dma(cmempool_t tlsf, void* ptr, size_t bytes);

/**
 * @brief 运行时统计信息
 */
typedef struct {
    unsigned int total_allocs;      /**< 总分配次数 */
    unsigned int total_frees;       /**< 总释放次数 */
    unsigned int current_blocks;    /**< 当前已分配块数 */
    unsigned int peak_blocks;       /**< 峰值块数 */
    size_t current_allocated;       /**< 当前已分配字节数 */
    size_t peak_allocated;          /**< 峰值分配字节数 */
    unsigned int failed_allocs;     /**< 分配失败次数 */
    unsigned int oom_events;        /**< 内存耗尽事件次数 */
} cmempool_stats_t;

/**
 * @brief 获取运行时统计信息
 * 
 * @param tlsf TLSF 实例
 * @param stats 输出统计信息的结构体指针
 * 
 * @note 此函数会禁用中断以确保数据一致性
 * 
 * 使用示例：
 * @code
 * cmempool_stats_t stats;
 * cmempool_get_stats(my_tlsf, &stats);
 * printf("Memory usage: %u / %u (peak: %u)\n", 
 *        stats.current_allocated, total_pool_size, stats.peak_allocated);
 * @endcode
 */
void cmempool_get_stats(cmempool_t tlsf, cmempool_stats_t* stats);

/**
 * @brief 重置统计信息
 * 
 * @param tlsf TLSF 实例
 */
void cmempool_reset_stats(cmempool_t tlsf);

#endif /* STM32H743xx */

/* ==================== 核心类型定义 ==================== */

/**
 * @brief TLSF 内存管理器类型
 * 
 * @note cmempool_t 是一个 TLSF 结构体，可以包含 1 到 N 个内存池
 * @note 这是一个不透明指针类型，内部结构对用户隐藏
 */
typedef void* cmempool_t;

/**
 * @brief 内存池类型
 * 
 * @note pool_t 是一块可由 TLSF 管理的内存区域
 * @note 这是一个不透明指针类型，内部结构对用户隐藏
 */
typedef void* pool_t;

/* ==================== 创建/销毁内存池 ==================== */

/**
 * @brief 创建 TLSF 内存管理器（不带内存池）
 * 
 * @param mem 用于存储 TLSF 控制结构的内存地址，大小至少为 cmempool_size() 字节
 * @return 成功返回 TLSF 实例句柄，失败返回 NULL
 * 
 * @note 创建后需要使用 cmempool_add_pool() 添加内存池才能分配内存
 * @note mem 指向的内存必须已初始化且可写
 * @warning mem 的生命周期必须覆盖整个 TLSF 实例的使用期间
 * 
 * 使用示例：
 * @code
 * uint8_t cmempool_mem[cmempool_size()];
 * cmempool_t my_tlsf = cmempool_create(cmempool_mem);
 * if (my_tlsf != NULL) {
 *     // 添加内存池
 *     cmempool_add_pool(my_tlsf, pool_mem, pool_size);
 * }
 * @endcode
 */
cmempool_t cmempool_create(void* mem);

/**
 * @brief 创建 TLSF 内存管理器（带内存池）
 * 
 * @param mem 用于存储 TLSF 控制结构和内存池的内存地址
 * @param bytes 内存块总大小（包括 TLSF 控制结构和池）
 * @return 成功返回 TLSF 实例句柄，失败返回 NULL
 * 
 * @note 这是创建 TLSF 的便捷方法，自动分配控制结构和第一个内存池
 * @note bytes 必须至少为 cmempool_size() + cmempool_pool_overhead() + 最小块大小
 * @warning mem 必须满足对齐要求（至少 cmempool_align_size() 字节对齐）
 * 
 * 使用示例：
 * @code
 * #define HEAP_SIZE 8192
 * uint8_t heap_mem[HEAP_SIZE] __attribute__((aligned(16)));
 * cmempool_t my_tlsf = cmempool_create_with_pool(heap_mem, HEAP_SIZE);
 * if (my_tlsf != NULL) {
 *     void* ptr = cmempool_malloc(my_tlsf, 128);
 * }
 * @endcode
 */
cmempool_t cmempool_create_with_pool(void* mem, size_t bytes);

/**
 * @brief 销毁 TLSF 内存管理器
 * 
 * @param tlsf 要销毁的 TLSF 实例
 * 
 * @note 销毁前应确保所有分配的内存已释放
 * @note 此函数不会自动释放内存池，只是清理 TLSF 控制结构
 * @warning 销毁后不得再使用该 TLSF 实例，否则行为未定义
 * 
 * 使用示例：
 * @code
 * // 释放所有已分配的内存（由应用程序负责）
 * cmempool_free(my_tlsf, ptr1);
 * cmempool_free(my_tlsf, ptr2);
 * // 销毁 TLSF 实例
 * cmempool_destroy(my_tlsf);
 * @endcode
 */
void cmempool_destroy(cmempool_t tlsf);

/**
 * @brief 获取第一个内存池
 * 
 * @param tlsf TLSF 实例
 * @return 返回第一个内存池句柄，如果没有池则返回 NULL
 * 
 * @note 用于遍历或检查内存池
 * @note 如果有多个池，可以配合 cmempool_walk_pool() 使用
 * 
 * 使用示例：
 * @code
 * pool_t pool = cmempool_get_pool(my_tlsf);
 * if (pool != NULL) {
 *     if (cmempool_check_pool(pool) != 0) {
 *         // 检测到内存损坏
 *     }
 * }
 * @endcode
 */
pool_t cmempool_get_pool(cmempool_t tlsf);

/* ==================== 添加/移除内存池 ==================== */

/**
 * @brief 添加内存池到 TLSF 管理器
 * 
 * @param tlsf TLSF 实例
 * @param mem 内存池起始地址
 * @param bytes 内存池大小（字节）
 * @return 成功返回内存池句柄，失败返回 NULL
 * 
 * @note 可以多次调用以添加多个不连续的内存池
 * @note bytes 必须至少为 cmempool_pool_overhead() + cmempool_block_size_min()
 * @note mem 必须满足对齐要求（至少 cmempool_align_size() 字节对齐）
 * @warning mem 的生命周期必须覆盖整个内存池的使用期间
 * 
 * 使用示例：
 * @code
 * // 添加多个内存区域
 * uint8_t sram1[4096] __attribute__((aligned(16)));
 * uint8_t sram2[8192] __attribute__((aligned(16)));
 * 
 * pool_t pool1 = cmempool_add_pool(my_tlsf, sram1, sizeof(sram1));
 * pool_t pool2 = cmempool_add_pool(my_tlsf, sram2, sizeof(sram2));
 * @endcode
 */
pool_t cmempool_add_pool(cmempool_t tlsf, void* mem, size_t bytes);

/**
 * @brief 从 TLSF 管理器中移除内存池
 * 
 * @param tlsf TLSF 实例
 * @param pool 要移除的内存池句柄
 * 
 * @note 移除前必须确保该池中的所有内存块已释放
 * @warning 如果池中仍有未释放的内存块，行为未定义
 * @warning 移除后不得再使用该池句柄
 * 
 * 使用示例：
 * @code
 * // 移除不再需要的内存池
 * pool_t temp_pool = cmempool_add_pool(my_tlsf, temp_mem, temp_size);
 * // ... 使用临时池 ...
 * // 确保池中内存已全部释放
 * cmempool_remove_pool(my_tlsf, temp_pool);
 * @endcode
 */
void cmempool_remove_pool(cmempool_t tlsf, pool_t pool);

/* ==================== 内存分配函数（替代标准库） ==================== */

/**
 * @brief 分配指定大小的内存块
 * 
 * @param tlsf TLSF 实例
 * @param bytes 请求的字节数
 * @return 成功返回内存块指针，失败返回 NULL
 * 
 * @note 返回的指针满足默认对齐要求（通常为 4 或 8 字节）
 * @note 类似标准库的 malloc()，但需要显式传入 TLSF 实例
 * @note 时间复杂度为 O(1)，适合实时系统
 * @warning bytes 为 0 时行为由实现定义，建议避免
 * 
 * 使用示例：
 * @code
 * void* buffer = cmempool_malloc(my_tlsf, 1024);
 * if (buffer != NULL) {
 *     memset(buffer, 0, 1024);
 *     // 使用 buffer
 *     cmempool_free(my_tlsf, buffer);
 * }
 * @endcode
 */
void* cmempool_malloc(cmempool_t tlsf, size_t bytes);

/**
 * @brief 分配指定对齐的内存块
 * 
 * @param tlsf TLSF 实例
 * @param align 对齐要求（必须是 2 的幂次）
 * @param bytes 请求的字节数
 * @return 成功返回对齐的内存块指针，失败返回 NULL
 * 
 * @note 返回的指针地址是 align 的整数倍
 * @note align 必须是 2 的幂次且 >= cmempool_align_size()
 * @note 用于需要特殊对齐的场景（如 SIMD、DMA 缓冲区）
 * @warning 对齐要求越大，内存浪费越多
 * 
 * 使用示例：
 * @code
 * // 分配 64 字节对齐的缓冲区（用于 Cache line 对齐）
 * void* aligned_buf = cmempool_memalign(my_tlsf, 64, 4096);
 * if (aligned_buf != NULL) {
 *     // 地址保证是 64 的倍数
 *     assert(((uintptr_t)aligned_buf & 63) == 0);
 * }
 * @endcode
 */
void* cmempool_memalign(cmempool_t tlsf, size_t align, size_t bytes);

/**
 * @brief 重新分配内存块大小
 * 
 * @param tlsf TLSF 实例
 * @param ptr 原内存块指针（可以为 NULL）
 * @param size 新的大小（字节）
 * @return 成功返回新内存块指针，失败返回 NULL
 * 
 * @note 如果 ptr 为 NULL，等同于 cmempool_malloc(tlsf, size)
 * @note 如果 size 为 0，等同于 cmempool_free(tlsf, ptr) 并返回 NULL
 * @note 如果新大小更小，可能原地收缩；如果更大，可能移动内存块
 * @note 数据内容会被保留（取 min(old_size, new_size) 字节）
 * @warning 如果返回非 NULL 且不等于 ptr，必须使用新指针，旧指针已失效
 * @warning 如果返回 NULL（且 size != 0），原指针仍然有效，需要手动释放
 * 
 * 使用示例：
 * @code
 * char* buf = cmempool_malloc(my_tlsf, 100);
 * strcpy(buf, "Hello");
 * // 扩大缓冲区
 * char* new_buf = cmempool_realloc(my_tlsf, buf, 200);
 * if (new_buf != NULL) {
 *     buf = new_buf;  // 使用新指针
 *     strcat(buf, " World");
 * }
 * @endcode
 */
void* cmempool_realloc(cmempool_t tlsf, void* ptr, size_t size);

/**
 * @brief 释放内存块
 * 
 * @param tlsf TLSF 实例
 * @param ptr 要释放的内存块指针
 * 
 * @note 类似标准库的 free()，但需要显式传入 TLSF 实例
 * @note 如果 ptr 为 NULL，此函数无操作（安全调用）
 * @note 时间复杂度为 O(1)
 * @warning ptr 必须是通过该 TLSF 实例分配的，否则行为未定义
 * @warning 重复释放同一指针会导致严重错误（双重释放）
 * @warning 释放后不得再访问该内存，否则行为未定义
 * 
 * 使用示例：
 * @code
 * void* ptr = cmempool_malloc(my_tlsf, 100);
 * if (ptr != NULL) {
 *     // 使用 ptr
 *     cmempool_free(my_tlsf, ptr);
 *     ptr = NULL;  // 良好实践：防止悬空指针
 * }
 * @endcode
 */
void cmempool_free(cmempool_t tlsf, void* ptr);

/* ==================== 查询函数 ==================== */

/**
 * @brief 获取内存块的实际大小
 * 
 * @param ptr 内存块指针（由 cmempool_malloc 等函数返回）
 * @return 返回内存块的内部大小（字节），通常 >= 请求的大小
 * 
 * @note 返回的是实际可用的内存块大小，不是分配时请求的大小
 * @note 由于对齐和最小块限制，实际大小通常会大于请求大小
 * @warning ptr 必须是有效的已分配内存块，否则行为未定义
 * 
 * 使用示例：
 * @code
 * void* ptr = cmempool_malloc(my_tlsf, 100);
 * size_t actual = cmempool_block_size(ptr);
 * // actual 可能是 112、128 等（取决于内部块大小粒度）
 * printf("Requested: 100, Actual: %zu\n", actual);
 * @endcode
 */
size_t cmempool_block_size(void* ptr);

/**
 * @brief 获取 TLSF 控制结构的大小
 * 
 * @return 返回 TLSF 控制结构所需的字节数
 * 
 * @note 在调用 cmempool_create() 前使用此函数确定所需内存
 * @note 此大小是固定的，不依赖于内存池数量或大小
 * 
 * 使用示例：
 * @code
 * size_t cmempool_ctrl_size = cmempool_size();
 * uint8_t* mem = malloc(cmempool_ctrl_size);
 * cmempool_t my_tlsf = cmempool_create(mem);
 * @endcode
 */
size_t cmempool_size(void);

/**
 * @brief 获取内存对齐要求
 * 
 * @return 返回 TLSF 内存对齐的字节数（通常为 4、8 或 16）
 * 
 * @note cmempool_malloc() 返回的指针满足此对齐
 * @note 传递给 cmempool_create() 和 cmempool_add_pool() 的内存也应满足此对齐
 * 
 * 使用示例：
 * @code
 * size_t align = cmempool_align_size();
 * // 在某些平台上可以使用 posix_memalign
 * void* mem;
 * posix_memalign(&mem, align, pool_size);
 * @endcode
 */
size_t cmempool_align_size(void);

/**
 * @brief 获取最小内存块大小
 * 
 * @return 返回 TLSF 可分配的最小块大小（字节）
 * 
 * @note 请求小于此值的分配会被向上舍入到此大小
 * @note 用于计算内存池的最小有效大小
 * 
 * 使用示例：
 * @code
 * size_t min_block = cmempool_block_size_min();
 * printf("Minimum allocation size: %zu bytes\n", min_block);
 * @endcode
 */
size_t cmempool_block_size_min(void);

/**
 * @brief 获取最大内存块大小
 * 
 * @return 返回 TLSF 可分配的最大块大小（字节）
 * 
 * @note 超过此大小的分配请求会失败
 * @note 在 32 位系统上通常为 2GB 左右
 * 
 * 使用示例：
 * @code
 * size_t max_block = cmempool_block_size_max();
 * if (requested_size > max_block) {
 *     // 请求过大，需要其他策略
 * }
 * @endcode
 */
size_t cmempool_block_size_max(void);

/**
 * @brief 获取内存池的开销
 * 
 * @return 返回每个内存池的管理开销（字节）
 * 
 * @note 从 cmempool_add_pool() 的 bytes 参数中，此大小会被用于管理结构
 * @note 实际可用内存 = bytes - cmempool_pool_overhead()
 * 
 * 使用示例：
 * @code
 * size_t overhead = cmempool_pool_overhead();
 * size_t needed = requested_size + overhead;
 * pool_t pool = cmempool_add_pool(my_tlsf, mem, needed);
 * @endcode
 */
size_t cmempool_pool_overhead(void);

/**
 * @brief 获取每次分配的开销
 * 
 * @return 返回每个分配块的管理开销（字节）
 * 
 * @note 每次 cmempool_malloc() 调用实际使用的内存 = 请求大小 + 此开销
 * @note 用于估算内存使用效率
 * 
 * 使用示例：
 * @code
 * size_t alloc_overhead = cmempool_alloc_overhead();
 * size_t efficiency = 100 * requested / (requested + alloc_overhead);
 * printf("Memory efficiency: %zu%%\n", efficiency);
 * @endcode
 */
size_t cmempool_alloc_overhead(void);

/* ==================== 调试功能 ==================== */

/**
 * @brief 内存池遍历回调函数类型
 * 
 * @param ptr 内存块地址
 * @param size 内存块大小（字节）
 * @param used 是否已分配（非零表示已分配，0 表示空闲）
 * @param user 用户自定义数据（由 cmempool_walk_pool 传入）
 * 
 * @note 此回调会被 cmempool_walk_pool() 调用，用于遍历内存池中的所有块
 * @note 可用于统计内存使用、检测内存泄漏、生成内存映射等
 */
typedef void (*cmempool_walker)(void* ptr, size_t size, int used, void* user);

/**
 * @brief 遍历内存池中的所有内存块
 * 
 * @param pool 要遍历的内存池句柄
 * @param walker 回调函数指针
 * @param user 传递给回调函数的用户数据
 * 
 * @note 对池中的每个内存块（已分配和空闲）调用一次 walker
 * @note 用于诊断、统计、内存泄漏检测等
 * @warning 遍历过程中不得修改内存池结构（不要调用 malloc/free）
 * 
 * 使用示例：
 * @code
 * void print_block(void* ptr, size_t size, int used, void* user) {
 *     printf("Block at %p: size=%zu, %s\n", 
 *            ptr, size, used ? "USED" : "FREE");
 * }
 * 
 * pool_t pool = cmempool_get_pool(my_tlsf);
 * cmempool_walk_pool(pool, print_block, NULL);
 * @endcode
 */
void cmempool_walk_pool(pool_t pool, cmempool_walker walker, void* user);

/**
 * @brief 检查 TLSF 实例的一致性
 * 
 * @param tlsf TLSF 实例
 * @return 0 表示通过检查，非零表示检测到错误
 * 
 * @note 执行内部一致性检查，验证数据结构完整性
 * @note 用于调试阶段检测内存损坏、越界写入等问题
 * @warning 此函数开销较大，不建议在生产环境频繁调用
 * 
 * 使用示例：
 * @code
 * if (cmempool_check(my_tlsf) != 0) {
 *     fprintf(stderr, "TLSF corruption detected!\n");
 *     // 触发错误处理或系统复位
 * }
 * @endcode
 */
int cmempool_check(cmempool_t tlsf);

/**
 * @brief 检查单个内存池的一致性
 * 
 * @param pool 内存池句柄
 * @return 0 表示通过检查，非零表示检测到错误
 * 
 * @note 类似 cmempool_check()，但只检查单个内存池
 * @note 比检查整个 TLSF 实例更快
 * 
 * 使用示例：
 * @code
 * pool_t pool = cmempool_get_pool(my_tlsf);
 * if (pool != NULL && cmempool_check_pool(pool) != 0) {
 *     fprintf(stderr, "Pool corruption detected!\n");
 * }
 * @endcode
 */
int cmempool_check_pool(pool_t pool);

#if defined(__cplusplus)
};
#endif

#endif

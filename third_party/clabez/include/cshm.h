#ifndef C_CORE_SHM_H_
#define C_CORE_SHM_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file cshm.h
 * @brief 跨平台 C99 共享内存库，完整对标 Boost.Interprocess 共享内存功能。
 *
 * - Windows: 要求 Vista+ (0x0600)，使用高性能文件映射 API 和原生同步原语。
 * - Linux/Unix: 依赖 POSIX 共享内存 API (shm_open/mmap)。
 * - 提供完整的共享内存管理功能：
 *   * 创建模式：仅创建、仅打开、打开或创建
 *   * 分离的映射视图管理（mapped_region）
 *   * 线程安全的访问和操作
 *   * 同步原语：互斥锁、读写锁、条件变量、信号量（均支持进程间共享）
 *   * offset_ptr：可重定位指针
 *   * managed_shared_memory：高级段管理
 *   * 错误处理机制
 *   * 权限控制
 * - 所有接口均为线程安全设计，性能对标 Boost.Interprocess。
 *
 * @note 参考 Boost.Interprocess 设计理念：
 *       - shared_memory_object: 共享内存对象（资源句柄）
 *       - mapped_region: 映射视图（可多次映射不同区域）
 *       - interprocess_mutex: 进程间互斥锁
 *       - interprocess_sharable_mutex: 进程间读写锁
 *       - interprocess_condition: 进程间条件变量
 *       - offset_ptr<T>: 相对偏移指针
 */

/* 确保 Windows 使用 Vista+ API */
#ifdef _WIN32
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0600
#  endif
#endif

#include <stddef.h>
#include <stdint.h>

/* DLL 导出/导入宏定义 */
#if defined(_WIN32) || defined(_WIN64)
#ifndef CC_EXPORTS
#ifdef CC_STATIC
#undef CC_API
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

/** \addtogroup cshm 共享内存
 * @{
 */

/** @name 错误码
 *  @brief 所有 API 返回值统一使用 @ref cshm_errc。
 *  @{
 */
typedef enum cshm_errc {
    CSHM_OK                    = 0, /**< 操作成功。 */
    CSHM_ERROR_GENERAL         = 1, /**< 未归类的运行时错误。 */
    CSHM_ERROR_INVALID_ARG     = 2, /**< 传入参数无效或为 NULL。 */
    CSHM_ERROR_NOMEM           = 3, /**< 内存不足。 */
    CSHM_ERROR_EXISTS          = 4, /**< 共享内存已存在（create_only 模式）。 */
    CSHM_ERROR_NOT_FOUND       = 5, /**< 共享内存不存在（open_only 模式）。 */
    CSHM_ERROR_ACCESS_DENIED   = 6, /**< 访问被拒绝（权限不足）。 */
    CSHM_ERROR_ALREADY_MAPPED  = 7, /**< 已经映射到地址空间。 */
    CSHM_ERROR_MAP_FAILED      = 8, /**< 映射失败。 */
    CSHM_ERROR_SIZE_MISMATCH   = 9, /**< 大小不匹配。 */
    CSHM_ERROR_TIMEOUT         = 10 /**< 操作超时。 */
} cshm_errc;
/** @} */

/** @name 打开模式
 *  @brief 控制共享内存的创建/打开行为。
 *  @{
 */
typedef enum cshm_open_mode {
    CSHM_OPEN_ONLY         = 0, /**< 仅打开，不存在则失败。 */
    CSHM_OPEN_OR_CREATE    = 1, /**< 打开或创建（默认）。 */
    CSHM_CREATE_ONLY       = 2  /**< 仅创建，已存在则失败。 */
} cshm_open_mode;
/** @} */

/** @name 访问权限
 *  @brief 控制共享内存的访问权限。
 *  @{
 */
typedef enum cshm_access {
    CSHM_READ_ONLY   = 0x01, /**< 只读访问。 */
    CSHM_READ_WRITE  = 0x03  /**< 读写访问（READ | WRITE）。 */
} cshm_access;
/** @} */

/**
 * @struct cshm
 * @brief 共享内存对象，类似 boost::interprocess::shared_memory_object。
 *        表示共享内存资源本身，不包含映射信息。
 */
typedef struct cshm cshm;

/**
 * @struct cshm_region
 * @brief 映射区域，类似 boost::interprocess::mapped_region。
 *        表示共享内存在进程地址空间中的映射视图。
 */
typedef struct cshm_region cshm_region;

/**
 * @struct cshm_mutex
 * @brief 共享内存互斥锁，用于进程间同步。
 *        类似 boost::interprocess::interprocess_mutex。
 */
typedef struct cshm_mutex cshm_mutex;

/**
 * @struct cshm_sharable_mutex
 * @brief 共享内存读写锁，用于进程间同步。
 *        类似 boost::interprocess::interprocess_sharable_mutex。
 */
typedef struct cshm_sharable_mutex cshm_sharable_mutex;

/**
 * @struct cshm_condition
 * @brief 共享内存条件变量，用于进程间同步。
 *        类似 boost::interprocess::interprocess_condition。
 */
typedef struct cshm_condition cshm_condition;

/**
 * @struct cshm_semaphore
 * @brief 共享内存信号量，用于进程间同步。
 *        类似 boost::interprocess::interprocess_semaphore。
 */
typedef struct cshm_semaphore cshm_semaphore;

/**
 * @struct cshm_managed
 * @brief 管理型共享内存，类似 boost::interprocess::managed_shared_memory。
 *        提供内存段管理和命名对象分配功能。
 */
typedef struct cshm_managed cshm_managed;

/** @name 共享内存对象管理 API（shared_memory_object）
 *  @{
 */

/**
 * @brief 创建或打开共享内存对象（高级版本）。
 *
 * @param name 共享内存名称（不含路径分隔符，自动添加前缀）
 * @param size_byte 大小（字节），仅在创建时有效
 * @param mode 打开模式（OPEN_ONLY, OPEN_OR_CREATE, CREATE_ONLY）
 * @param access 访问权限（READ_ONLY, READ_WRITE）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回共享内存对象，失败返回 NULL
 *
 * @note 线程安全。此函数只创建/打开共享内存资源，不进行映射。
 *       使用 cshm_region_create 进行映射。
 *       Windows 使用 CreateFileMapping/OpenFileMapping，
 *       Linux 使用 shm_open。
 */
CC_API cshm *CC_CALL cshm_open_ex(const char *name, 
                                   size_t size_byte,
                                   cshm_open_mode mode,
                                   cshm_access access,
                                   cshm_errc *err);

/**
 * @brief 打开或创建共享内存（简化版本，兼容旧接口，会自动创建默认映射）。
 *
 * @param name 共享内存名称
 * @param size_byte 大小（字节）
 * @return 成功返回共享内存对象，失败返回 NULL
 *
 * @note 等价于 cshm_open_ex + 自动映射。保留用于向后兼容。
 *       新代码建议使用 cshm_open_ex + cshm_region_create。
 */
CC_API cshm *CC_CALL cshm_open(const char *name, int size_byte);

/**
 * @brief 获取共享内存映射地址（兼容旧接口）。
 *
 * @param self 共享内存对象
 * @return 映射地址，失败返回 NULL
 *
 * @note 仅对通过 cshm_open 创建的对象有效。
 *       新代码建议使用 cshm_region_get_addr。
 */
CC_API void *CC_CALL cshm_get_addr(cshm *self);

/**
 * @brief 获取共享内存大小。
 *
 * @param self 共享内存对象
 * @return 大小（字节），失败返回 0
 */
CC_API size_t CC_CALL cshm_get_size(cshm *self);

/**
 * @brief 获取共享内存名称。
 *
 * @param self 共享内存对象
 * @return 名称字符串，失败返回 NULL
 *
 * @note 返回的字符串生命周期与 cshm 对象相同。
 */
CC_API const char *CC_CALL cshm_get_name(cshm *self);

/**
 * @brief 调整共享内存大小（仅在创建时或独占访问时有效）。
 *
 * @param self 共享内存对象
 * @param new_size 新大小（字节）
 * @return 错误码
 *
 * @note 所有已有映射需要重新创建。线程安全。
 */
CC_API cshm_errc CC_CALL cshm_truncate(cshm *self, size_t new_size);

/**
 * @brief 调整共享内存大小（旧接口，兼容性）。
 *
 * @deprecated 使用 cshm_truncate 替代
 */
CC_API cshm_errc CC_CALL cshm_resize(cshm *self, size_t new_size);

/**
 * @brief 刷新共享内存到持久化存储（旧接口，兼容性）。
 *
 * @deprecated 使用 cshm_region_flush 替代
 */
CC_API cshm_errc CC_CALL cshm_flush(cshm *self, int async);

/**
 * @brief 关闭共享内存对象。
 *
 * @param pself 共享内存对象指针的指针
 *
 * @note 自动释放资源，*pself 设为 NULL。线程安全。
 *       不会自动删除共享内存，使用 cshm_remove 删除。
 */
CC_API void CC_CALL cshm_close(cshm **pself);

/**
 * @brief 删除共享内存（静态方法，无需打开）。
 *
 * @param name 共享内存名称
 * @return 错误码
 *
 * @note Windows 自动清理，Linux 调用 shm_unlink。线程安全。
 */
CC_API cshm_errc CC_CALL cshm_remove(const char *name);

/**
 * @brief 检查共享内存是否存在。
 *
 * @param name 共享内存名称
 * @return 存在返回 1，不存在返回 0
 */
CC_API int CC_CALL cshm_exists(const char *name);

/** @} */

/** @name 映射区域管理 API（mapped_region）
 *  @{
 */

/**
 * @brief 创建共享内存的映射区域。
 *
 * @param shm 共享内存对象
 * @param offset 映射起始偏移量（字节）
 * @param size 映射大小（字节），0 表示映射整个共享内存
 * @param access 访问权限（READ_ONLY, READ_WRITE）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回映射区域对象，失败返回 NULL
 *
 * @note 线程安全。可以对同一个共享内存创建多个不同的映射区域。
 *       Windows 使用 MapViewOfFile，Linux 使用 mmap。
 */
CC_API cshm_region *CC_CALL cshm_region_create(cshm *shm,
                                                 size_t offset,
                                                 size_t size,
                                                 cshm_access access,
                                                 cshm_errc *err);

/**
 * @brief 获取映射区域的地址。
 *
 * @param region 映射区域对象
 * @return 映射地址，失败返回 NULL
 */
CC_API void *CC_CALL cshm_region_get_addr(cshm_region *region);

/**
 * @brief 获取映射区域的大小。
 *
 * @param region 映射区域对象
 * @return 大小（字节），失败返回 0
 */
CC_API size_t CC_CALL cshm_region_get_size(cshm_region *region);

/**
 * @brief 刷新映射区域到持久化存储（如果支持）。
 *
 * @param region 映射区域对象
 * @param async 是否异步刷新（0=同步，1=异步）
 * @return 错误码
 *
 * @note Windows 使用 FlushViewOfFile，Linux 使用 msync。
 */
CC_API cshm_errc CC_CALL cshm_region_flush(cshm_region *region, int async);

/**
 * @brief 关闭映射区域。
 *
 * @param pregion 映射区域对象指针的指针
 *
 * @note 自动取消映射和释放资源，*pregion 设为 NULL。线程安全。
 */
CC_API void CC_CALL cshm_region_close(cshm_region **pregion);

/** @} */

/** @name 共享内存同步原语 API（interprocess synchronization）
 *  @{
 */

/**
 * @brief 在共享内存中创建互斥锁。
 *
 * @param region 映射区域对象（或使用旧接口的 cshm 对象）
 * @param offset 在映射区域中的偏移量（字节）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回互斥锁对象，失败返回 NULL
 *
 * @note Windows Vista+ 使用共享内存中的同步结构（模拟 PTHREAD_PROCESS_SHARED），
 *       Linux 使用 pthread_mutex with PTHREAD_PROCESS_SHARED。
 *       需要预留 cshm_mutex_sizeof() 字节空间。
 */
CC_API cshm_mutex *CC_CALL cshm_mutex_create(void *region, size_t offset, cshm_errc *err);

/**
 * @brief 获取互斥锁所需的空间大小。
 *
 * @return 字节数
 */
CC_API size_t CC_CALL cshm_mutex_sizeof(void);

/**
 * @brief 锁定互斥锁。
 *
 * @param mutex 互斥锁对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_mutex_lock(cshm_mutex *mutex);

/**
 * @brief 尝试锁定互斥锁（非阻塞）。
 *
 * @param mutex 互斥锁对象
 * @return 成功返回 CSHM_OK，已被锁定返回 CSHM_ERROR_TIMEOUT
 */
CC_API cshm_errc CC_CALL cshm_mutex_trylock(cshm_mutex *mutex);

/**
 * @brief 解锁互斥锁。
 *
 * @param mutex 互斥锁对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_mutex_unlock(cshm_mutex *mutex);

/**
 * @brief 销毁互斥锁。
 *
 * @param pmutex 互斥锁对象指针的指针
 */
CC_API void CC_CALL cshm_mutex_destroy(cshm_mutex **pmutex);

/**
 * @brief 在共享内存中创建读写锁（可共享互斥锁）。
 *
 * @param region 映射区域对象（或使用旧接口的 cshm 对象）
 * @param offset 在映射区域中的偏移量（字节）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回读写锁对象，失败返回 NULL
 *
 * @note 类似 boost::interprocess::interprocess_sharable_mutex。
 *       支持多读单写模式。需要预留 cshm_sharable_mutex_sizeof() 字节空间。
 */
CC_API cshm_sharable_mutex *CC_CALL cshm_sharable_mutex_create(void *region, size_t offset, cshm_errc *err);

/**
 * @brief 获取读写锁所需的空间大小。
 *
 * @return 字节数
 */
CC_API size_t CC_CALL cshm_sharable_mutex_sizeof(void);

/**
 * @brief 写锁定（独占锁定）。
 *
 * @param mutex 读写锁对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_sharable_mutex_lock(cshm_sharable_mutex *mutex);

/**
 * @brief 读锁定（共享锁定）。
 *
 * @param mutex 读写锁对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_sharable_mutex_lock_sharable(cshm_sharable_mutex *mutex);

/**
 * @brief 尝试写锁定（非阻塞）。
 *
 * @param mutex 读写锁对象
 * @return 成功返回 CSHM_OK，已被锁定返回 CSHM_ERROR_TIMEOUT
 */
CC_API cshm_errc CC_CALL cshm_sharable_mutex_try_lock(cshm_sharable_mutex *mutex);

/**
 * @brief 尝试读锁定（非阻塞）。
 *
 * @param mutex 读写锁对象
 * @return 成功返回 CSHM_OK，已被锁定返回 CSHM_ERROR_TIMEOUT
 */
CC_API cshm_errc CC_CALL cshm_sharable_mutex_try_lock_sharable(cshm_sharable_mutex *mutex);

/**
 * @brief 解锁（写锁）。
 *
 * @param mutex 读写锁对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_sharable_mutex_unlock(cshm_sharable_mutex *mutex);

/**
 * @brief 解锁（读锁）。
 *
 * @param mutex 读写锁对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_sharable_mutex_unlock_sharable(cshm_sharable_mutex *mutex);

/**
 * @brief 销毁读写锁。
 *
 * @param pmutex 读写锁对象指针的指针
 */
CC_API void CC_CALL cshm_sharable_mutex_destroy(cshm_sharable_mutex **pmutex);

/**
 * @brief 在共享内存中创建条件变量。
 *
 * @param region 映射区域对象（或使用旧接口的 cshm 对象）
 * @param offset 在映射区域中的偏移量（字节）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回条件变量对象，失败返回 NULL
 *
 * @note Windows Vista+ 使用 CONDITION_VARIABLE（原生API），
 *       Linux 使用 pthread_cond with PTHREAD_PROCESS_SHARED。
 *       需要预留 cshm_condition_sizeof() 字节空间。
 */
CC_API cshm_condition *CC_CALL cshm_condition_create(void *region, size_t offset, cshm_errc *err);

/**
 * @brief 获取条件变量所需的空间大小。
 *
 * @return 字节数
 */
CC_API size_t CC_CALL cshm_condition_sizeof(void);

/**
 * @brief 等待条件变量。
 *
 * @param cond 条件变量对象
 * @param mutex 关联的互斥锁对象
 * @param timeout_ms 超时时间（毫秒），0 表示无限等待
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_condition_wait(cshm_condition *cond, cshm_mutex *mutex, uint32_t timeout_ms);

/**
 * @brief 通知一个等待的线程。
 *
 * @param cond 条件变量对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_condition_signal(cshm_condition *cond);

/**
 * @brief 通知所有等待的线程。
 *
 * @param cond 条件变量对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_condition_broadcast(cshm_condition *cond);

/**
 * @brief 销毁条件变量。
 *
 * @param pcond 条件变量对象指针的指针
 */
CC_API void CC_CALL cshm_condition_destroy(cshm_condition **pcond);

/**
 * @brief 在共享内存中创建信号量（匿名）。
 *
 * @param region 映射区域对象（或使用旧接口的 cshm 对象）
 * @param offset 在映射区域中的偏移量（字节）
 * @param initial_value 初始值
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回信号量对象，失败返回 NULL
 *
 * @note 需要预留 cshm_semaphore_sizeof() 字节空间。
 */
CC_API cshm_semaphore *CC_CALL cshm_semaphore_create(void *region, size_t offset, uint32_t initial_value, cshm_errc *err);

/**
 * @brief 创建命名信号量（跨进程）。
 *
 * @param name 信号量名称
 * @param initial_value 初始值
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回信号量对象，失败返回 NULL
 *
 * @note Windows 使用 CreateSemaphore，Linux 使用 sem_open。
 */
CC_API cshm_semaphore *CC_CALL cshm_semaphore_open(const char *name, uint32_t initial_value, cshm_errc *err);

/**
 * @brief 获取信号量所需的空间大小。
 *
 * @return 字节数
 */
CC_API size_t CC_CALL cshm_semaphore_sizeof(void);

/**
 * @brief 等待信号量（P 操作）。
 *
 * @param sem 信号量对象
 * @param timeout_ms 超时时间（毫秒），0 表示无限等待
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_semaphore_wait(cshm_semaphore *sem, uint32_t timeout_ms);

/**
 * @brief 释放信号量（V 操作）。
 *
 * @param sem 信号量对象
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_semaphore_post(cshm_semaphore *sem);

/**
 * @brief 关闭信号量。
 *
 * @param psem 信号量对象指针的指针
 */
CC_API void CC_CALL cshm_semaphore_close(cshm_semaphore **psem);

/**
 * @brief 删除命名信号量。
 *
 * @param name 信号量名称
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_semaphore_remove(const char *name);

/** @} */

/** @name offset_ptr 可重定位指针 API
 *  @{
 */

/**
 * @brief offset_ptr 类型。
 *        在共享内存中使用相对偏移而非绝对地址，支持跨进程的指针重定位。
 */
typedef struct cshm_offset_ptr
{
    ptrdiff_t offset; /**< 相对偏移量 */
} cshm_offset_ptr;

/**
 * @brief 初始化 offset_ptr（设置为 NULL）。
 *
 * @param ptr offset_ptr 对象
 */
static inline void cshm_offset_ptr_init(cshm_offset_ptr *ptr)
{
    if (ptr) ptr->offset = 1; /* offset=1 表示 NULL */
}

/**
 * @brief 设置 offset_ptr 指向的地址。
 *
 * @param ptr offset_ptr 对象（位于共享内存中）
 * @param target 目标地址（位于同一共享内存中）
 */
static inline void cshm_offset_ptr_set(cshm_offset_ptr *ptr, void *target)
{
    if (!ptr) return;
    if (!target) {
        ptr->offset = 1; /* NULL */
    } else {
        ptr->offset = (char *)target - (char *)ptr;
    }
}

/**
 * @brief 获取 offset_ptr 指向的地址。
 *
 * @param ptr offset_ptr 对象
 * @return 目标地址，NULL 如果 offset_ptr 为空
 */
static inline void *cshm_offset_ptr_get(const cshm_offset_ptr *ptr)
{
    if (!ptr || ptr->offset == 1) return NULL;
    return (char *)ptr + ptr->offset;
}

/**
 * @brief 检查 offset_ptr 是否为空。
 *
 * @param ptr offset_ptr 对象
 * @return 1 表示为空，0 表示非空
 */
static inline int cshm_offset_ptr_is_null(const cshm_offset_ptr *ptr)
{
    return !ptr || ptr->offset == 1;
}

/** @} */

/** @name managed_shared_memory 管理型共享内存 API
 *  @{
 */

/**
 * @brief 创建或打开管理型共享内存。
 *
 * @param name 共享内存名称
 * @param size_byte 大小（字节），仅在创建时有效
 * @param mode 打开模式（OPEN_ONLY, OPEN_OR_CREATE, CREATE_ONLY）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回管理型共享内存对象，失败返回 NULL
 *
 * @note 类似 boost::interprocess::managed_shared_memory。
 *       提供内存段管理、命名对象分配等高级功能。
 *       线程安全。
 */
CC_API cshm_managed *CC_CALL cshm_managed_open(const char *name,
                                                 size_t size_byte,
                                                 cshm_open_mode mode,
                                                 cshm_errc *err);

/**
 * @brief 在管理型共享内存中分配命名对象。
 *
 * @param managed 管理型共享内存对象
 * @param name 对象名称（NULL 表示匿名对象）
 * @param size_byte 大小（字节）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回对象地址，失败返回 NULL
 *
 * @note 线程安全。如果对象已存在，返回 NULL 并设置错误码为 CSHM_ERROR_EXISTS。
 */
CC_API void *CC_CALL cshm_managed_allocate(cshm_managed *managed,
                                             const char *name,
                                             size_t size_byte,
                                             cshm_errc *err);

/**
 * @brief 在管理型共享内存中查找命名对象。
 *
 * @param managed 管理型共享内存对象
 * @param name 对象名称
 * @param out_size 输出对象大小（可为 NULL）
 * @return 成功返回对象地址，失败返回 NULL
 *
 * @note 线程安全。
 */
CC_API void *CC_CALL cshm_managed_find(cshm_managed *managed,
                                         const char *name,
                                         size_t *out_size);

/**
 * @brief 在管理型共享内存中查找或创建命名对象。
 *
 * @param managed 管理型共享内存对象
 * @param name 对象名称
 * @param size_byte 大小（字节），仅在创建时有效
 * @param created 输出是否为新创建（可为 NULL）
 * @param err 输出错误码（可为 NULL）
 * @return 成功返回对象地址，失败返回 NULL
 *
 * @note 线程安全。
 */
CC_API void *CC_CALL cshm_managed_find_or_allocate(cshm_managed *managed,
                                                     const char *name,
                                                     size_t size_byte,
                                                     int *created,
                                                     cshm_errc *err);

/**
 * @brief 释放管理型共享内存中的对象。
 *
 * @param managed 管理型共享内存对象
 * @param ptr 对象地址
 * @return 错误码
 *
 * @note 线程安全。匿名对象通过地址释放，命名对象也可以使用此方法释放。
 */
CC_API cshm_errc CC_CALL cshm_managed_deallocate(cshm_managed *managed, void *ptr);

/**
 * @brief 根据名称释放管理型共享内存中的对象。
 *
 * @param managed 管理型共享内存对象
 * @param name 对象名称
 * @return 错误码
 *
 * @note 线程安全。
 */
CC_API cshm_errc CC_CALL cshm_managed_destroy(cshm_managed *managed, const char *name);

/**
 * @brief 获取管理型共享内存的总大小。
 *
 * @param managed 管理型共享内存对象
 * @return 大小（字节），失败返回 0
 */
CC_API size_t CC_CALL cshm_managed_get_size(cshm_managed *managed);

/**
 * @brief 获取管理型共享内存的空闲空间。
 *
 * @param managed 管理型共享内存对象
 * @return 空闲空间（字节），失败返回 0
 */
CC_API size_t CC_CALL cshm_managed_get_free_memory(cshm_managed *managed);

/**
 * @brief 关闭管理型共享内存。
 *
 * @param pmanaged 管理型共享内存对象指针的指针
 *
 * @note 自动释放资源，*pmanaged 设为 NULL。线程安全。
 */
CC_API void CC_CALL cshm_managed_close(cshm_managed **pmanaged);

/**
 * @brief 删除管理型共享内存（静态方法）。
 *
 * @param name 共享内存名称
 * @return 错误码
 */
CC_API cshm_errc CC_CALL cshm_managed_remove(const char *name);

/** @} */

/** @} */ /* end of cshm group */

#ifdef __cplusplus
}
#endif

#endif /* C_CORE_SHM_H_ */
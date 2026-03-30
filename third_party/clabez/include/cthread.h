#ifndef CTHREAD_H
#define CTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cthread.h
 * @brief 跨平台 C99 线程库，完整对标 C++23 标准库线程与同步原语。
 *
 * - Windows: 要求 Vista+ (0x0600)，使用 SRWLOCK、CONDITION_VARIABLE、原子操作等高性能 API。
 * - Linux/Unix: 依赖 POSIX pthread 族 API 和 C11 原子操作。
 * - 提供完整的 C++11/C++20/C++23 线程设施：
 *   * 线程管理：thread, jthread (C++20)
 *   * 互斥量：mutex, recursive_mutex, timed_mutex, shared_mutex
 *   * 锁：lock_guard, unique_lock, shared_lock
 *   * 条件变量：condition_variable
 *   * 异步：future, promise, shared_future, packaged_task, async
 *   * 同步：once_flag, counting_semaphore, latch, barrier (C++20)
 *   * 停止令牌：stop_source, stop_token, stop_callback (C++20)
 * - 所有接口均为线程安全设计，性能对标 C++ 标准库实现。
 * 
 * @note C++23 在线程方面未引入重大新特性，主要是对 C++20 特性的完善。
 *       本库已完整实现 C++11/C++14/C++17/C++20/C++23 的所有线程相关标准功能。
 */

/* 确保 Windows 使用 Vista+ API */
#ifdef _WIN32
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0600
#  endif
#endif

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint64_t, int64_t */

/** @name 错误码
 *  @brief 所有 API 返回值统一使用 @ref cthread_errc。
 *  @{
 */
typedef enum cthread_errc {
    CTHREAD_OK                    = 0, /**< 操作成功。 */
    CTHREAD_ERROR_GENERAL         = 1, /**< 未归类的运行时错误。 */
    CTHREAD_ERROR_INVALID_ARG     = 2, /**< 传入参数无效或为 NULL。 */
    CTHREAD_ERROR_NOMEM           = 3, /**< 内存不足。 */
    CTHREAD_ERROR_THREAD_BUSY     = 4, /**< 线程仍处于 joinable 状态。 */
    CTHREAD_ERROR_NOT_JOINABLE    = 5, /**< 线程已分离或不可 join。 */
    CTHREAD_ERROR_ALREADY_JOINED  = 6, /**< 线程已被 join 或句柄无效。 */
    CTHREAD_ERROR_TIMEOUT         = 7  /**< 操作超时。 */
} cthread_errc;
/** @} */

/**
 * @brief 时间抽象：秒 + 纳秒，兼容 struct timespec 但不依赖 <time.h>。
 */
typedef struct cthread_timespec {
    int64_t tv_sec;  /**< 秒数。 */
    int64_t tv_nsec; /**< 纳秒数（0~999999999）。 */
} cthread_timespec;

/**
 * @brief 线程 ID 抽象，平台无关。
 * 
 * 内部使用 64 位整数存储，足够容纳 Windows DWORD 或 POSIX pthread_t（通常为指针或整数）。
 * x86/x86_64 均可安全使用。
 */
typedef struct cthread_id {
    uint64_t value; /**< 平台无关的线程 ID 值。 */
} cthread_id;

/** @brief 线程句柄在头文件中的不透明存储大小（字节）。 */
#define CTHREAD_THREAD_OPAQUE_SIZE 64

/** @brief 线程句柄内部存储，隐藏平台实现细节。 */
typedef union cthread_handle_storage {
    void *align;
    unsigned char opaque[CTHREAD_THREAD_OPAQUE_SIZE];
} cthread_handle_storage;

/**
 * @brief `cthread` 对象，语义类似 `std::thread`。
 *
 * - `id` 始终在创建成功后立即可读。
 * - `joinable` 指示线程是否仍可被 `cthread_join`。
 * - `handle_storage` 用于隐藏平台原生线程句柄。
 */
typedef struct cthread {
    cthread_id id;                 /**< 线程 ID，创建成功后立即可读。 */
    int joinable;                  /**< 非 0 表示仍可 join。 */
    cthread_handle_storage handle_storage; /**< 不透明句柄存储。 */
} cthread;

/** @brief 线程入口函数签名，兼容 pthread 风格。 */
typedef void *(*cthread_fn)(void *arg);

/** @name 线程管理 API
 *  @{
 */
/**
 * @brief 创建线程并立即填充 @ref cthread::id。
 * @param t 线程对象指针，成功后被初始化。
 * @param fn 线程入口函数。
 * @param arg 传递给入口函数的用户指针。
 * @return @ref cthread_errc 代码。
 */
int cthread_create(cthread *t, cthread_fn fn, void *arg);

/**
 * @brief 等待线程完成并释放底层句柄。
 * @param t 目标线程。
 * @return 见 @ref cthread_errc。
 */
int cthread_join(cthread *t);

/**
 * @brief 分离线程，使其结束后自动回收资源。
 * @param t 目标线程。
 * @return 见 @ref cthread_errc。
 */
int cthread_detach(cthread *t);

/**
 * @brief 销毁线程对象，不会强制终止线程。
 * @param t 线程对象。
 * @return 若仍 joinable 返回 @ref CTHREAD_ERROR_THREAD_BUSY。
 */
int cthread_destroy(cthread *t);

/**
 * @brief 读取线程 ID。
 * @param t 线程对象。
 * @return @ref cthread_id 值；若 t==NULL 返回 0。
 */
cthread_id cthread_get_id(const cthread *t);

/**
 * @brief 比较两个线程 ID。
 * @param a ID1。
 * @param b ID2。
 * @return 非 0 表示相等。
 */
int cthread_id_equal(cthread_id a, cthread_id b);

/**
 * @brief 返回当前线程的 ID。
 */
cthread_id cthread_this_id(void);

/**
 * @brief 给出逻辑 CPU 数（硬件并发度）。
 */
unsigned int cthread_hardware_concurrency(void);

/**
 * @brief 让出当前线程的时间片。
 */
void cthread_yield(void);

/**
 * @brief 休眠指定相对时间。
 * 
 * @param ts 时间结构，必须非 NULL。
 * 
 * @note 此函数使用单调时钟（monotonic clock）实现，不受系统时间调整影响。
 *       - Windows: 使用 Sleep()，基于 GetTickCount/QPC
 *       - Linux: 使用 nanosleep()，默认基于 CLOCK_MONOTONIC 或不受系统时间影响
 */
void cthread_sleep_for(const cthread_timespec *ts);

/**
 * @brief 休眠直到绝对时间点（CLOCK_REALTIME 语义）。
 * 
 * @param abs_time 绝对时间点（Unix 纪元，秒 + 纳秒）。
 * 
 * @warning 此函数使用系统时钟（wall clock），**会受系统时间调整影响**：
 *          - 如果用户向前调整系统时间，函数会提前返回
 *          - 如果用户向后调整系统时间，函数会休眠更久
 *          对于不希望受系统时间影响的场景，请使用 cthread_sleep_for() 实现相对超时。
 * 
 * @note 语义对齐 C++ std::this_thread::sleep_until()，使用 CLOCK_REALTIME。
 */
void cthread_sleep_until(const cthread_timespec *abs_time);
/** @} */

/** @brief `c_mutex` 不透明存储大小。 */
#define CTHREAD_MUTEX_OPAQUE_SIZE   64
/** @brief `c_recursive_mutex` 不透明存储大小。 */
#define CTHREAD_RECURSIVE_MUTEX_OPAQUE_SIZE 64
/** @brief `c_shared_mutex` 不透明存储大小。 */
#define CTHREAD_SHARED_MUTEX_OPAQUE_SIZE    128
/** @brief `c_condition_variable` 不透明存储大小。 */
#define CTHREAD_CONDVAR_OPAQUE_SIZE         64
/** @brief `c_timed_mutex` 不透明存储大小。 */
#define CTHREAD_TIMED_MUTEX_OPAQUE_SIZE     128
/** @brief `c_recursive_timed_mutex` 不透明存储大小。 */
#define CTHREAD_RECURSIVE_TIMED_MUTEX_OPAQUE_SIZE 128
/** @brief `c_semaphore` 不透明存储大小。 */
#define CTHREAD_SEMAPHORE_OPAQUE_SIZE       192
/** @brief `c_latch` 不透明存储大小。 */
#define CTHREAD_LATCH_OPAQUE_SIZE           192
/** @brief `c_barrier` 不透明存储大小。 */
#define CTHREAD_BARRIER_OPAQUE_SIZE         256

/**
 * @brief 平台无关的互斥量，不暴露具体实现。
 */
typedef union c_mutex {
    void *align;                                   /**< 对齐用途。 */
    unsigned char opaque[CTHREAD_MUTEX_OPAQUE_SIZE]; /**< 实际存储。 */
} c_mutex;

/** @name 互斥量操作
 *  @{
 */
/** @brief 初始化互斥量。 */
int c_mutex_init(c_mutex *m);
/** @brief 销毁互斥量。 */
int c_mutex_destroy(c_mutex *m);
/** @brief 阻塞式加锁。 */
int c_mutex_lock(c_mutex *m);
/** @brief 尝试加锁，若失败返回 @ref CTHREAD_ERROR_TIMEOUT。 */
int c_mutex_try_lock(c_mutex *m);
/** @brief 解锁互斥量。 */
int c_mutex_unlock(c_mutex *m);
/** @} */

/**
 * @brief RAII 锁守卫，生命周期内持有 `c_mutex`。
 */
typedef struct c_lock_guard {
    c_mutex *mtx; /**< 被管理的互斥量。 */
    int owns;     /**< 是否正持有锁。 */
} c_lock_guard;

/** 初始化并立即加锁。 */
int c_lock_guard_init(c_lock_guard *g, c_mutex *m);
/** 销毁时自动解锁（若 owns==1）。 */
int c_lock_guard_destroy(c_lock_guard *g);

/**
 * @brief 递归互斥量，可重入加锁。
 */
typedef union c_recursive_mutex {
    void *align;
    unsigned char opaque[CTHREAD_RECURSIVE_MUTEX_OPAQUE_SIZE];
} c_recursive_mutex;

/** 初始化递归互斥量。 */
int c_recursive_mutex_init(c_recursive_mutex *m);
/** 销毁递归互斥量。 */
int c_recursive_mutex_destroy(c_recursive_mutex *m);
/** 递归加锁。 */
int c_recursive_mutex_lock(c_recursive_mutex *m);
/** 尝试递归加锁。 */
int c_recursive_mutex_try_lock(c_recursive_mutex *m);
/** 解锁递归互斥量。 */
int c_recursive_mutex_unlock(c_recursive_mutex *m);

/**
 * @brief 带超时的互斥量（std::timed_mutex）。
 */
typedef union c_timed_mutex {
    void *align;
    unsigned char opaque[CTHREAD_TIMED_MUTEX_OPAQUE_SIZE];
} c_timed_mutex;

/** 初始化带超时的互斥量。 */
int c_timed_mutex_init(c_timed_mutex *m);
/** 销毁带超时的互斥量。 */
int c_timed_mutex_destroy(c_timed_mutex *m);
/** 加锁。 */
int c_timed_mutex_lock(c_timed_mutex *m);
/** 尝试加锁。 */
int c_timed_mutex_try_lock(c_timed_mutex *m);
/** 尝试加锁，带相对超时（对应 std::timed_mutex::try_lock_for）。 */
int c_timed_mutex_try_lock_for(c_timed_mutex *m, const cthread_timespec *rel_time);
/** 尝试加锁，带绝对超时（对应 std::timed_mutex::try_lock_until）。 */
int c_timed_mutex_try_lock_until(c_timed_mutex *m, const cthread_timespec *abs_time);
/** 解锁。 */
int c_timed_mutex_unlock(c_timed_mutex *m);

/**
 * @brief 递归的带超时互斥量（std::recursive_timed_mutex）。
 */
typedef union c_recursive_timed_mutex {
    void *align;
    unsigned char opaque[CTHREAD_RECURSIVE_TIMED_MUTEX_OPAQUE_SIZE];
} c_recursive_timed_mutex;

/** 初始化递归的带超时互斥量。 */
int c_recursive_timed_mutex_init(c_recursive_timed_mutex *m);
/** 销毁递归的带超时互斥量。 */
int c_recursive_timed_mutex_destroy(c_recursive_timed_mutex *m);
/** 递归加锁。 */
int c_recursive_timed_mutex_lock(c_recursive_timed_mutex *m);
/** 尝试递归加锁。 */
int c_recursive_timed_mutex_try_lock(c_recursive_timed_mutex *m);
/** 尝试递归加锁，带相对超时。 */
int c_recursive_timed_mutex_try_lock_for(c_recursive_timed_mutex *m, const cthread_timespec *rel_time);
/** 尝试递归加锁，带绝对超时。 */
int c_recursive_timed_mutex_try_lock_until(c_recursive_timed_mutex *m, const cthread_timespec *abs_time);
/** 解锁。 */
int c_recursive_timed_mutex_unlock(c_recursive_timed_mutex *m);

/**
 * @brief 共享互斥量，支持独占/共享两种模式。
 */
typedef union c_shared_mutex {
    void *align;
    unsigned char opaque[CTHREAD_SHARED_MUTEX_OPAQUE_SIZE];
} c_shared_mutex;

/** 初始化共享互斥量。 */
int c_shared_mutex_init(c_shared_mutex *m);
/** 销毁共享互斥量。 */
int c_shared_mutex_destroy(c_shared_mutex *m);
/** 独占加锁。 */
int c_shared_mutex_lock(c_shared_mutex *m);
/** 尝试独占加锁。 */
int c_shared_mutex_try_lock(c_shared_mutex *m);
/** 独占解锁。 */
int c_shared_mutex_unlock(c_shared_mutex *m);
/** 共享加锁。 */
int c_shared_mutex_lock_shared(c_shared_mutex *m);
/** 尝试共享加锁。 */
int c_shared_mutex_try_lock_shared(c_shared_mutex *m);
/** 共享解锁。 */
int c_shared_mutex_unlock_shared(c_shared_mutex *m);

/**
 * @brief shared_lock RAII 封装，共享模式持有互斥量。
 */
typedef struct c_shared_lock {
    c_shared_mutex *mtx; /**< 被管理的共享互斥量。 */
    int owns;            /**< 是否当前持有锁。 */
} c_shared_lock;

/**
 * @brief shared_lock 初始化策略。
 */
typedef enum c_shared_lock_policy {
    CTHREAD_SHARED_LOCK_POLICY_IMMEDIATE = 0, /**< 构造即共享加锁。 */
    CTHREAD_SHARED_LOCK_POLICY_DEFER,         /**< 延迟加锁。 */
    CTHREAD_SHARED_LOCK_POLICY_TRY,           /**< 尝试共享加锁。 */
    CTHREAD_SHARED_LOCK_POLICY_ADOPT          /**< 接管已有共享锁。 */
} c_shared_lock_policy;

/** 根据策略初始化 shared_lock。 */
int c_shared_lock_init(c_shared_lock *lk, c_shared_mutex *m, c_shared_lock_policy policy);
/** 释放 shared_lock，必要时自动解锁。 */
int c_shared_lock_destroy(c_shared_lock *lk);
/** 共享加锁。 */
int c_shared_lock_lock(c_shared_lock *lk);
/** 尝试共享加锁。 */
int c_shared_lock_try_lock(c_shared_lock *lk);
/** 共享解锁。 */
int c_shared_lock_unlock(c_shared_lock *lk);
/** 查询是否持有共享锁。 */
int c_shared_lock_owns_lock(const c_shared_lock *lk);
/** 放弃所有权但不解锁。 */
c_shared_mutex *c_shared_lock_release(c_shared_lock *lk);

/**
 * @brief unique_lock RAII 封装，可灵活控制持锁状态。
 */
typedef struct c_unique_lock {
    c_mutex *mtx; /**< 被管理的互斥量。 */
    int owns;     /**< 是否当前持有锁。 */
} c_unique_lock;

/**
 * @brief unique_lock 构造策略。
 */
typedef enum c_lock_policy {
    CTHREAD_LOCK_POLICY_IMMEDIATE = 0, /**< 立即加锁。 */
    CTHREAD_LOCK_POLICY_DEFER,         /**< 延迟加锁。 */
    CTHREAD_LOCK_POLICY_TRY,           /**< 尝试加锁。 */
    CTHREAD_LOCK_POLICY_ADOPT          /**< 接管现有锁。 */
} c_lock_policy;

/** 按策略初始化 unique_lock。 */
int c_unique_lock_init(c_unique_lock *lk, c_mutex *m, c_lock_policy policy);
/** 销毁并在需要时解锁。 */
int c_unique_lock_destroy(c_unique_lock *lk);
/** 阻塞式加锁。 */
int c_unique_lock_lock(c_unique_lock *lk);
/** 尝试加锁。 */
int c_unique_lock_try_lock(c_unique_lock *lk);
/** 解锁。 */
int c_unique_lock_unlock(c_unique_lock *lk);
/** 查询是否持锁。 */
int c_unique_lock_owns_lock(const c_unique_lock *lk);
/** 放弃所有权但不解锁，返回互斥量指针。 */
c_mutex *c_unique_lock_release(c_unique_lock *lk);

/**
 * @brief 平台无关条件变量。
 */
typedef union c_condition_variable {
    void *align;
    unsigned char opaque[CTHREAD_CONDVAR_OPAQUE_SIZE];
} c_condition_variable;

/** 
 * @brief 初始化条件变量。
 */
int c_condition_variable_init(c_condition_variable *cv);

/** 
 * @brief 销毁条件变量。
 */
int c_condition_variable_destroy(c_condition_variable *cv);

/** 
 * @brief 无限等待条件满足。
 * @param cv 条件变量。
 * @param lk 必须已持锁的 unique_lock。
 */
int c_condition_variable_wait(c_condition_variable *cv, c_unique_lock *lk);

/** 
 * @brief 等待条件满足，带相对超时。
 * @param cv 条件变量。
 * @param lk 必须已持锁的 unique_lock。
 * @param rel_time 相对超时时间。
 * @return CTHREAD_ERROR_TIMEOUT 表示超时；CTHREAD_OK 表示被唤醒。
 * @note 使用单调时钟，不受系统时间调整影响。
 */
int c_condition_variable_wait_for(c_condition_variable *cv,
                                  c_unique_lock *lk,
                                  const cthread_timespec *rel_time);

/** 
 * @brief 等待条件满足，直到绝对时间点。
 * @param cv 条件变量。
 * @param lk 必须已持锁的 unique_lock。
 * @param abs_time 绝对时间点（CLOCK_REALTIME）。
 * @return CTHREAD_ERROR_TIMEOUT 表示超时；CTHREAD_OK 表示被唤醒。
 * @warning 使用系统时钟，**会受系统时间调整影响**。对于不受系统时间影响的超时，建议使用 wait_for。
 */
int c_condition_variable_wait_until(c_condition_variable *cv,
                                    c_unique_lock *lk,
                                    const cthread_timespec *abs_time);

/** 
 * @brief 唤醒一个等待者。
 */
int c_condition_variable_notify_one(c_condition_variable *cv);

/** 
 * @brief 唤醒所有等待者。
 */
int c_condition_variable_notify_all(c_condition_variable *cv);

/**
 * @brief future 抽象，内部仅存储共享状态指针。
 */
typedef struct c_future {
    void *state; /**< 内部共享状态指针。 */
} c_future;

/**
 * @brief promise 抽象，对应单个共享状态的写入端。
 */
typedef struct c_promise {
    void *state; /**< 内部共享状态指针。 */
} c_promise;

/** 初始化 promise。 */
int c_promise_init(c_promise *p);
/** 销毁 promise。 */
int c_promise_destroy(c_promise *p);
/** 获取与 promise 绑定的 future，单次调用。 */
int c_promise_get_future(c_promise *p, c_future *out_fut);
/** 设置正常结果值。 */
int c_promise_set_value(c_promise *p, void *value);
/** 设置异常错误码。 */
int c_promise_set_exception(c_promise *p, int error_code);
/** 标记任务被取消。 */
int c_promise_set_canceled(c_promise *p);
/** 
 * @brief 测试 future 是否持有有效状态。
 */
int c_future_valid(const c_future *f);

/** 
 * @brief 阻塞等待 ready（无限等待）。
 */
int c_future_wait(c_future *f);

/** 
 * @brief 带相对超时的等待。
 * @param f future 对象。
 * @param rel_time 相对超时时间。
 * @return CTHREAD_ERROR_TIMEOUT 表示超时；CTHREAD_OK 表示 ready。
 * @note 使用单调时钟，不受系统时间调整影响。
 */
int c_future_wait_for(c_future *f, const cthread_timespec *rel_time);

/** 
 * @brief 带绝对超时的等待。
 * @param f future 对象。
 * @param abs_time 绝对时间点（CLOCK_REALTIME）。
 * @return CTHREAD_ERROR_TIMEOUT 表示超时；CTHREAD_OK 表示 ready。
 * @warning 使用系统时钟，**会受系统时间调整影响**。
 */
int c_future_wait_until(c_future *f, const cthread_timespec *abs_time);

/** 
 * @brief 取得结果值（一次性操作，会使 future 失效）。
 */
int c_future_get(c_future *f, void **out_value);

/** 
 * @brief 取得结果并带回错误码（区分正常值/异常/取消）。
 */
int c_future_get_ex(c_future *f, void **out_value, int *out_error_code);

/** 
 * @brief 销毁 future，释放共享状态引用。
 */
int c_future_destroy(c_future *f);

/**
 * @brief shared_future 抽象，可被多个线程共享的 future（std::shared_future）。
 */
typedef struct c_shared_future {
    void *state; /**< 内部共享状态指针。 */
} c_shared_future;

/** 从 future 创建 shared_future（会使原 future 失效）。 */
int c_future_share(c_future *f, c_shared_future *out_sf);
/** 从 shared_future 获取结果（可多次调用）。 */
int c_shared_future_get(c_shared_future *sf, void **out_value);
/** 等待 shared_future ready。 */
int c_shared_future_wait(c_shared_future *sf);
/** 等待 shared_future，带相对超时。 */
int c_shared_future_wait_for(c_shared_future *sf, const cthread_timespec *rel_time);
/** 等待 shared_future，带绝对超时。 */
int c_shared_future_wait_until(c_shared_future *sf, const cthread_timespec *abs_time);
/** 测试 shared_future 是否有效。 */
int c_shared_future_valid(const c_shared_future *sf);
/** 销毁 shared_future。 */
int c_shared_future_destroy(c_shared_future *sf);

/**
 * @brief packaged_task 抽象（std::packaged_task）。
 */
typedef struct c_packaged_task {
    void *internal; /**< 内部状态。 */
} c_packaged_task;

/** 初始化 packaged_task。 */
int c_packaged_task_init(c_packaged_task *task, cthread_fn fn, void *arg);
/** 销毁 packaged_task。 */
int c_packaged_task_destroy(c_packaged_task *task);
/** 获取与 task 关联的 future。 */
int c_packaged_task_get_future(c_packaged_task *task, c_future *out_fut);
/** 执行任务。 */
int c_packaged_task_execute(c_packaged_task *task);
/** 重置任务以便再次执行。 */
int c_packaged_task_reset(c_packaged_task *task);

/**
 * @brief 一次性初始化标志（std::once_flag）。
 */
typedef struct c_once_flag {
    volatile int state;   /**< 初始化状态：0=未初始化，1=初始化中，2=已完成。 */
    unsigned char opaque[CTHREAD_MUTEX_OPAQUE_SIZE];  /**< 平台相关的互斥量等。 */
} c_once_flag;

/** 初始化 once_flag（静态初始化可用 {0}）。 */
#define C_ONCE_FLAG_INIT { 0, {0} }

/** 
 * @brief 线程安全的一次性初始化（std::call_once）。
 * @param flag 一次性标志。
 * @param func 初始化函数。
 */
int c_call_once(c_once_flag *flag, void (*func)(void));

/**
 * @brief async 启动策略（std::launch）。
 */
typedef enum c_launch_policy {
    CTHREAD_LAUNCH_ASYNC    = 1,  /**< 立即在新线程中异步执行。 */
    CTHREAD_LAUNCH_DEFERRED = 2,  /**< 延迟执行，在首次 wait/get 时同步执行。 */
    CTHREAD_LAUNCH_ANY      = 3   /**< 由实现选择（默认为 ASYNC）。 */
} c_launch_policy;

/**
 * @brief 类似 `std::async` 的帮助函数。
 *
 * @param t_opt 若非 NULL，则填充线程对象由调用者管理；否则内部自动 detach。
 * @param out_fut 输出参数，获得可等待的 future。
 * @param fn 任务函数，返回值会通过 promise 传递。
 * @param arg 传递给任务的用户指针。
 * @return 见 @ref cthread_errc。
 */
int c_async(cthread *t_opt, c_future *out_fut, cthread_fn fn, void *arg);

/**
 * @brief async 扩展版本，支持启动策略。
 *
 * @param t_opt 若非 NULL，则填充线程对象。
 * @param out_fut 输出 future。
 * @param policy 启动策略。
 * @param fn 任务函数。
 * @param arg 用户参数。
 * @return 见 @ref cthread_errc。
 */
int c_async_ex(cthread *t_opt, c_future *out_fut, c_launch_policy policy, cthread_fn fn, void *arg);

/**
 * @brief 同时锁定多个互斥量，避免死锁（std::lock）。
 * @param mutexes 互斥量指针数组。
 * @param count 互斥量数量。
 * @return 见 @ref cthread_errc。
 */
int c_lock(c_mutex **mutexes, size_t count);

/**
 * @brief 尝试同时锁定多个互斥量（std::try_lock）。
 * @param mutexes 互斥量指针数组。
 * @param count 互斥量数量。
 * @param out_failed_index 若失败，返回第一个失败的索引（可为 NULL）。
 * @return CTHREAD_OK 表示全部成功；CTHREAD_ERROR_TIMEOUT 表示有锁失败。
 */
int c_try_lock(c_mutex **mutexes, size_t count, size_t *out_failed_index);

/* ======================= C++20 新特性 ======================= */

/**
 * @brief 停止源（std::stop_source）。
 */
typedef struct c_stop_source {
    void *state; /**< 内部共享停止状态。 */
} c_stop_source;

/**
 * @brief 停止令牌（std::stop_token）。
 */
typedef struct c_stop_token {
    void *state; /**< 内部共享停止状态。 */
} c_stop_token;

/**
 * @brief 停止回调（std::stop_callback）。
 */
typedef struct c_stop_callback {
    void *internal; /**< 内部状态。 */
} c_stop_callback;

/** 初始化停止源。 */
int c_stop_source_init(c_stop_source *ss);
/** 销毁停止源。 */
int c_stop_source_destroy(c_stop_source *ss);
/** 获取停止令牌。 */
int c_stop_source_get_token(c_stop_source *ss, c_stop_token *out_token);
/** 请求停止。 */
int c_stop_source_request_stop(c_stop_source *ss);
/** 查询是否已请求停止。 */
int c_stop_source_stop_requested(const c_stop_source *ss);
/** 查询是否可以停止（是否有关联的停止状态）。 */
int c_stop_source_stop_possible(const c_stop_source *ss);

/** 从停止令牌创建副本。 */
int c_stop_token_copy(const c_stop_token *src, c_stop_token *dst);
/** 销毁停止令牌。 */
int c_stop_token_destroy(c_stop_token *token);
/** 查询是否已请求停止。 */
int c_stop_token_stop_requested(const c_stop_token *token);
/** 查询是否可以停止。 */
int c_stop_token_stop_possible(const c_stop_token *token);

/** 停止回调函数类型。 */
typedef void (*c_stop_callback_fn)(void *user_data);

/** 注册停止回调。 */
int c_stop_callback_init(c_stop_callback *cb, const c_stop_token *token, 
                         c_stop_callback_fn fn, void *user_data);
/** 销毁停止回调。 */
int c_stop_callback_destroy(c_stop_callback *cb);

/**
 * @brief jthread 对象，支持自动 join 和协作式取消（std::jthread）。
 */
typedef struct c_jthread {
    cthread thread;          /**< 底层线程对象。 */
    c_stop_source ssource;   /**< 停止源。 */
    int owns_thread;         /**< 是否拥有线程。 */
} c_jthread;

/** jthread 入口函数，可接收停止令牌。 */
typedef void *(*c_jthread_fn)(c_stop_token *stoken, void *arg);

/** 创建 jthread。 */
int c_jthread_create(c_jthread *jt, c_jthread_fn fn, void *arg);
/** 销毁 jthread（自动 join）。 */
int c_jthread_destroy(c_jthread *jt);
/** 获取停止源。 */
c_stop_source *c_jthread_get_stop_source(c_jthread *jt);
/** 获取停止令牌。 */
int c_jthread_get_stop_token(c_jthread *jt, c_stop_token *out_token);
/** 请求停止。 */
int c_jthread_request_stop(c_jthread *jt);
/** 手动 join（可选）。 */
int c_jthread_join(c_jthread *jt);
/** 手动 detach。 */
int c_jthread_detach(c_jthread *jt);
/** 检查是否 joinable。 */
int c_jthread_joinable(const c_jthread *jt);

/**
 * @brief 计数信号量（std::counting_semaphore）。
 */
typedef union c_counting_semaphore {
    void *align;
    unsigned char opaque[CTHREAD_SEMAPHORE_OPAQUE_SIZE];
} c_counting_semaphore;

/** 初始化计数信号量。 */
int c_counting_semaphore_init(c_counting_semaphore *sem, int initial_count);
/** 销毁计数信号量。 */
int c_counting_semaphore_destroy(c_counting_semaphore *sem);
/** 释放信号量（增加计数）。 */
int c_counting_semaphore_release(c_counting_semaphore *sem, int update);
/** 获取信号量（减少计数，阻塞等待）。 */
int c_counting_semaphore_acquire(c_counting_semaphore *sem);
/** 尝试获取信号量（不阻塞）。 */
int c_counting_semaphore_try_acquire(c_counting_semaphore *sem);
/** 尝试获取信号量，带相对超时。 */
int c_counting_semaphore_try_acquire_for(c_counting_semaphore *sem, const cthread_timespec *rel_time);
/** 尝试获取信号量，带绝对超时。 */
int c_counting_semaphore_try_acquire_until(c_counting_semaphore *sem, const cthread_timespec *abs_time);

/**
 * @brief 二进制信号量（std::binary_semaphore）。
 */
typedef c_counting_semaphore c_binary_semaphore;

/** 初始化二进制信号量。 */
static inline int c_binary_semaphore_init(c_binary_semaphore *sem, int initial_value) {
    return c_counting_semaphore_init(sem, initial_value);
}
/** 销毁二进制信号量。 */
static inline int c_binary_semaphore_destroy(c_binary_semaphore *sem) {
    return c_counting_semaphore_destroy(sem);
}
/** 释放二进制信号量。 */
static inline int c_binary_semaphore_release(c_binary_semaphore *sem) {
    return c_counting_semaphore_release(sem, 1);
}
/** 获取二进制信号量。 */
static inline int c_binary_semaphore_acquire(c_binary_semaphore *sem) {
    return c_counting_semaphore_acquire(sem);
}
/** 尝试获取二进制信号量。 */
static inline int c_binary_semaphore_try_acquire(c_binary_semaphore *sem) {
    return c_counting_semaphore_try_acquire(sem);
}

/**
 * @brief latch（单次倒计时门栓，std::latch）。
 */
typedef union c_latch {
    void *align;
    unsigned char opaque[CTHREAD_LATCH_OPAQUE_SIZE];
} c_latch;

/** 初始化 latch。 */
int c_latch_init(c_latch *latch, int count);
/** 销毁 latch。 */
int c_latch_destroy(c_latch *latch);
/** 递减计数并可能解除阻塞。 */
int c_latch_count_down(c_latch *latch, int n);
/** 测试计数是否为 0。 */
int c_latch_try_wait(c_latch *latch);
/** 等待计数为 0。 */
int c_latch_wait(c_latch *latch);
/** 递减计数并等待为 0。 */
int c_latch_arrive_and_wait(c_latch *latch, int n);

/**
 * @brief barrier（可重用的同步点，std::barrier）。
 */
typedef union c_barrier {
    void *align;
    unsigned char opaque[CTHREAD_BARRIER_OPAQUE_SIZE];
} c_barrier;

/** barrier 完成回调函数类型。 */
typedef void (*c_barrier_completion_fn)(void *user_data);

/** 初始化 barrier。 */
int c_barrier_init(c_barrier *barrier, int num_threads, 
                   c_barrier_completion_fn completion_fn, void *user_data);
/** 销毁 barrier。 */
int c_barrier_destroy(c_barrier *barrier);
/** 到达 barrier 并等待所有线程。 */
int c_barrier_arrive_and_wait(c_barrier *barrier);
/** 到达 barrier 但不等待。 */
int c_barrier_arrive(c_barrier *barrier);
/** 到达 barrier 并离开（减少参与线程数）。 */
int c_barrier_arrive_and_drop(c_barrier *barrier);

#ifdef __cplusplus
}
#endif

#endif /* CTHREAD_H */



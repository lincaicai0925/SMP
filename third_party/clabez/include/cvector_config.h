/**
 * @file cvector_config.h
 * @brief cvector 配置文件 - 平台相关的配置和优化
 * @details 针对不同平台（STM32H743、标准PC等）提供优化配置
 */

#ifndef CVECTOR_CONFIG_H_
#define CVECTOR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== STM32H743 专用配置 ==================== */
#ifdef STM32H743xx

    /* 包含 HAL 库以使用中断控制 */
    #if defined(USE_HAL_DRIVER)
        #include "stm32h7xx_hal.h"
    #else
        #include "stm32h743xx.h"
        /* 手动定义中断控制宏（如果不使用HAL） */
        #define __get_PRIMASK() __builtin_arm_rsr("PRIMASK")
        #define __set_PRIMASK(x) __builtin_arm_wsr("PRIMASK", (x))
        #define __disable_irq() __builtin_arm_disable_irq()
        #define __enable_irq() __builtin_arm_enable_irq()
        #define __get_IPSR() __builtin_arm_rsr("IPSR")
    #endif

    /**
     * @brief 中断安全的临界区保护
     * @details 保存中断状态并禁用中断，确保多任务/中断环境下的线程安全
     * @note 使用 PRIMASK 保存/恢复中断状态，支持嵌套调用
     * @note 添加内存屏障防止编译器优化重排内存访问
     */
    #define CVECTOR_LOCK() \
        uint32_t cvector_primask_save = __get_PRIMASK(); \
        __disable_irq(); \
        __asm volatile("" ::: "memory")  /* 编译器内存屏障 */

    #define CVECTOR_UNLOCK() \
        __asm volatile("" ::: "memory"); /* 编译器内存屏障 */ \
        if (!cvector_primask_save) { \
            __enable_irq(); \
        }

    /**
     * @brief 栈安全配置
     * @details STM32H743 典型任务栈大小为 2-8KB，需要限制栈使用
     */
    #define CVECTOR_STACK_BUFFER_SIZE 64  /**< 栈缓冲区大小：64字节（避免栈溢出） */

    /**
     * @brief 容量限制
     * @details 限制最大容量以避免内存耗尽
     */
    #define CVECTOR_MAX_CAPACITY 32768  /**< 最大容量：32K 元素（取决于可用 RAM） */

    /**
     * @brief 内存分配策略
     */
    #define CVECTOR_INITIAL_CAPACITY 4  /**< 初始容量：4 个元素 */
    #define CVECTOR_LARGE_CAPACITY_THRESHOLD 512  /**< 大容量阈值：超过此值使用 25% 增长 */

    /**
     * @brief 排序算法阈值
     * @details 小于此值使用插入排序，大于此值使用堆排序（栈安全）
     */
    #define INSERTION_SORT_THRESHOLD 16  /**< 插入排序阈值 */

    /**
     * @brief Release 模式也启用边界检查
     * @details 增强安全性，防止越界访问
     */
    #define CVECTOR_ALWAYS_CHECK_BOUNDS 1

    /**
     * @brief 启用内存使用跟踪
     * @details 用于监控内存使用情况
     */
    #define CVECTOR_ENABLE_MEMORY_TRACKING 1

    /**
     * @brief 断言：不应在中断服务程序中使用 cvector
     * @details 检查 IPSR 寄存器，确保不在中断上下文中调用
     * @warning 在中断中使用 cvector 会导致死锁或数据竞争
     */
    #define CVECTOR_ASSERT_NOT_IN_ISR() \
        do { \
            if ((__get_IPSR() & 0x1FF) != 0) { \
                /* 在中断中调用，触发错误 */ \
                while(1); /* 或者调用 Error_Handler() */ \
            } \
        } while(0)

/* ==================== 其他嵌入式平台（通用配置） ==================== */
#elif defined(__arm__) || defined(__EMBEDDED__)

    /* 基本的中断保护（需要用户实现） */
    extern void cvector_enter_critical(void);
    extern void cvector_exit_critical(void);
    
    #define CVECTOR_LOCK() cvector_enter_critical()
    #define CVECTOR_UNLOCK() cvector_exit_critical()
    
    /* 保守的栈和容量限制 */
    #define CVECTOR_STACK_BUFFER_SIZE 64
    #define CVECTOR_MAX_CAPACITY 16384
    #define CVECTOR_INITIAL_CAPACITY 4
    #define CVECTOR_LARGE_CAPACITY_THRESHOLD 256
    #define INSERTION_SORT_THRESHOLD 16
    
    /* 启用边界检查 */
    #define CVECTOR_ALWAYS_CHECK_BOUNDS 1
    #define CVECTOR_ENABLE_MEMORY_TRACKING 1
    #define CVECTOR_ASSERT_NOT_IN_ISR() ((void)0)

/* ==================== PC/Linux/Windows 标准平台 ==================== */
#else

    /* 无需中断保护（单线程或用户自行处理） */
    #define CVECTOR_LOCK() ((void)0)
    #define CVECTOR_UNLOCK() ((void)0)

    /* 宽松的栈和容量限制 */
    #define CVECTOR_STACK_BUFFER_SIZE 256  /**< 大栈缓冲区 */
    #define CVECTOR_MAX_CAPACITY SIZE_MAX  /**< 无限制 */
    #define CVECTOR_INITIAL_CAPACITY 4
    #define CVECTOR_LARGE_CAPACITY_THRESHOLD 4096
    #define INSERTION_SORT_THRESHOLD 32

    /* Debug 模式才启用边界检查 */
    #ifndef CVECTOR_ALWAYS_CHECK_BOUNDS
        #define CVECTOR_ALWAYS_CHECK_BOUNDS 0
    #endif
    
    /* 可选的内存跟踪 */
    #ifndef CVECTOR_ENABLE_MEMORY_TRACKING
        #define CVECTOR_ENABLE_MEMORY_TRACKING 0
    #endif
    
    #define CVECTOR_ASSERT_NOT_IN_ISR() ((void)0)

#endif

/* ==================== 错误码定义 ==================== */

/**
 * @brief cvector 错误码
 */
typedef enum {
    CVECTOR_OK = 0,              /**< 操作成功 */
    CVECTOR_ERROR_NOMEM,         /**< 内存不足 */
    CVECTOR_ERROR_INVALID,       /**< 无效参数 */
    CVECTOR_ERROR_BOUNDS,        /**< 越界访问 */
    CVECTOR_ERROR_OVERFLOW       /**< 容量溢出 */
} cvector_error_t;

/* ==================== 调试配置 ==================== */

/**
 * @brief 启用详细的错误报告
 * @details 定义此宏以启用详细的错误信息（会增加代码大小）
 */
/* #define CVECTOR_ENABLE_VERBOSE_ERRORS */

/**
 * @brief 启用性能分析
 * @details 定义此宏以启用性能计数器（会有小幅性能损失）
 */
/* #define CVECTOR_ENABLE_PROFILING */

#ifdef __cplusplus
}
#endif

#endif /* CVECTOR_CONFIG_H_ */

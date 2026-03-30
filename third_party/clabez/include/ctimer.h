/**
 * @file ctimer.h
 * @brief 异步定时器，参考boost::asio::steady_timer设计
 * @details 提供异步定时功能，支持超时、延迟执行等
 * @author labez_core
 * @date 2025-11-22
 */

#ifndef LABEZ_CTIMER_H
#define LABEZ_CTIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciosrv.h"
#include <stdint.h>
#include <stddef.h>

/* API导出定义 */
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

/**
 * @brief 定时器错误码
 */
typedef enum {
    CTIMER_OK = 0,                      /**< 成功 */
    CTIMER_ERROR_INVALID_PARAM,         /**< 无效参数 */
    CTIMER_ERROR_CREATE,                /**< 创建失败 */
    CTIMER_ERROR_MEMORY,                /**< 内存分配失败 */
    CTIMER_ERROR_CANCELLED,             /**< 定时器已取消 */
    CTIMER_ERROR_SYSTEM,                /**< 系统错误 */
    CTIMER_ERROR_UNKNOWN                /**< 未知错误 */
} ctimer_error_t;

/**
 * @brief 定时器句柄（不透明结构）
 */
typedef struct ctimer_t ctimer_t;

/**
 * @brief 定时器回调函数类型
 * @param timer 定时器句柄
 * @param error 错误码
 * @param user_data 用户数据
 */
typedef void (*ctimer_handler_t)(ctimer_t* timer, ctimer_error_t error, void* user_data);

/* ========== 创建和销毁 ========== */

/**
 * @brief 创建定时器
 * @param io_service IO服务
 * @param timer 输出定时器指针
 * @return 错误码
 */
CC_API ctimer_error_t CC_CALL ctimer_create(ciosrv_t* io_service, ctimer_t** timer);

/**
 * @brief 销毁定时器
 * @param timer 定时器指针的指针
 * @return 错误码
 * @note 会自动取消所有待执行的定时器
 */
CC_API ctimer_error_t CC_CALL ctimer_destroy(ctimer_t** timer);

/* ========== 定时器操作 ========== */

/**
 * @brief 异步等待（一次性）
 * @param timer 定时器指针
 * @param timeout_ms 超时时间（毫秒）
 * @param handler 回调函数
 * @param user_data 用户数据
 * @return 错误码
 * @note 在指定时间后调用回调函数，如果在此之前调用cancel，回调会收到CTIMER_ERROR_CANCELLED
 */
CC_API ctimer_error_t CC_CALL ctimer_async_wait(ctimer_t* timer, 
                                                  uint32_t timeout_ms,
                                                  ctimer_handler_t handler,
                                                  void* user_data);

/**
 * @brief 异步等待（重复）
 * @param timer 定时器指针
 * @param interval_ms 间隔时间（毫秒）
 * @param handler 回调函数
 * @param user_data 用户数据
 * @return 错误码
 * @note 每隔指定时间调用一次回调函数，直到调用cancel
 */
CC_API ctimer_error_t CC_CALL ctimer_async_wait_repeat(ctimer_t* timer,
                                                         uint32_t interval_ms,
                                                         ctimer_handler_t handler,
                                                         void* user_data);

/**
 * @brief 取消定时器
 * @param timer 定时器指针
 * @return 错误码
 * @note 取消后，待执行的回调会收到CTIMER_ERROR_CANCELLED错误
 */
CC_API ctimer_error_t CC_CALL ctimer_cancel(ctimer_t* timer);

/**
 * @brief 检查定时器是否已取消
 * @param timer 定时器指针
 * @return 1表示已取消，0表示未取消
 */
CC_API int CC_CALL ctimer_is_cancelled(ctimer_t* timer);

/**
 * @brief 重新设置超时时间（不改变回调）
 * @param timer 定时器指针
 * @param timeout_ms 新的超时时间（毫秒）
 * @return 错误码
 * @note 会取消当前定时器并重新启动
 */
CC_API ctimer_error_t CC_CALL ctimer_expires_from_now(ctimer_t* timer, uint32_t timeout_ms);

/**
 * @brief 获取剩余时间
 * @param timer 定时器指针
 * @param remaining_ms 输出剩余时间（毫秒）
 * @return 错误码
 */
CC_API ctimer_error_t CC_CALL ctimer_get_remaining_time(ctimer_t* timer, uint32_t* remaining_ms);

/* ========== 工具函数 ========== */

/**
 * @brief 获取错误描述字符串
 * @param error 错误码
 * @return 错误描述字符串
 */
CC_API const char* CC_CALL ctimer_strerror(ctimer_error_t error);

/**
 * @brief 获取当前时间戳（毫秒）
 * @return 当前时间戳（毫秒）
 */
CC_API uint64_t CC_CALL ctimer_now_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* LABEZ_CTIMER_H */


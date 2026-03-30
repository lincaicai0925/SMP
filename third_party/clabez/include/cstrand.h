/**
 * @file cstrand.h
 * @brief Strand（串行执行器），参考boost::asio::strand设计
 * @details 保证一组异步操作串行执行，即使在多线程环境下也不会并发
 * @author labez_core
 * @date 2025-11-22
 */

#ifndef LABEZ_CSTRAND_H
#define LABEZ_CSTRAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciosrv.h"
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
 * @brief Strand错误码
 */
typedef enum {
    CSTRAND_OK = 0,                     /**< 成功 */
    CSTRAND_ERROR_INVALID_PARAM,        /**< 无效参数 */
    CSTRAND_ERROR_MEMORY,               /**< 内存分配失败 */
    CSTRAND_ERROR_UNKNOWN               /**< 未知错误 */
} cstrand_error_t;

/**
 * @brief Strand句柄（不透明结构）
 */
typedef struct cstrand_t cstrand_t;

/* ========== 创建和销毁 ========== */

/**
 * @brief 创建Strand
 * @param io_service IO服务
 * @param strand 输出Strand指针
 * @return 错误码
 * @note Strand保证投递到它的所有任务串行执行，即使io_service在多线程中运行
 */
CC_API cstrand_error_t CC_CALL cstrand_create(ciosrv_t* io_service, cstrand_t** strand);

/**
 * @brief 销毁Strand
 * @param strand Strand指针的指针
 * @return 错误码
 * @note 会等待所有待执行的任务完成
 */
CC_API cstrand_error_t CC_CALL cstrand_destroy(cstrand_t** strand);

/* ========== 任务投递 ========== */

/**
 * @brief 在Strand中投递任务
 * @param strand Strand指针
 * @param handler 任务处理函数
 * @param user_data 用户数据
 * @return 错误码
 * @note 任务会按照投递顺序串行执行
 */
CC_API cstrand_error_t CC_CALL cstrand_post(cstrand_t* strand, 
                                              ciosrv_task_handler_t handler, 
                                              void* user_data);

/**
 * @brief 在Strand中派发任务
 * @param strand Strand指针
 * @param handler 任务处理函数
 * @param user_data 用户数据
 * @return 错误码
 * @note 如果当前线程正在执行Strand的任务，则直接执行；否则投递
 */
CC_API cstrand_error_t CC_CALL cstrand_dispatch(cstrand_t* strand,
                                                  ciosrv_task_handler_t handler,
                                                  void* user_data);

/**
 * @brief 检查当前线程是否在Strand中运行
 * @param strand Strand指针
 * @return 1表示在Strand中运行，0表示不在
 */
CC_API int CC_CALL cstrand_running_in_this_thread(cstrand_t* strand);

/**
 * @brief 包装handler，使其在strand中执行
 * @param strand Strand指针
 * @param handler 原始handler
 * @param user_data 用户数据
 * @param wrapped_handler 输出包装后的handler（需要调用者释放）
 * @param wrapped_data 输出包装后的user_data
 * @return 错误码
 * @note 这个函数用于将普通handler包装成在strand中执行的handler
 */
CC_API cstrand_error_t CC_CALL cstrand_wrap(cstrand_t* strand,
                                              ciosrv_task_handler_t handler,
                                              void* user_data,
                                              ciosrv_task_handler_t* wrapped_handler,
                                              void** wrapped_data);

/* ========== 工具函数 ========== */

/**
 * @brief 获取错误描述字符串
 * @param error 错误码
 * @return 错误描述字符串
 */
CC_API const char* CC_CALL cstrand_strerror(cstrand_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* LABEZ_CSTRAND_H */


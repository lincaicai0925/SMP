/**
 * @file ciosrv.h
 * @brief 跨平台IO服务库，参考boost::asio::io_service设计
 * @details 提供统一的异步IO事件循环，Windows使用IOCP，Linux使用epoll
 * @author labez_core
 * @date 2025-11-22
 */

#ifndef LABEZ_CIOSRV_H
#define LABEZ_CIOSRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* 平台相关定义 */
#ifdef _WIN32
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600  /* Windows Vista及以上 */
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <mswsock.h>
    #include <windows.h>
#else
    #include <sys/epoll.h>
    #include <sys/eventfd.h>
    #include <pthread.h>
#endif

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
 * @brief IO服务错误码
 */
typedef enum {
    CIOSRV_OK = 0,                      /**< 成功 */
    CIOSRV_ERROR_INVALID_PARAM,         /**< 无效参数 */
    CIOSRV_ERROR_CREATE,                /**< 创建失败 */
    CIOSRV_ERROR_ALREADY_RUNNING,       /**< 已经在运行 */
    CIOSRV_ERROR_NOT_RUNNING,           /**< 未运行 */
    CIOSRV_ERROR_MEMORY,                /**< 内存分配失败 */
    CIOSRV_ERROR_SYSTEM,                /**< 系统错误 */
    CIOSRV_ERROR_STOPPED,               /**< 服务已停止 */
    CIOSRV_ERROR_UNKNOWN                /**< 未知错误 */
} ciosrv_error_t;

/**
 * @brief IO操作类型
 */
typedef enum {
    CIOSRV_OP_ACCEPT = 0,    /**< accept操作 */
    CIOSRV_OP_CONNECT,       /**< connect操作 */
    CIOSRV_OP_READ,          /**< read操作 */
    CIOSRV_OP_WRITE,         /**< write操作 */
    CIOSRV_OP_CLOSE,         /**< close操作 */
    CIOSRV_OP_USER           /**< 用户自定义操作 */
} ciosrv_op_type_t;

/* 前向声明 */
typedef struct ciosrv_t ciosrv_t;
typedef struct ciosrv_op_t ciosrv_op_t;

/**
 * @brief IO操作完成回调函数
 * @param op 操作上下文
 * @param error 错误码（0表示成功，其他表示错误）
 * @param bytes_transferred 传输的字节数
 */
typedef void (*ciosrv_completion_handler_t)(ciosrv_op_t* op, int error, size_t bytes_transferred);

/**
 * @brief 用户任务回调函数
 * @param user_data 用户数据
 */
typedef void (*ciosrv_task_handler_t)(void* user_data);

/**
 * @brief IO操作上下文
 */
struct ciosrv_op_t {
#ifdef _WIN32
    OVERLAPPED overlapped;               /**< Windows OVERLAPPED结构 */
#endif
    ciosrv_t* io_service;                /**< 所属IO服务 */
    ciosrv_op_type_t op_type;            /**< 操作类型 */
    ciosrv_completion_handler_t handler; /**< 完成回调 */
    void* user_data;                     /**< 用户数据 */
    void* buffer;                        /**< 缓冲区 */
    size_t buffer_size;                  /**< 缓冲区大小 */
    int fd;                              /**< 文件描述符（Linux） */
    int internal_flags;                  /**< 内部标志位 */
    void* reserved[4];                   /**< 保留字段 */
};

/**
 * @brief IO服务句柄
 */
struct ciosrv_t {
#ifdef _WIN32
    HANDLE iocp;                         /**< IOCP句柄 */
    CRITICAL_SECTION lock;               /**< 临界区 */
#else
    int epoll_fd;                        /**< epoll文件描述符 */
    int event_fd;                        /**< eventfd用于唤醒 */
    pthread_mutex_t lock;                /**< 互斥锁 */
#endif
    volatile int stopped;                /**< 停止标志 */
    volatile int running;                /**< 运行标志 */
    size_t outstanding_work;             /**< 未完成的工作计数 */
    void* reserved[8];                   /**< 保留字段 */
};

/* ========== 初始化和清理 ========== */

/**
 * @brief 初始化IO服务库
 * @return 错误码
 * @note Windows平台会初始化Winsock
 */
CC_API ciosrv_error_t CC_CALL ciosrv_init(void);

/**
 * @brief 清理IO服务库
 * @note Windows平台会清理Winsock
 */
CC_API void CC_CALL ciosrv_cleanup(void);

/* ========== IO服务创建和销毁 ========== */

/**
 * @brief 创建IO服务
 * @param srv IO服务指针的指针
 * @param concurrency_hint 并发提示（线程数），0表示使用默认值
 * @return 错误码
 */
CC_API ciosrv_error_t CC_CALL ciosrv_create(ciosrv_t** srv, size_t concurrency_hint);

/**
 * @brief 销毁IO服务
 * @param srv IO服务指针
 * @return 错误码
 */
CC_API ciosrv_error_t CC_CALL ciosrv_destroy(ciosrv_t* srv);

/* ========== 运行控制 ========== */

/**
 * @brief 运行IO服务（阻塞直到停止）
 * @param srv IO服务指针
 * @return 执行的处理器数量，失败返回0
 * @note 这是主事件循环函数，会一直运行直到调用stop或所有工作完成
 */
CC_API size_t CC_CALL ciosrv_run(ciosrv_t* srv);

/**
 * @brief 运行IO服务一次（非阻塞）
 * @param srv IO服务指针
 * @return 执行的处理器数量
 * @note 处理一个就绪的事件后立即返回
 */
CC_API size_t CC_CALL ciosrv_run_one(ciosrv_t* srv);

/**
 * @brief 轮询IO服务（非阻塞）
 * @param srv IO服务指针
 * @return 执行的处理器数量
 * @note 处理所有就绪的事件后立即返回，不等待
 */
CC_API size_t CC_CALL ciosrv_poll(ciosrv_t* srv);

/**
 * @brief 轮询IO服务一次（非阻塞）
 * @param srv IO服务指针
 * @return 执行的处理器数量
 * @note 处理一个就绪的事件后立即返回，不等待
 */
CC_API size_t CC_CALL ciosrv_poll_one(ciosrv_t* srv);

/**
 * @brief 停止IO服务
 * @param srv IO服务指针
 * @return 错误码
 * @note 停止事件循环，所有run函数将返回
 */
CC_API ciosrv_error_t CC_CALL ciosrv_stop(ciosrv_t* srv);

/**
 * @brief 重置IO服务
 * @param srv IO服务指针
 * @return 错误码
 * @note 重置停止标志，使IO服务可以再次运行
 */
CC_API ciosrv_error_t CC_CALL ciosrv_reset(ciosrv_t* srv);

/**
 * @brief 检查IO服务是否已停止
 * @param srv IO服务指针
 * @return 1表示已停止，0表示未停止
 */
CC_API int CC_CALL ciosrv_stopped(ciosrv_t* srv);

/* ========== 任务投递 ========== */

/**
 * @brief 投递任务到IO服务
 * @param srv IO服务指针
 * @param handler 任务处理函数
 * @param user_data 用户数据
 * @return 错误码
 * @note 任务会在IO服务线程中异步执行
 */
CC_API ciosrv_error_t CC_CALL ciosrv_post(ciosrv_t* srv, ciosrv_task_handler_t handler, void* user_data);

/**
 * @brief 派发任务到IO服务
 * @param srv IO服务指针
 * @param handler 任务处理函数
 * @param user_data 用户数据
 * @return 错误码
 * @note 如果当前在IO服务线程中，直接执行；否则投递
 */
CC_API ciosrv_error_t CC_CALL ciosrv_dispatch(ciosrv_t* srv, ciosrv_task_handler_t handler, void* user_data);

/* ========== IO操作 ========== */

/**
 * @brief 创建IO操作上下文
 * @param op IO操作指针的指针
 * @return 错误码
 */
CC_API ciosrv_error_t CC_CALL ciosrv_op_create(ciosrv_op_t** op);

/**
 * @brief 销毁IO操作上下文
 * @param op IO操作指针
 */
CC_API void CC_CALL ciosrv_op_destroy(ciosrv_op_t* op);

/**
 * @brief 关联文件描述符到IO服务
 * @param srv IO服务指针
 * @param fd 文件描述符（socket）
 * @return 错误码
 * @note Windows会关联到IOCP，Linux会添加到epoll
 */
CC_API ciosrv_error_t CC_CALL ciosrv_register_fd(ciosrv_t* srv, int fd);

/**
 * @brief 取消关联文件描述符
 * @param srv IO服务指针
 * @param fd 文件描述符（socket）
 * @return 错误码
 */
CC_API ciosrv_error_t CC_CALL ciosrv_unregister_fd(ciosrv_t* srv, int fd);

/**
 * @brief 投递异步读操作
 * @param srv IO服务指针
 * @param op IO操作上下文
 * @param fd 文件描述符
 * @param buffer 接收缓冲区
 * @param buffer_size 缓冲区大小
 * @param handler 完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
CC_API ciosrv_error_t CC_CALL ciosrv_async_read(ciosrv_t* srv, ciosrv_op_t* op, int fd,
                                                  void* buffer, size_t buffer_size,
                                                  ciosrv_completion_handler_t handler,
                                                  void* user_data);

/**
 * @brief 投递异步写操作
 * @param srv IO服务指针
 * @param op IO操作上下文
 * @param fd 文件描述符
 * @param buffer 发送缓冲区
 * @param buffer_size 数据大小
 * @param handler 完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
CC_API ciosrv_error_t CC_CALL ciosrv_async_write(ciosrv_t* srv, ciosrv_op_t* op, int fd,
                                                   const void* buffer, size_t buffer_size,
                                                   ciosrv_completion_handler_t handler,
                                                   void* user_data);

/**
 * @brief 获取最后一个系统错误码
 * @return 系统错误码
 */
CC_API int CC_CALL ciosrv_get_last_error(void);

/**
 * @brief 获取错误描述字符串
 * @param error 错误码
 * @return 错误描述字符串
 */
CC_API const char* CC_CALL ciosrv_strerror(ciosrv_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* LABEZ_CIOSRV_H */


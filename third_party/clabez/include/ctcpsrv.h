/**
 * @file ctcpsrv.h
 * @brief 跨平台TCP服务器库，参考boost::asio::ip::tcp::acceptor设计
 * @details 提供完整的TCP服务器功能，支持异步/同步accept、连接管理
 * @author labez_core
 * @date 2025-11-22
 */

#ifndef LABEZ_CTCPSRV_H
#define LABEZ_CTCPSRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciosrv.h"
#include "ctcpcli.h"
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
    typedef SOCKET ctcpsrv_socket_t;
    #define CTCPSRV_INVALID_SOCKET INVALID_SOCKET
    #define CTCPSRV_SOCKET_ERROR SOCKET_ERROR
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <netdb.h>
    #include <pthread.h>
    typedef int ctcpsrv_socket_t;
    #define CTCPSRV_INVALID_SOCKET (-1)
    #define CTCPSRV_SOCKET_ERROR (-1)
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
 * @brief TCP服务器错误码
 */
typedef enum {
    CTCPSRV_OK = 0,                     /**< 成功 */
    CTCPSRV_ERROR_INVALID_PARAM,        /**< 无效参数 */
    CTCPSRV_ERROR_SOCKET_CREATE,        /**< socket创建失败 */
    CTCPSRV_ERROR_SOCKET_BIND,          /**< socket绑定失败 */
    CTCPSRV_ERROR_SOCKET_LISTEN,        /**< socket监听失败 */
    CTCPSRV_ERROR_ACCEPT,               /**< accept失败 */
    CTCPSRV_ERROR_TIMEOUT,              /**< 超时 */
    CTCPSRV_ERROR_WOULDBLOCK,           /**< 操作将阻塞 */
    CTCPSRV_ERROR_SETSOCKOPT,           /**< 设置socket选项失败 */
    CTCPSRV_ERROR_GETSOCKOPT,           /**< 获取socket选项失败 */
    CTCPSRV_ERROR_NOT_LISTENING,        /**< 未监听 */
    CTCPSRV_ERROR_ALREADY_LISTENING,    /**< 已经在监听 */
    CTCPSRV_ERROR_NOT_INITIALIZED,      /**< 未初始化 */
    CTCPSRV_ERROR_MEMORY,               /**< 内存分配失败 */
    CTCPSRV_ERROR_CLOSED,               /**< 连接已关闭 */
    CTCPSRV_ERROR_UNKNOWN               /**< 未知错误 */
} ctcpsrv_error_t;

/**
 * @brief TCP服务器句柄
 */
typedef struct {
    ctcpsrv_socket_t socket;            /**< 底层socket */
    ciosrv_t* io_service;               /**< IO服务 */
    ctcp_address_family_t family;       /**< 地址族 */
    int is_open;                        /**< 是否打开 */
    int is_listening;                   /**< 是否在监听 */
    ctcp_endpoint_t local_endpoint;     /**< 本地endpoint */
    int backlog;                        /**< listen backlog */
    
#ifdef _WIN32
    CRITICAL_SECTION lock;              /**< 临界区 */
    
    /* Windows异步accept需要的扩展函数 */
    LPFN_ACCEPTEX AcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs;
#else
    pthread_mutex_t lock;               /**< 互斥锁 */
#endif
    
    void* reserved[8];                  /**< 保留字段 */
} ctcpsrv_handle_t;

/**
 * @brief 异步accept回调函数类型
 * @param server 服务器句柄
 * @param client 接受的客户端连接
 * @param error 错误码
 * @param user_data 用户数据
 */
typedef void (*ctcpsrv_accept_handler_t)(ctcpsrv_handle_t* server, ctcp_handle_t* client, 
                                          ctcpsrv_error_t error, void* user_data);

/**
 * @brief 异步accept上下文
 */
typedef struct {
    ciosrv_op_t* op;                    /**< IO操作 */
    ctcpsrv_handle_t* server;           /**< 服务器句柄 */
    ctcp_handle_t* client;              /**< 接受的客户端 */
    ctcpsrv_accept_handler_t handler;   /**< 回调函数 */
    void* user_data;                    /**< 用户数据 */
#ifdef _WIN32
    /* AcceptEx 需要的地址缓冲区：
     * 本地地址：sizeof(sockaddr_storage) + 16 = 128 + 16 = 144 字节
     * 远程地址：sizeof(sockaddr_storage) + 16 = 128 + 16 = 144 字节
     * 总共：288 字节 + 额外空间以防万一 = 512 字节
     */
    char addr_buffer[512];              /**< Windows AcceptEx需要的地址缓冲 */
    DWORD bytes_received;               /**< 接收的字节数 */
#endif
} ctcpsrv_accept_context_t;

/* ========== 初始化和清理 ========== */

/**
 * @brief 初始化TCP服务器库
 * @return 错误码
 * @note Windows平台会初始化Winsock，必须在使用其他函数前调用
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_init(void);

/**
 * @brief 清理TCP服务器库
 * @note Windows平台会清理Winsock，程序结束前调用
 */
CC_API void CC_CALL ctcpsrv_cleanup(void);

/* ========== 服务器创建和销毁 ========== */

/**
 * @brief 创建TCP服务器socket
 * @param server 服务器句柄指针的指针
 * @param io_service IO服务（可选，传NULL表示不使用异步IO）
 * @param family 地址族（IPv4或IPv6）
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_create(ctcpsrv_handle_t** server, ciosrv_t* io_service, 
                                                ctcp_address_family_t family);

/**
 * @brief 打开TCP服务器socket
 * @param server 服务器句柄
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_open(ctcpsrv_handle_t* server);

/**
 * @brief 关闭TCP服务器socket
 * @param server 服务器句柄
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_close(ctcpsrv_handle_t* server);

/**
 * @brief 销毁TCP服务器socket
 * @param server 服务器句柄指针的指针
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_destroy(ctcpsrv_handle_t** server);

/* ========== 绑定和监听 ========== */

/**
 * @brief 绑定到本地地址和端口
 * @param server 服务器句柄
 * @param host 主机地址（IP地址或主机名），NULL表示INADDR_ANY
 * @param port 端口号
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_bind(ctcpsrv_handle_t* server, const char* host, uint16_t port);

/**
 * @brief 开始监听连接
 * @param server 服务器句柄
 * @param backlog 监听队列长度，0表示使用默认值
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_listen(ctcpsrv_handle_t* server, int backlog);

/* ========== 接受连接 ========== */

/**
 * @brief 同步接受连接
 * @param server 服务器句柄
 * @param client 接受的客户端连接（输出参数）
 * @param timeout_ms 超时时间（毫秒），0表示阻塞
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_accept(ctcpsrv_handle_t* server, ctcp_handle_t** client, int timeout_ms);

/**
 * @brief 异步接受连接
 * @param server 服务器句柄
 * @param client 预分配的客户端连接对象
 * @param handler 接受完成回调
 * @param user_data 用户数据
 * @return 错误码
 * @note client必须是预先创建的ctcp_handle_t对象
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_async_accept(ctcpsrv_handle_t* server, ctcp_handle_t* client,
                                                      ctcpsrv_accept_handler_t handler, void* user_data);

/* ========== socket选项 ========== */

/**
 * @brief 设置socket选项
 * @param server 服务器句柄
 * @param level 选项级别
 * @param option 选项名称
 * @param value 选项值指针
 * @param value_len 选项值长度
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_set_option(ctcpsrv_handle_t* server, ctcp_socket_level_t level,
                                                    ctcp_socket_option_t option, const void* value, socklen_t value_len);

/**
 * @brief 获取socket选项
 * @param server 服务器句柄
 * @param level 选项级别
 * @param option 选项名称
 * @param value 选项值指针
 * @param value_len 选项值长度指针（输入输出参数）
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_get_option(ctcpsrv_handle_t* server, ctcp_socket_level_t level,
                                                    ctcp_socket_option_t option, void* value, socklen_t* value_len);

/**
 * @brief 设置地址重用
 * @param server 服务器句柄
 * @param reuse 1表示启用，0表示禁用
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_set_reuseaddr(ctcpsrv_handle_t* server, int reuse);

/**
 * @brief 设置非阻塞模式
 * @param server 服务器句柄
 * @param nonblocking 1表示非阻塞，0表示阻塞
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_set_nonblocking(ctcpsrv_handle_t* server, int nonblocking);

/* ========== 状态查询 ========== */

/**
 * @brief 检查是否在监听
 * @param server 服务器句柄
 * @return 1表示在监听，0表示未监听
 */
CC_API int CC_CALL ctcpsrv_is_listening(ctcpsrv_handle_t* server);

/**
 * @brief 检查是否已打开
 * @param server 服务器句柄
 * @return 1表示已打开，0表示未打开
 */
CC_API int CC_CALL ctcpsrv_is_open(ctcpsrv_handle_t* server);

/**
 * @brief 获取本地endpoint
 * @param server 服务器句柄
 * @param endpoint 输出endpoint
 * @return 错误码
 */
CC_API ctcpsrv_error_t CC_CALL ctcpsrv_local_endpoint(ctcpsrv_handle_t* server, ctcp_endpoint_t* endpoint);

/* ========== 错误处理 ========== */

/**
 * @brief 获取最后一个系统错误码
 * @return 系统错误码
 */
CC_API int CC_CALL ctcpsrv_get_last_error(void);

/**
 * @brief 获取错误描述字符串
 * @param error 错误码
 * @return 错误描述字符串
 */
CC_API const char* CC_CALL ctcpsrv_strerror(ctcpsrv_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* LABEZ_CTCPSRV_H */


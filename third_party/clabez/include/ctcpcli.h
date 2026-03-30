/**
 * @file ctcpcli.h
 * @brief 跨平台TCP客户端库，参考boost::asio::ip::tcp设计
 * @details 提供完整的TCP客户端功能，支持异步/同步连接、读写操作
 * @author labez_core
 * @date 2025-11-22
 */

#ifndef LABEZ_CTCPCLI_H
#define LABEZ_CTCPCLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciosrv.h"
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
    typedef SOCKET ctcp_socket_t;
    #define CTCP_INVALID_SOCKET INVALID_SOCKET
    #define CTCP_SOCKET_ERROR SOCKET_ERROR
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
    typedef int ctcp_socket_t;
    #define CTCP_INVALID_SOCKET (-1)
    #define CTCP_SOCKET_ERROR (-1)
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
 * @brief TCP错误码
 */
typedef enum {
    CTCP_OK = 0,                        /**< 成功 */
    CTCP_ERROR_INVALID_PARAM,           /**< 无效参数 */
    CTCP_ERROR_SOCKET_CREATE,           /**< socket创建失败 */
    CTCP_ERROR_SOCKET_BIND,             /**< socket绑定失败 */
    CTCP_ERROR_SOCKET_CONNECT,          /**< socket连接失败 */
    CTCP_ERROR_SEND,                    /**< 发送失败 */
    CTCP_ERROR_RECV,                    /**< 接收失败 */
    CTCP_ERROR_TIMEOUT,                 /**< 超时 */
    CTCP_ERROR_WOULDBLOCK,              /**< 操作将阻塞 */
    CTCP_ERROR_ADDR_RESOLVE,            /**< 地址解析失败 */
    CTCP_ERROR_SETSOCKOPT,              /**< 设置socket选项失败 */
    CTCP_ERROR_GETSOCKOPT,              /**< 获取socket选项失败 */
    CTCP_ERROR_NOT_CONNECTED,           /**< 未连接 */
    CTCP_ERROR_ALREADY_CONNECTED,       /**< 已经连接 */
    CTCP_ERROR_CONNECTION_REFUSED,      /**< 连接被拒绝 */
    CTCP_ERROR_CONNECTION_RESET,        /**< 连接被重置 */
    CTCP_ERROR_NETWORK_UNREACHABLE,     /**< 网络不可达 */
    CTCP_ERROR_HOST_UNREACHABLE,        /**< 主机不可达 */
    CTCP_ERROR_NOT_INITIALIZED,         /**< 未初始化 */
    CTCP_ERROR_ALREADY_INITIALIZED,     /**< 已经初始化 */
    CTCP_ERROR_MEMORY,                  /**< 内存分配失败 */
    CTCP_ERROR_CLOSED,                  /**< 连接已关闭 */
    CTCP_ERROR_UNKNOWN                  /**< 未知错误 */
} ctcp_error_t;

/**
 * @brief IP地址族
 */
typedef enum {
    CTCP_AF_INET = AF_INET,             /**< IPv4 */
    CTCP_AF_INET6 = AF_INET6            /**< IPv6 */
} ctcp_address_family_t;

/**
 * @brief socket选项级别
 */
typedef enum {
    CTCP_SOL_SOCKET = SOL_SOCKET,       /**< socket级别 */
    CTCP_IPPROTO_TCP = IPPROTO_TCP      /**< TCP协议级别 */
} ctcp_socket_level_t;

/**
 * @brief socket选项名称
 */
typedef enum {
    CTCP_SO_REUSEADDR = SO_REUSEADDR,   /**< 地址重用 */
    CTCP_SO_KEEPALIVE = SO_KEEPALIVE,   /**< 保活 */
    CTCP_SO_RCVBUF = SO_RCVBUF,         /**< 接收缓冲区大小 */
    CTCP_SO_SNDBUF = SO_SNDBUF,         /**< 发送缓冲区大小 */
    CTCP_SO_RCVTIMEO = SO_RCVTIMEO,     /**< 接收超时 */
    CTCP_SO_SNDTIMEO = SO_SNDTIMEO,     /**< 发送超时 */
    CTCP_TCP_NODELAY = TCP_NODELAY      /**< 禁用Nagle算法 */
} ctcp_socket_option_t;

/**
 * @brief shutdown方式
 */
typedef enum {
    CTCP_SHUT_RD = 0,                   /**< 关闭读 */
    CTCP_SHUT_WR = 1,                   /**< 关闭写 */
    CTCP_SHUT_RDWR = 2                  /**< 关闭读写 */
} ctcp_shutdown_type_t;

/**
 * @brief IP地址结构
 */
typedef struct {
    ctcp_address_family_t family;       /**< 地址族 */
    union {
        struct sockaddr_in ipv4;        /**< IPv4地址 */
        struct sockaddr_in6 ipv6;       /**< IPv6地址 */
    } addr;
    socklen_t addr_len;                 /**< 地址长度 */
} ctcp_address_t;

/**
 * @brief TCP endpoint（地址+端口）
 */
typedef struct {
    ctcp_address_t address;             /**< 地址 */
    uint16_t port;                      /**< 端口号 */
} ctcp_endpoint_t;

/**
 * @brief TCP客户端句柄
 */
typedef struct {
    ctcp_socket_t socket;               /**< 底层socket */
    ciosrv_t* io_service;               /**< IO服务 */
    ctcp_address_family_t family;       /**< 地址族 */
    int is_open;                        /**< 是否打开 */
    int is_connected;                   /**< 是否已连接 */
    int is_nonblocking;                 /**< 是否非阻塞 */
    ctcp_endpoint_t local_endpoint;     /**< 本地endpoint */
    ctcp_endpoint_t remote_endpoint;    /**< 远程endpoint */
    
#ifdef _WIN32
    CRITICAL_SECTION lock;              /**< 临界区 */
    
    /* Windows异步连接需要的扩展函数 */
    LPFN_CONNECTEX ConnectEx;
    LPFN_DISCONNECTEX DisconnectEx;
#else
    pthread_mutex_t lock;               /**< 互斥锁 */
#endif
    
    void* reserved[8];                  /**< 保留字段 */
} ctcp_handle_t;

/**
 * @brief 异步连接回调函数类型
 * @param handle TCP句柄
 * @param error 错误码
 * @param user_data 用户数据
 */
typedef void (*ctcp_connect_handler_t)(ctcp_handle_t* handle, ctcp_error_t error, void* user_data);

/**
 * @brief 异步读回调函数类型
 * @param handle TCP句柄
 * @param error 错误码
 * @param bytes_transferred 传输的字节数
 * @param user_data 用户数据
 */
typedef void (*ctcp_read_handler_t)(ctcp_handle_t* handle, ctcp_error_t error, size_t bytes_transferred, void* user_data);

/**
 * @brief 异步写回调函数类型
 * @param handle TCP句柄
 * @param error 错误码
 * @param bytes_transferred 传输的字节数
 * @param user_data 用户数据
 */
typedef void (*ctcp_write_handler_t)(ctcp_handle_t* handle, ctcp_error_t error, size_t bytes_transferred, void* user_data);

/**
 * @brief 异步连接上下文
 */
typedef struct {
    ciosrv_op_t* op;                    /**< IO操作 */
    ctcp_handle_t* handle;              /**< TCP句柄 */
    ctcp_endpoint_t endpoint;           /**< 目标endpoint */
    ctcp_connect_handler_t handler;     /**< 回调函数 */
    void* user_data;                    /**< 用户数据 */
#ifdef _WIN32
    char addr_buffer[64];               /**< Windows ConnectEx需要的地址缓冲 */
#endif
} ctcp_connect_context_t;

/**
 * @brief 异步读上下文
 */
typedef struct {
    ciosrv_op_t* op;                    /**< IO操作 */
    ctcp_handle_t* handle;              /**< TCP句柄 */
    void* buffer;                       /**< 缓冲区 */
    size_t buffer_size;                 /**< 缓冲区大小 */
    ctcp_read_handler_t handler;        /**< 回调函数 */
    void* user_data;                    /**< 用户数据 */
} ctcp_read_context_t;

/**
 * @brief 异步写上下文
 */
typedef struct {
    ciosrv_op_t* op;                    /**< IO操作 */
    ctcp_handle_t* handle;              /**< TCP句柄 */
    const void* buffer;                 /**< 缓冲区 */
    size_t buffer_size;                 /**< 数据大小 */
    ctcp_write_handler_t handler;       /**< 回调函数 */
    void* user_data;                    /**< 用户数据 */
} ctcp_write_context_t;

/* ========== 初始化和清理 ========== */

/**
 * @brief 初始化TCP客户端库
 * @return 错误码
 * @note Windows平台会初始化Winsock，必须在使用其他函数前调用
 */
CC_API ctcp_error_t CC_CALL ctcp_init(void);

/**
 * @brief 清理TCP客户端库
 * @note Windows平台会清理Winsock，程序结束前调用
 */
CC_API void CC_CALL ctcp_cleanup(void);

/* ========== socket创建和销毁 ========== */

/**
 * @brief 创建TCP客户端socket
 * @param handle TCP句柄指针的指针
 * @param io_service IO服务（可选，传NULL表示不使用异步IO）
 * @param family 地址族（IPv4或IPv6）
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_create(ctcp_handle_t** handle, ciosrv_t* io_service, ctcp_address_family_t family);

/**
 * @brief 打开TCP socket
 * @param handle TCP句柄
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_open(ctcp_handle_t* handle);

/**
 * @brief 关闭TCP socket
 * @param handle TCP句柄
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_close(ctcp_handle_t* handle);

/**
 * @brief 销毁TCP socket
 * @param handle TCP句柄指针的指针
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_destroy(ctcp_handle_t** handle);

/* ========== 连接操作 ========== */

/**
 * @brief 同步连接到服务器
 * @param handle TCP句柄
 * @param host 主机名或IP地址
 * @param port 端口号
 * @param timeout_ms 超时时间（毫秒），0表示阻塞
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_connect(ctcp_handle_t* handle, const char* host, uint16_t port, int timeout_ms);

/**
 * @brief 异步连接到服务器
 * @param handle TCP句柄
 * @param host 主机名或IP地址
 * @param port 端口号
 * @param handler 连接完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_async_connect(ctcp_handle_t* handle, const char* host, uint16_t port,
                                                 ctcp_connect_handler_t handler, void* user_data);

/**
 * @brief 断开连接
 * @param handle TCP句柄
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_disconnect(ctcp_handle_t* handle);

/**
 * @brief shutdown连接
 * @param handle TCP句柄
 * @param how shutdown方式
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_shutdown(ctcp_handle_t* handle, ctcp_shutdown_type_t how);

/* ========== 数据收发 ========== */

/**
 * @brief 同步发送数据
 * @param handle TCP句柄
 * @param buffer 发送缓冲区
 * @param size 数据大小
 * @param bytes_sent 实际发送的字节数（输出参数）
 * @param timeout_ms 超时时间（毫秒），0表示阻塞
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_send(ctcp_handle_t* handle, const void* buffer, size_t size,
                                       size_t* bytes_sent, int timeout_ms);

/**
 * @brief 同步接收数据
 * @param handle TCP句柄
 * @param buffer 接收缓冲区
 * @param size 缓冲区大小
 * @param bytes_received 实际接收的字节数（输出参数）
 * @param timeout_ms 超时时间（毫秒），0表示阻塞
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_recv(ctcp_handle_t* handle, void* buffer, size_t size,
                                       size_t* bytes_received, int timeout_ms);

/**
 * @brief 异步发送数据
 * @param handle TCP句柄
 * @param buffer 发送缓冲区
 * @param size 数据大小
 * @param handler 发送完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_async_send(ctcp_handle_t* handle, const void* buffer, size_t size,
                                              ctcp_write_handler_t handler, void* user_data);

/**
 * @brief 异步接收数据
 * @param handle TCP句柄
 * @param buffer 接收缓冲区
 * @param size 缓冲区大小
 * @param handler 接收完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_async_recv(ctcp_handle_t* handle, void* buffer, size_t size,
                                              ctcp_read_handler_t handler, void* user_data);

/**
 * @brief 同步发送全部数据（直到全部发送完成或出错）
 * @param handle TCP句柄
 * @param buffer 发送缓冲区
 * @param size 数据大小
 * @param bytes_sent 实际发送的字节数（输出参数）
 * @param timeout_ms 超时时间（毫秒），0表示阻塞
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_send_all(ctcp_handle_t* handle, const void* buffer, size_t size,
                                            size_t* bytes_sent, int timeout_ms);

/**
 * @brief 同步接收指定数量数据（直到接收完成或出错）
 * @param handle TCP句柄
 * @param buffer 接收缓冲区
 * @param size 要接收的数据大小
 * @param bytes_received 实际接收的字节数（输出参数）
 * @param timeout_ms 超时时间（毫秒），0表示阻塞
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_recv_all(ctcp_handle_t* handle, void* buffer, size_t size,
                                            size_t* bytes_received, int timeout_ms);

/* ========== socket选项 ========== */

/**
 * @brief 设置socket选项
 * @param handle TCP句柄
 * @param level 选项级别
 * @param option 选项名称
 * @param value 选项值指针
 * @param value_len 选项值长度
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_set_option(ctcp_handle_t* handle, ctcp_socket_level_t level,
                                              ctcp_socket_option_t option, const void* value, socklen_t value_len);

/**
 * @brief 获取socket选项
 * @param handle TCP句柄
 * @param level 选项级别
 * @param option 选项名称
 * @param value 选项值指针
 * @param value_len 选项值长度指针（输入输出参数）
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_get_option(ctcp_handle_t* handle, ctcp_socket_level_t level,
                                              ctcp_socket_option_t option, void* value, socklen_t* value_len);

/**
 * @brief 设置非阻塞模式
 * @param handle TCP句柄
 * @param nonblocking 1表示非阻塞，0表示阻塞
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_set_nonblocking(ctcp_handle_t* handle, int nonblocking);

/**
 * @brief 设置TCP_NODELAY（禁用Nagle算法）
 * @param handle TCP句柄
 * @param nodelay 1表示禁用，0表示启用
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_set_nodelay(ctcp_handle_t* handle, int nodelay);

/**
 * @brief 设置SO_KEEPALIVE（保活）
 * @param handle TCP句柄
 * @param keepalive 1表示启用，0表示禁用
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_set_keepalive(ctcp_handle_t* handle, int keepalive);

/* ========== 地址解析 ========== */

/**
 * @brief 解析主机名到IP地址
 * @param host 主机名
 * @param port 端口号
 * @param family 地址族
 * @param endpoint 输出endpoint
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_resolve(const char* host, uint16_t port,
                                           ctcp_address_family_t family, ctcp_endpoint_t* endpoint);

/**
 * @brief 将endpoint转换为字符串
 * @param endpoint endpoint指针
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_endpoint_to_string(const ctcp_endpoint_t* endpoint, char* buffer, size_t buffer_size);

/* ========== 状态查询 ========== */

/**
 * @brief 检查是否已连接
 * @param handle TCP句柄
 * @return 1表示已连接，0表示未连接
 */
CC_API int CC_CALL ctcp_is_connected(ctcp_handle_t* handle);

/**
 * @brief 检查是否已打开
 * @param handle TCP句柄
 * @return 1表示已打开，0表示未打开
 */
CC_API int CC_CALL ctcp_is_open(ctcp_handle_t* handle);

/**
 * @brief 获取本地endpoint
 * @param handle TCP句柄
 * @param endpoint 输出endpoint
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_local_endpoint(ctcp_handle_t* handle, ctcp_endpoint_t* endpoint);

/**
 * @brief 获取远程endpoint
 * @param handle TCP句柄
 * @param endpoint 输出endpoint
 * @return 错误码
 */
CC_API ctcp_error_t CC_CALL ctcp_remote_endpoint(ctcp_handle_t* handle, ctcp_endpoint_t* endpoint);

/* ========== 错误处理 ========== */

/**
 * @brief 获取最后一个系统错误码
 * @return 系统错误码
 */
CC_API int CC_CALL ctcp_get_last_error(void);

/**
 * @brief 获取错误描述字符串
 * @param error 错误码
 * @return 错误描述字符串
 */
CC_API const char* CC_CALL ctcp_strerror(ctcp_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* LABEZ_CTCPCLI_H */


/**
 * @file cudp.h
 * @brief 跨平台UDP库，参考boost::asio设计
 * @details 提供完整的UDP socket功能，包括同步/异步IO、组播支持等
 * @author labez_core
 * @date 2025-11-20
 */

#ifndef LABEZ_CUDP_H
#define LABEZ_CUDP_H

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
    typedef SOCKET cudp_socket_t;
    typedef CRITICAL_SECTION cudp_mutex_t;
    #define CUDP_INVALID_SOCKET INVALID_SOCKET
    #define CUDP_SOCKET_ERROR SOCKET_ERROR
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <netdb.h>
    #include <pthread.h>
    typedef int cudp_socket_t;
    typedef pthread_mutex_t cudp_mutex_t;
    #define CUDP_INVALID_SOCKET (-1)
    #define CUDP_SOCKET_ERROR (-1)
#endif

/**
 * @brief UDP错误码
 */
typedef enum {
    CUDP_OK = 0,                    /**< 成功 */
    CUDP_ERROR_INVALID_PARAM,       /**< 无效参数 */
    CUDP_ERROR_SOCKET_CREATE,       /**< socket创建失败 */
    CUDP_ERROR_SOCKET_BIND,         /**< socket绑定失败 */
    CUDP_ERROR_SOCKET_CONNECT,      /**< socket连接失败 */
    CUDP_ERROR_SEND,                /**< 发送失败 */
    CUDP_ERROR_RECV,                /**< 接收失败 */
    CUDP_ERROR_TIMEOUT,             /**< 超时 */
    CUDP_ERROR_WOULDBLOCK,          /**< 操作将阻塞 */
    CUDP_ERROR_ADDR_RESOLVE,        /**< 地址解析失败 */
    CUDP_ERROR_SETSOCKOPT,          /**< 设置socket选项失败 */
    CUDP_ERROR_GETSOCKOPT,          /**< 获取socket选项失败 */
    CUDP_ERROR_MULTICAST_JOIN,      /**< 加入组播组失败 */
    CUDP_ERROR_MULTICAST_LEAVE,     /**< 离开组播组失败 */
    CUDP_ERROR_NOT_INITIALIZED,     /**< 未初始化 */
    CUDP_ERROR_ALREADY_INITIALIZED, /**< 已经初始化 */
    CUDP_ERROR_MEMORY,              /**< 内存分配失败 */
    CUDP_ERROR_UNKNOWN              /**< 未知错误 */
} cudp_error_t;

/**
 * @brief IP地址族
 */
typedef enum {
    CUDP_AF_INET = AF_INET,    /**< IPv4 */
    CUDP_AF_INET6 = AF_INET6   /**< IPv6 */
} cudp_address_family_t;

/**
 * @brief socket选项级别
 */
typedef enum {
    CUDP_SOL_SOCKET = SOL_SOCKET,      /**< socket级别 */
    CUDP_IPPROTO_IP = IPPROTO_IP,      /**< IP协议级别 */
    CUDP_IPPROTO_IPV6 = IPPROTO_IPV6,  /**< IPv6协议级别 */
    CUDP_IPPROTO_UDP = IPPROTO_UDP     /**< UDP协议级别 */
} cudp_socket_level_t;

/**
 * @brief socket选项名称
 */
typedef enum {
    CUDP_SO_REUSEADDR = SO_REUSEADDR,      /**< 地址重用 */
    CUDP_SO_BROADCAST = SO_BROADCAST,       /**< 广播 */
    CUDP_SO_RCVBUF = SO_RCVBUF,            /**< 接收缓冲区大小 */
    CUDP_SO_SNDBUF = SO_SNDBUF,            /**< 发送缓冲区大小 */
    CUDP_SO_RCVTIMEO = SO_RCVTIMEO,        /**< 接收超时 */
    CUDP_SO_SNDTIMEO = SO_SNDTIMEO,        /**< 发送超时 */
    CUDP_IP_MULTICAST_LOOP = IP_MULTICAST_LOOP,  /**< 组播回环 */
    CUDP_IP_MULTICAST_TTL = IP_MULTICAST_TTL,    /**< 组播TTL */
    CUDP_IP_TTL = IP_TTL                   /**< TTL */
} cudp_socket_option_t;

/**
 * @brief IP地址结构
 */
typedef struct {
    cudp_address_family_t family;  /**< 地址族 */
    union {
        struct sockaddr_in ipv4;   /**< IPv4地址 */
        struct sockaddr_in6 ipv6;  /**< IPv6地址 */
    } addr;
    socklen_t addr_len;            /**< 地址长度 */
} cudp_address_t;

/**
 * @brief UDP endpoint（地址+端口）
 */
typedef struct {
    cudp_address_t address;  /**< 地址 */
    uint16_t port;           /**< 端口号 */
} cudp_endpoint_t;

/**
 * @brief UDP socket句柄
 */
typedef struct {
    cudp_socket_t socket;           /**< 底层socket */
    ciosrv_t* io_service;           /**< IO服务（异步IO） */
    cudp_address_family_t family;   /**< 地址族 */
    int is_open;                    /**< 是否打开 */
    int is_bound;                   /**< 是否已绑定 */
    int is_connected;               /**< 是否已连接 */
    int is_nonblocking;             /**< 是否非阻塞 */
    cudp_endpoint_t local_endpoint; /**< 本地endpoint */
    cudp_endpoint_t remote_endpoint;/**< 远程endpoint */
    cudp_mutex_t mutex;             /**< 互斥锁，保护并发访问 */
} cudp_handle_t;

/**
 * @brief 异步操作回调函数类型
 * @param error 错误码
 * @param bytes_transferred 传输的字节数
 * @param user_data 用户数据
 */
typedef void (*cudp_async_callback_t)(cudp_error_t error, size_t bytes_transferred, void* user_data);

/**
 * @brief 异步操作上下文
 */
typedef struct {
    cudp_handle_t* handle;           /**< UDP句柄 */
    void* buffer;                    /**< 缓冲区 */
    size_t buffer_size;              /**< 缓冲区大小 */
    cudp_endpoint_t* endpoint;       /**< endpoint */
    cudp_async_callback_t callback;  /**< 回调函数 */
    void* user_data;                 /**< 用户数据 */
} cudp_async_context_t;

/* ========== 初始化和清理 ========== */

/**
 * @brief 初始化UDP库
 * @return 错误码
 * @note Windows平台会初始化Winsock，必须在使用其他函数前调用
 */
cudp_error_t cudp_init(void);

/**
 * @brief 清理UDP库
 * @note Windows平台会清理Winsock，程序结束前调用
 */
void cudp_cleanup(void);

/* ========== socket创建和销毁 ========== */

/**
 * @brief 创建UDP socket
 * @param handle UDP句柄指针
 * @param io_service IO服务（可选，传NULL表示不使用异步IO）
 * @param family 地址族（IPv4或IPv6）
 * @return 错误码
 */
cudp_error_t cudp_create(cudp_handle_t** handle, ciosrv_t* io_service, cudp_address_family_t family);

/**
 * @brief 打开UDP socket
 * @param handle UDP句柄
 * @return 错误码
 */
cudp_error_t cudp_open(cudp_handle_t* handle);

/**
 * @brief 关闭UDP socket
 * @param handle UDP句柄
 * @return 错误码
 */
cudp_error_t cudp_close(cudp_handle_t* handle);

/**
 * @brief 销毁UDP socket
 * @param handle UDP句柄指针
 */
void cudp_destroy(cudp_handle_t** handle);

/* ========== 地址和endpoint操作 ========== */

/**
 * @brief 创建endpoint
 * @param endpoint endpoint指针
 * @param ip_str IP地址字符串
 * @param port 端口号
 * @param family 地址族
 * @return 错误码
 */
cudp_error_t cudp_endpoint_create(cudp_endpoint_t* endpoint, const char* ip_str, uint16_t port, cudp_address_family_t family);

/**
 * @brief 将endpoint转换为字符串
 * @param endpoint endpoint
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 错误码
 */
cudp_error_t cudp_endpoint_to_string(const cudp_endpoint_t* endpoint, char* buffer, size_t buffer_size);

/**
 * @brief 解析主机名和服务名
 * @param hostname 主机名或IP地址
 * @param service 服务名或端口号
 * @param family 地址族
 * @param endpoints endpoint数组
 * @param max_endpoints 最大endpoint数量
 * @param num_endpoints 实际解析到的endpoint数量
 * @return 错误码
 */
cudp_error_t cudp_resolve(const char* hostname, const char* service, cudp_address_family_t family,
                          cudp_endpoint_t* endpoints, size_t max_endpoints, size_t* num_endpoints);

/* ========== socket绑定和连接 ========== */

/**
 * @brief 绑定socket到本地endpoint
 * @param handle UDP句柄
 * @param endpoint 本地endpoint
 * @return 错误码
 */
cudp_error_t cudp_bind(cudp_handle_t* handle, const cudp_endpoint_t* endpoint);

/**
 * @brief 连接到远程endpoint
 * @param handle UDP句柄
 * @param endpoint 远程endpoint
 * @return 错误码
 * @note 连接后可以使用cudp_send/cudp_recv而不需要指定地址
 */
cudp_error_t cudp_connect(cudp_handle_t* handle, const cudp_endpoint_t* endpoint);

/* ========== 同步IO操作 ========== */

/**
 * @brief 发送数据到指定endpoint
 * @param handle UDP句柄
 * @param buffer 发送缓冲区
 * @param size 数据大小
 * @param endpoint 目标endpoint
 * @param bytes_sent 实际发送的字节数
 * @return 错误码
 */
cudp_error_t cudp_send_to(cudp_handle_t* handle, const void* buffer, size_t size,
                          const cudp_endpoint_t* endpoint, size_t* bytes_sent);

/**
 * @brief 从指定endpoint接收数据
 * @param handle UDP句柄
 * @param buffer 接收缓冲区
 * @param size 缓冲区大小
 * @param endpoint 源endpoint（输出参数）
 * @param bytes_received 实际接收的字节数
 * @return 错误码
 */
cudp_error_t cudp_recv_from(cudp_handle_t* handle, void* buffer, size_t size,
                            cudp_endpoint_t* endpoint, size_t* bytes_received);

/**
 * @brief 发送数据（需要先connect）
 * @param handle UDP句柄
 * @param buffer 发送缓冲区
 * @param size 数据大小
 * @param bytes_sent 实际发送的字节数
 * @return 错误码
 */
cudp_error_t cudp_send(cudp_handle_t* handle, const void* buffer, size_t size, size_t* bytes_sent);

/**
 * @brief 接收数据（需要先connect）
 * @param handle UDP句柄
 * @param buffer 接收缓冲区
 * @param size 缓冲区大小
 * @param bytes_received 实际接收的字节数
 * @return 错误码
 */
cudp_error_t cudp_recv(cudp_handle_t* handle, void* buffer, size_t size, size_t* bytes_received);

/* ========== 异步IO操作 ========== */

/**
 * @brief 异步发送数据到指定endpoint
 * @param handle UDP句柄
 * @param buffer 发送缓冲区
 * @param size 数据大小
 * @param endpoint 目标endpoint
 * @param callback 完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
cudp_error_t cudp_async_send_to(cudp_handle_t* handle, const void* buffer, size_t size,
                                const cudp_endpoint_t* endpoint, cudp_async_callback_t callback, void* user_data);

/**
 * @brief 异步从指定endpoint接收数据
 * @param handle UDP句柄
 * @param buffer 接收缓冲区
 * @param size 缓冲区大小
 * @param endpoint 源endpoint（输出参数）
 * @param callback 完成回调
 * @param user_data 用户数据
 * @return 错误码
 */
cudp_error_t cudp_async_recv_from(cudp_handle_t* handle, void* buffer, size_t size,
                                  cudp_endpoint_t* endpoint, cudp_async_callback_t callback, void* user_data);

/* ========== socket选项 ========== */

/**
 * @brief 设置socket选项
 * @param handle UDP句柄
 * @param level 选项级别
 * @param option 选项名称
 * @param value 选项值
 * @param value_len 值长度
 * @return 错误码
 */
cudp_error_t cudp_set_option(cudp_handle_t* handle, cudp_socket_level_t level,
                             cudp_socket_option_t option, const void* value, socklen_t value_len);

/**
 * @brief 获取socket选项
 * @param handle UDP句柄
 * @param level 选项级别
 * @param option 选项名称
 * @param value 选项值（输出参数）
 * @param value_len 值长度（输入输出参数）
 * @return 错误码
 */
cudp_error_t cudp_get_option(cudp_handle_t* handle, cudp_socket_level_t level,
                             cudp_socket_option_t option, void* value, socklen_t* value_len);

/**
 * @brief 设置非阻塞模式
 * @param handle UDP句柄
 * @param nonblocking 是否非阻塞
 * @return 错误码
 */
cudp_error_t cudp_set_nonblocking(cudp_handle_t* handle, int nonblocking);

/**
 * @brief 设置广播模式
 * @param handle UDP句柄
 * @param broadcast 是否允许广播
 * @return 错误码
 */
cudp_error_t cudp_set_broadcast(cudp_handle_t* handle, int broadcast);

/**
 * @brief 设置地址重用
 * @param handle UDP句柄
 * @param reuse 是否重用地址
 * @return 错误码
 */
cudp_error_t cudp_set_reuse_address(cudp_handle_t* handle, int reuse);

/**
 * @brief 设置接收缓冲区大小
 * @param handle UDP句柄
 * @param size 缓冲区大小
 * @return 错误码
 */
cudp_error_t cudp_set_recv_buffer_size(cudp_handle_t* handle, int size);

/**
 * @brief 设置发送缓冲区大小
 * @param handle UDP句柄
 * @param size 缓冲区大小
 * @return 错误码
 */
cudp_error_t cudp_set_send_buffer_size(cudp_handle_t* handle, int size);

/**
 * @brief 设置接收超时
 * @param handle UDP句柄
 * @param timeout_ms 超时时间（毫秒）
 * @return 错误码
 */
cudp_error_t cudp_set_recv_timeout(cudp_handle_t* handle, int timeout_ms);

/**
 * @brief 设置发送超时
 * @param handle UDP句柄
 * @param timeout_ms 超时时间（毫秒）
 * @return 错误码
 */
cudp_error_t cudp_set_send_timeout(cudp_handle_t* handle, int timeout_ms);

/* ========== 组播操作 ========== */

/**
 * @brief 加入组播组
 * @param handle UDP句柄
 * @param multicast_addr 组播地址
 * @param interface_addr 网络接口地址（NULL表示使用默认接口）
 * @return 错误码
 */
cudp_error_t cudp_multicast_join(cudp_handle_t* handle, const char* multicast_addr, const char* interface_addr);

/**
 * @brief 离开组播组
 * @param handle UDP句柄
 * @param multicast_addr 组播地址
 * @param interface_addr 网络接口地址（NULL表示使用默认接口）
 * @return 错误码
 */
cudp_error_t cudp_multicast_leave(cudp_handle_t* handle, const char* multicast_addr, const char* interface_addr);

/**
 * @brief 设置组播TTL
 * @param handle UDP句柄
 * @param ttl TTL值
 * @return 错误码
 */
cudp_error_t cudp_multicast_set_ttl(cudp_handle_t* handle, int ttl);

/**
 * @brief 设置组播回环
 * @param handle UDP句柄
 * @param loop 是否回环
 * @return 错误码
 */
cudp_error_t cudp_multicast_set_loop(cudp_handle_t* handle, int loop);

/**
 * @brief 设置组播发送接口
 * @param handle UDP句柄
 * @param interface_addr 接口地址
 * @return 错误码
 */
cudp_error_t cudp_multicast_set_interface(cudp_handle_t* handle, const char* interface_addr);

/* ========== 辅助函数 ========== */

/**
 * @brief 获取错误消息
 * @param error 错误码
 * @return 错误消息字符串
 */
const char* cudp_error_message(cudp_error_t error);

/**
 * @brief 获取最后的系统错误码
 * @return 系统错误码
 */
int cudp_get_last_error(void);

/**
 * @brief 获取本地endpoint
 * @param handle UDP句柄
 * @param endpoint 本地endpoint（输出参数）
 * @return 错误码
 */
cudp_error_t cudp_get_local_endpoint(cudp_handle_t* handle, cudp_endpoint_t* endpoint);

/**
 * @brief 获取远程endpoint
 * @param handle UDP句柄
 * @param endpoint 远程endpoint（输出参数）
 * @return 错误码
 */
cudp_error_t cudp_get_remote_endpoint(cudp_handle_t* handle, cudp_endpoint_t* endpoint);

/**
 * @brief 检查socket是否打开
 * @param handle UDP句柄
 * @return 1表示打开，0表示关闭
 */
int cudp_is_open(const cudp_handle_t* handle);

/**
 * @brief 获取可用数据大小
 * @param handle UDP句柄
 * @param available 可用字节数（输出参数）
 * @return 错误码
 */
cudp_error_t cudp_available(cudp_handle_t* handle, size_t* available);

#ifdef __cplusplus
}
#endif

#endif /* LABEZ_CUDP_H */

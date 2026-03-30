#ifndef C_CORE_SIO_H_
#define C_CORE_SIO_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file csio.h
 * @brief 跨平台 C99 串口通信库，功能对标 boost::asio::serial_port
 *
 * - Windows: 使用 Win32 API (要求 Vista+ / 0x0600)
 * - Linux/Unix: 使用 termios API
 * - 线程安全设计，所有 API 均可在多线程环境下安全调用
 * - 支持同步和异步 I/O 操作
 * - 完整的串口参数配置：波特率、数据位、停止位、校验位、流控制
 * - 支持超时控制、缓冲区管理、错误处理
 */

/* Windows Vista+ API */
#ifdef _WIN32
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0600
#  endif
#endif

#include "ciosrv.h"
#include <stddef.h>
#include <stdint.h>

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

/** \addtogroup LabEZ_SIO 串口通信
 * @{
 */

/**
 * @brief 串口错误码
 */
typedef enum csio_errc {
    CSIO_OK                    = 0,  /**< 操作成功 */
    CSIO_ERROR_GENERAL         = 1,  /**< 一般性错误 */
    CSIO_ERROR_INVALID_ARG     = 2,  /**< 参数无效 */
    CSIO_ERROR_NOMEM           = 3,  /**< 内存不足 */
    CSIO_ERROR_NOT_OPEN        = 4,  /**< 串口未打开 */
    CSIO_ERROR_ALREADY_OPEN    = 5,  /**< 串口已打开 */
    CSIO_ERROR_OPEN_FAILED     = 6,  /**< 打开串口失败 */
    CSIO_ERROR_CONFIG_FAILED   = 7,  /**< 配置串口失败 */
    CSIO_ERROR_READ_FAILED     = 8,  /**< 读取失败 */
    CSIO_ERROR_WRITE_FAILED    = 9,  /**< 写入失败 */
    CSIO_ERROR_TIMEOUT         = 10, /**< 操作超时 */
    CSIO_ERROR_CANCELLED       = 11, /**< 操作被取消 */
    CSIO_ERROR_INVALID_HANDLE  = 12, /**< 句柄无效 */
    CSIO_ERROR_PORT_NOT_FOUND  = 13, /**< 端口不存在 */
    CSIO_ERROR_ACCESS_DENIED   = 14  /**< 访问被拒绝 */
} csio_errc;

/**
 * @brief 波特率枚举
 */
typedef enum csio_baud_rate {
    CSIO_BAUD_110      = 110,
    CSIO_BAUD_300      = 300,
    CSIO_BAUD_600      = 600,
    CSIO_BAUD_1200     = 1200,
    CSIO_BAUD_2400     = 2400,
    CSIO_BAUD_4800     = 4800,
    CSIO_BAUD_9600     = 9600,
    CSIO_BAUD_14400    = 14400,
    CSIO_BAUD_19200    = 19200,
    CSIO_BAUD_38400    = 38400,
    CSIO_BAUD_56000    = 56000,
    CSIO_BAUD_57600    = 57600,
    CSIO_BAUD_115200   = 115200,
    CSIO_BAUD_128000   = 128000,
    CSIO_BAUD_230400   = 230400,
    CSIO_BAUD_256000   = 256000,
    CSIO_BAUD_460800   = 460800,
    CSIO_BAUD_500000   = 500000,
    CSIO_BAUD_921600   = 921600,
    CSIO_BAUD_1000000  = 1000000,
    CSIO_BAUD_1500000  = 1500000,
    CSIO_BAUD_2000000  = 2000000,
    CSIO_BAUD_3000000  = 3000000
} csio_baud_rate;

/**
 * @brief 数据位数
 */
typedef enum csio_character_size {
    CSIO_DATA_BITS_5 = 5,
    CSIO_DATA_BITS_6 = 6,
    CSIO_DATA_BITS_7 = 7,
    CSIO_DATA_BITS_8 = 8
} csio_character_size;

/**
 * @brief 停止位
 */
typedef enum csio_stop_bits {
    CSIO_STOP_BITS_ONE,           /**< 1 个停止位 */
    CSIO_STOP_BITS_ONE_POINT_FIVE, /**< 1.5 个停止位 */
    CSIO_STOP_BITS_TWO            /**< 2 个停止位 */
} csio_stop_bits;

/**
 * @brief 校验位
 */
typedef enum csio_parity {
    CSIO_PARITY_NONE  = 0, /**< 无校验 */
    CSIO_PARITY_ODD   = 1, /**< 奇校验 */
    CSIO_PARITY_EVEN  = 2, /**< 偶校验 */
    CSIO_PARITY_MARK  = 3, /**< 标记校验 */
    CSIO_PARITY_SPACE = 4  /**< 空格校验 */
} csio_parity;

/**
 * @brief 流控制
 */
typedef enum csio_flow_control {
    CSIO_FLOW_CONTROL_NONE     = 0, /**< 无流控 */
    CSIO_FLOW_CONTROL_SOFTWARE = 1, /**< 软件流控 (XON/XOFF) */
    CSIO_FLOW_CONTROL_HARDWARE = 2  /**< 硬件流控 (RTS/CTS) */
} csio_flow_control;

/**
 * @brief 串口配置结构
 */
typedef struct csio_config {
    csio_baud_rate baud_rate;           /**< 波特率 */
    csio_character_size character_size; /**< 数据位 */
    csio_stop_bits stop_bits;           /**< 停止位 */
    csio_parity parity;                 /**< 校验位 */
    csio_flow_control flow_control;     /**< 流控制 */
    uint32_t read_timeout_ms;           /**< 读超时(毫秒), 0=无限等待 */
    uint32_t write_timeout_ms;          /**< 写超时(毫秒), 0=无限等待 */
} csio_config;

/**
 * @brief 串口句柄（不透明结构）
 */
typedef struct csio_port csio_port;

/**
 * @brief 异步回调函数类型
 * @param error 错误码
 * @param bytes_transferred 传输的字节数
 * @param userdata 用户数据
 */
typedef void (*csio_async_callback)(int error, size_t bytes_transferred, void *userdata);

/**
 * @brief 创建默认串口配置
 * @return 默认配置（9600, 8N1, 无流控）
 */
CC_API csio_config CC_CALL csio_default_config(void);

/**
 * @brief 创建串口对象
 * @return 串口对象指针，失败返回 NULL
 */
CC_API csio_port *CC_CALL csio_create(void);

/**
 * @brief 创建串口对象（带IO服务）
 * @param io_service IO服务（用于异步IO）
 * @return 串口对象指针，失败返回 NULL
 */
CC_API csio_port *CC_CALL csio_create_with_io_service(ciosrv_t* io_service);

/**
 * @brief 销毁串口对象
 * @param pport 串口对象指针的指针
 */
CC_API void CC_CALL csio_destroy(csio_port **pport);

/**
 * @brief 打开串口
 * @param port 串口对象
 * @param device 设备名称 (Windows: "COM1", Linux: "/dev/ttyS0" 或 "/dev/ttyUSB0")
 * @param config 串口配置，NULL 则使用默认配置
 * @return 错误码
 */
CC_API int CC_CALL csio_open(csio_port *port, const char *device, const csio_config *config);

/**
 * @brief 关闭串口
 * @param port 串口对象
 * @return 错误码
 */
CC_API int CC_CALL csio_close(csio_port *port);

/**
 * @brief 检查串口是否已打开
 * @param port 串口对象
 * @return 非 0 表示已打开
 */
CC_API int CC_CALL csio_is_open(const csio_port *port);

/**
 * @brief 同步写入数据
 * @param port 串口对象
 * @param data 数据缓冲区
 * @param size 数据大小
 * @param bytes_written 实际写入的字节数（可选）
 * @return 错误码
 */
CC_API int CC_CALL csio_write(csio_port *port, const void *data, size_t size, size_t *bytes_written);

/**
 * @brief 同步读取数据
 * @param port 串口对象
 * @param buffer 接收缓冲区
 * @param size 缓冲区大小
 * @param bytes_read 实际读取的字节数（可选）
 * @return 错误码
 */
CC_API int CC_CALL csio_read(csio_port *port, void *buffer, size_t size, size_t *bytes_read);

/**
 * @brief 异步写入数据
 * @param port 串口对象
 * @param data 数据缓冲区（调用者负责保持有效直到回调完成）
 * @param size 数据大小
 * @param callback 完成回调
 * @param userdata 用户数据
 * @return 错误码
 */
CC_API int CC_CALL csio_async_write(csio_port *port, const void *data, size_t size, 
                                     csio_async_callback callback, void *userdata);

/**
 * @brief 异步读取数据
 * @param port 串口对象
 * @param buffer 接收缓冲区（调用者负责保持有效直到回调完成）
 * @param size 缓冲区大小
 * @param callback 完成回调
 * @param userdata 用户数据
 * @return 错误码
 */
CC_API int CC_CALL csio_async_read(csio_port *port, void *buffer, size_t size,
                                    csio_async_callback callback, void *userdata);

/**
 * @brief 取消所有异步操作
 * @param port 串口对象
 * @return 错误码
 */
CC_API int CC_CALL csio_cancel(csio_port *port);

/**
 * @brief 设置波特率
 * @param port 串口对象
 * @param baud_rate 波特率
 * @return 错误码
 */
CC_API int CC_CALL csio_set_baud_rate(csio_port *port, csio_baud_rate baud_rate);

/**
 * @brief 获取波特率
 * @param port 串口对象
 * @param baud_rate 输出波特率
 * @return 错误码
 */
CC_API int CC_CALL csio_get_baud_rate(const csio_port *port, csio_baud_rate *baud_rate);

/**
 * @brief 设置数据位
 * @param port 串口对象
 * @param character_size 数据位数
 * @return 错误码
 */
CC_API int CC_CALL csio_set_character_size(csio_port *port, csio_character_size character_size);

/**
 * @brief 获取数据位
 * @param port 串口对象
 * @param character_size 输出数据位数
 * @return 错误码
 */
CC_API int CC_CALL csio_get_character_size(const csio_port *port, csio_character_size *character_size);

/**
 * @brief 设置停止位
 * @param port 串口对象
 * @param stop_bits 停止位
 * @return 错误码
 */
CC_API int CC_CALL csio_set_stop_bits(csio_port *port, csio_stop_bits stop_bits);

/**
 * @brief 获取停止位
 * @param port 串口对象
 * @param stop_bits 输出停止位
 * @return 错误码
 */
CC_API int CC_CALL csio_get_stop_bits(const csio_port *port, csio_stop_bits *stop_bits);

/**
 * @brief 设置校验位
 * @param port 串口对象
 * @param parity 校验位
 * @return 错误码
 */
CC_API int CC_CALL csio_set_parity(csio_port *port, csio_parity parity);

/**
 * @brief 获取校验位
 * @param port 串口对象
 * @param parity 输出校验位
 * @return 错误码
 */
CC_API int CC_CALL csio_get_parity(const csio_port *port, csio_parity *parity);

/**
 * @brief 设置流控制
 * @param port 串口对象
 * @param flow_control 流控制
 * @return 错误码
 */
CC_API int CC_CALL csio_set_flow_control(csio_port *port, csio_flow_control flow_control);

/**
 * @brief 获取流控制
 * @param port 串口对象
 * @param flow_control 输出流控制
 * @return 错误码
 */
CC_API int CC_CALL csio_get_flow_control(const csio_port *port, csio_flow_control *flow_control);

/**
 * @brief 设置读超时
 * @param port 串口对象
 * @param timeout_ms 超时时间(毫秒)，0 表示无限等待
 * @return 错误码
 */
CC_API int CC_CALL csio_set_read_timeout(csio_port *port, uint32_t timeout_ms);

/**
 * @brief 获取读超时
 * @param port 串口对象
 * @param timeout_ms 输出超时时间(毫秒)
 * @return 错误码
 */
CC_API int CC_CALL csio_get_read_timeout(const csio_port *port, uint32_t *timeout_ms);

/**
 * @brief 设置写超时
 * @param port 串口对象
 * @param timeout_ms 超时时间(毫秒)，0 表示无限等待
 * @return 错误码
 */
CC_API int CC_CALL csio_set_write_timeout(csio_port *port, uint32_t timeout_ms);

/**
 * @brief 获取写超时
 * @param port 串口对象
 * @param timeout_ms 输出超时时间(毫秒)
 * @return 错误码
 */
CC_API int CC_CALL csio_get_write_timeout(const csio_port *port, uint32_t *timeout_ms);

/**
 * @brief 刷新输入缓冲区
 * @param port 串口对象
 * @return 错误码
 */
CC_API int CC_CALL csio_flush_input(csio_port *port);

/**
 * @brief 刷新输出缓冲区
 * @param port 串口对象
 * @return 错误码
 */
CC_API int CC_CALL csio_flush_output(csio_port *port);

/**
 * @brief 刷新所有缓冲区
 * @param port 串口对象
 * @return 错误码
 */
CC_API int CC_CALL csio_flush_all(csio_port *port);

/**
 * @brief 获取输入缓冲区中可用字节数
 * @param port 串口对象
 * @param available 输出可用字节数
 * @return 错误码
 */
CC_API int CC_CALL csio_bytes_available(const csio_port *port, size_t *available);

/**
 * @brief 发送 break 信号
 * @param port 串口对象
 * @param duration_ms break 信号持续时间(毫秒)
 * @return 错误码
 */
CC_API int CC_CALL csio_send_break(csio_port *port, uint32_t duration_ms);

/**
 * @brief 设置 DTR (Data Terminal Ready) 信号
 * @param port 串口对象
 * @param level 信号电平 (非 0 为高)
 * @return 错误码
 */
CC_API int CC_CALL csio_set_dtr(csio_port *port, int level);

/**
 * @brief 设置 RTS (Request To Send) 信号
 * @param port 串口对象
 * @param level 信号电平 (非 0 为高)
 * @return 错误码
 */
CC_API int CC_CALL csio_set_rts(csio_port *port, int level);

/**
 * @brief 获取 CTS (Clear To Send) 信号状态
 * @param port 串口对象
 * @param level 输出信号电平
 * @return 错误码
 */
CC_API int CC_CALL csio_get_cts(const csio_port *port, int *level);

/**
 * @brief 获取 DSR (Data Set Ready) 信号状态
 * @param port 串口对象
 * @param level 输出信号电平
 * @return 错误码
 */
CC_API int CC_CALL csio_get_dsr(const csio_port *port, int *level);

/**
 * @brief 获取 RI (Ring Indicator) 信号状态
 * @param port 串口对象
 * @param level 输出信号电平
 * @return 错误码
 */
CC_API int CC_CALL csio_get_ri(const csio_port *port, int *level);

/**
 * @brief 获取 CD/DCD (Carrier Detect) 信号状态
 * @param port 串口对象
 * @param level 输出信号电平
 * @return 错误码
 */
CC_API int CC_CALL csio_get_cd(const csio_port *port, int *level);

/**
 * @brief 枚举系统中的串口
 * @param ports 输出端口名称数组（由调用者释放每个字符串和数组本身）
 * @param count 输出端口数量
 * @return 错误码
 */
CC_API int CC_CALL csio_enumerate_ports(char ***ports, size_t *count);

/**
 * @brief 释放枚举的端口列表
 * @param ports 端口名称数组
 * @param count 端口数量
 */
CC_API void CC_CALL csio_free_ports(char **ports, size_t count);

/**
 * @brief 获取错误描述字符串
 * @param error 错误码
 * @return 错误描述
 */
CC_API const char *CC_CALL csio_strerror(int error);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* C_CORE_SIO_H_ */


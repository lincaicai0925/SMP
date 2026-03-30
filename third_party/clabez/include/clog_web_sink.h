#ifndef CLOG_WEB_SINK_H_
#define CLOG_WEB_SINK_H_

#include "clog.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 配置说明
 * 
 * 可以在编译时通过以下宏配置Web服务器参数：
 * - MAX_WEB_CLIENTS: 最大客户端连接数（默认：16）
 *   例如：gcc -DMAX_WEB_CLIENTS=32 ...
 */

/**
 * @brief Web槽配置结构
 */
typedef struct clog_WebSinkConfig
{
    int port;                           /**< HTTP服务器端口 */
    const char *log_dir;               /**< 日志文件目录路径（UTF-8编码） */
    int max_clients;                   /**< 最大WebSocket客户端连接数 */
    int enable_file_browser;           /**< 是否启用文件浏览功能 */
    int enable_level_control;          /**< 是否启用级别控制功能 */
    int max_connection_lifetime;       /**< 最大连接时长（秒），0表示不限制，默认0 */
    int log_cache_size;                /**< 日志缓存队列大小，默认256 */
} clog_WebSinkConfig;

/**
 * @brief 创建默认的Web槽配置
 * 
 * @return 返回默认配置
 */
CC_API clog_WebSinkConfig CC_CALL clog_web_sink_default_config(void);

/**
 * @brief 添加Web日志槽
 * 
 * 创建一个HTTP服务器和WebSocket服务，支持：
 * - 浏览器查看日志文件列表
 * - 下载日志文件
 * - 实时查看日志输出
 * - 在线配置logger级别
 * 
 * @param config Web槽配置
 * @return 返回槽句柄，失败返回NULL
 */
CC_API clog_SinkHandle CC_CALL clog_add_web_sink(const clog_WebSinkConfig *config);

/**
 * @brief 获取Web槽的访问URL
 * 
 * @param sink_handle 槽句柄
 * @param url_buffer URL输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 0表示成功，非0表示失败
 */
CC_API int CC_CALL clog_web_sink_get_url(clog_SinkHandle sink_handle, char *url_buffer, int buffer_size);

/**
 * @brief 停止Web槽服务器
 * 
 * @param sink_handle 槽句柄
 * @return 0表示成功，非0表示失败
 */
CC_API int CC_CALL clog_web_sink_stop(clog_SinkHandle sink_handle);

#ifdef __cplusplus
}
#endif

#endif /* CLOG_WEB_SINK_H_ */


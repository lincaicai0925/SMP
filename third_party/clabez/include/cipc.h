#ifndef C_CORE_IPC_H
#define C_CORE_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

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

/** \addtogroup cipc 高速进程间通信(线程安全)
* 延时极低,微妙级，CPU占用<1%
* @{
*/

/**
* @brief \struct cipc_msg IPC消息
*
*/
typedef struct cipc_msg
{
    char buf[1500];   /*!< 消息 */
    int  buf_len;     /*!< 消息长度 */
    char target[64];  /*!< 对方节点 */
} cipc_msg;

typedef int cipc;

/**
* @brief 创建IPC实例
*
* @return 返回IPC实例指针
*/
CC_API cipc CC_CALL cipc_create();

/**
* @brief 打开IPC节点
* @param self ipc实例
* @param id ipc节点名称
* @return 返回错误代码,0表示成功
*/
CC_API int CC_CALL cipc_open(cipc self, const char *id);

/**
* @brief 判断节点是否打开
* @param self ipc实例
* @return 返回bool,非0表示打开，0表示关闭
*/
CC_API int CC_CALL cipc_is_open(cipc self);

/**
* @brief 阻塞式读取消息
* @param self ipc实例
* @param timeout_ms 超时时间，单位毫秒
* @param alloc_new_msg_copy_user_free 是否分配一个新的消息副本，由用户自己释放
* @return 返回消息
*/
CC_API cipc_msg * CC_CALL cipc_read(cipc self, unsigned int timeout_ms,int alloc_new_msg_copy_user_free);


/**
* @brief 向某个IPC节点发送消息
* @param self ipc实例
* @param target 目标节点
* @param buf 缓冲区
* @param buf_byte_size 数据长度,最大1024，为了高速通信，请不要发大包，大包使用共享内存
* @param timeout_ms 如果阻塞,指定最大超时时间,最小为1ms，用户传0时，内部当1ms处理
* @return 返回错误代码,0表示成功, 其中11表示写超时
*/
CC_API int CC_CALL cipc_write(cipc self, const char *target, const char *buf, unsigned int buf_byte_size,
                                   unsigned int timeout_ms);

/**
* @brief 关闭IPC节点
* @param self ipc实例
* @return 返回错误代码,0表示成功
*/
CC_API int CC_CALL cipc_close(cipc self);

/**
* @brief 重置IPC节点
* @param self ipc实例
* @return 返回错误代码,0表示成功
*/
CC_API int CC_CALL cipc_reset(cipc self);

/**
* @brief 销毁ICP实例
* @param self ipc实例的指针
* @return 无
*/

CC_API void CC_CALL cipc_destroy(cipc *pself);

/**
* 以下是库的内部函数，用户不应该直接调用以下函数
*/

/** @}*/
#ifdef __cplusplus
}
#endif

#endif

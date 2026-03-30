#ifndef C_CORE_SER_H_
#define C_CORE_SER_H_

#include "cvector.h"
#include <string.h>
#include <assert.h>
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
#else
#define CC_CALL
#define CC_API
#endif

/** \addtogroup cserdes_pack 超轻量级串行化
         * @{
         */

/**   \struct cserdes_pack
              * 容器
              */

typedef struct cser
{
    char  *data;
    size_t size;
    size_t capacity;
} cser;

typedef struct cserdes_iter
{
    cser *ser;
    char *buf_ptr;
    int   error;  // 错误标志：0=无错误，非0=有错误
} cserdes_iter;

CC_API cser *CC_CALL cserdes_create();
CC_API void CC_CALL  cserdes_destroy(cser **pser);

/**
     * @brief 提前为buf分配内存空间
     *        此操作可以提高打包时的效率，是可选操作。
     * @param ser char*类型的缓冲区地址
     * @param size 想要提前分配的内存缓冲区大小。单位(字节)
     * @return 无
     */
CC_API void CC_CALL cserdes_reserve(cser *ser, int size);
/**
     * @brief 获取缓冲区长度
     * @param ser char*类型的缓冲区地址
     * @return 返回缓冲区长度。单位(字节)
     */
CC_API size_t CC_CALL cserdes_size(cser *ser);

CC_API const char *CC_CALL cserdes_data(cser *ser);

/**
     * @brief 打包一个变量到缓冲区。
              注意：类型不包括字符串和数组
     * @param ser char*类型的缓冲区地址
     * @param var 要打包的变量，不包括字符串和数组
     * @return 无
     */
#define cserdes_pack(ser, var) __cserdes_pack((ser), (const char *)(&(var)), sizeof(var))
/**
     * @brief 打包一个字符串到缓冲区。
     * @param ser char*类型的缓冲区地址
     * @param var 要打包的字符串
     * @return 无
     */
#define cserdes_pack_str(ser, str) __cserdes_pack((ser), (const char *)(str), (int)strlen(str) + 1)
/**
     * @brief 打包一段内存串到缓冲区。
     * @param ser char*类型的缓冲区地址
     * @param var 要打包的字符串
     * @return 无
     */
CC_API void CC_CALL cserdes_pack_bytes(cser *ser, const char *bytes_ptr, unsigned int bytes_size);

//--反序列化接口----------------

CC_API cserdes_iter *CC_CALL cserdes_iter_create(cser *ser);
CC_API void CC_CALL       cserdes_iter_destroy(cserdes_iter *iter);
CC_API void CC_CALL       cserdes_iter_init(cserdes_iter *iter, cser *ser);

/**
     * @brief 检查迭代器是否有错误
     * @param iter 迭代器指针
     * @return 0表示无错误，非0表示有错误
     */
CC_API int CC_CALL cserdes_iter_has_error(cserdes_iter *iter);

/**
     * @brief 获取剩余可读字节数
     * @param iter 迭代器指针
     * @return 剩余可读字节数
     */
CC_API size_t CC_CALL cserdes_iter_remaining(cserdes_iter *iter);

/**
     * @brief 解包时要求跳过多少字节
     * @param buf char*类型的缓冲区地址
     * @param bytes_size 要跳过的字节数
     * @return 无
     */

CC_API void CC_CALL cserdes_skip_bytes(cserdes_iter *iter, unsigned int bytes_size);

/**
     * @brief 解包任意类型
     * @param buf char*类型的缓冲区地址
     * @param var 输出变量
     * @return 无
     */
#define cserdes_unpack(iter, var) __cserdes_unpack(iter, (char *)&var, sizeof(var))

/**
     * @brief 提前看一下即将要解包的字符串需要的缓冲区长度
     *        此操作不会将字符串解包，只是便于用户分配足够的内存空间。
     * @param buf char*类型的缓冲区地址
     * @return 返回解包字符串需要内存空间大小。己包括了结束符
     */
CC_API unsigned int CC_CALL cserdes_peek_str_size(cserdes_iter *iter);
/**
     * @brief 解包字符串
     * @param buf char*类型的缓冲区地址
     * @param buf str_buf*输出字符串
     * @return 无
     */
CC_API void CC_CALL cserdes_unpack_str(cserdes_iter *ser, char *str_buf);

/**
     * @brief 提前看一下即将要解包的内存需要的缓冲区长度
     * @param buf char*类型的缓冲区地址
     * @return 返回解包该内存需要内存空间大小。己包括了结束符
     */
CC_API unsigned int CC_CALL cserdes_peek_bytes_size(cserdes_iter *iter);
/**
     * @brief 解包内存
     * @param buf char*类型的缓冲区地址
     * @param dst_mem_ptr 解包的输出buffer地址
     * @param dst_mem_len 解包的输出buffer长度
     * @return 无
     */
CC_API void CC_CALL cserdes_unpack_bytes(cserdes_iter *ser, char *bytes, unsigned  int bytes_size);

/** @}*/

//注意：以下部分是库内部函数，用户不要调用-------------------
CC_API void CC_CALL __cserdes_pack(cser *ser, const char *item_ptr, unsigned  int item_size);

CC_API void CC_CALL __cserdes_unpack(cserdes_iter *ser, char *item_ptr, unsigned  int item_size);

#ifdef __cplusplus
}
#endif

#endif

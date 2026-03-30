#ifndef C_CORE_BUF_H__
#define C_CORE_BUF_H__

#ifdef __cplusplus
extern "C"
{
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
#define USE_STD_INT_T
#ifndef CC_API
#define CC_API __attribute__((visibility("default")))
#endif
#define CC_CALL
#else
#define CC_CALL
#define CC_API
#endif

#ifndef bool_t
    typedef unsigned char bool_t;
#endif

#ifdef USE_STD_INT_T

#include <stdint.h>

#else

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint64_t
#if defined(_MSC_VER) && _MSC_VER <= 1200
typedef unsigned __int64 uint64_t;
#else
typedef unsigned long long uint64_t;
#endif
#endif

#ifndef int8_t
typedef signed char int8_t;
#endif

#ifndef int16_t
typedef short int16_t;
#endif

#ifndef int32_t
typedef int int32_t;
#endif

#ifndef int64_t
#if defined(_MSC_VER) && _MSC_VER <= 1200
typedef __int64 int64_t;
#else
typedef long long int64_t;
#endif
#endif

#endif
    /** \addtogroup LabEZ_Buf 内存编解码
     * @{
     */

    /**
     * @brief 从小端字节序内存中获取某个bit
     *
     * @param buffer 输入缓冲区
     * @param byte_offset 字节偏移
     * @param bit_offset bit偏移
     * @return bool
     */
    CC_API bool_t CC_CALL cbuf_get_bit_le(const void *buffer, int byte_offset, int bit_offset);

    /**
     * @brief 从大端字节序内存中获取某个bit
     *
     * @param buffer 输入缓冲区
     * @param byte_offset 字节偏移
     * @param type_byte_size 原始数据类型占多少个字节,如：char:1;short:2;int:4; 常用sizof(type)获取
     * @param bit_offset bit偏移
     * @return bool
     */
    CC_API bool_t CC_CALL cbuf_get_bit_be(const void *buffer, int byte_offset, int type_byte_size, int bit_offset);

    /**
     * @brief 获取有符号8位整数
     *
     * @param buffer 输入缓冲区
     * @param byte_offset 字节偏移
     * @return 有符号8位整数
     */
    CC_API int8_t CC_CALL cbuf_get_int8(const void *buffer, int byte_offset);

    /**
     * @brief 获取无符号8位数整数
     *
     * @param buffer 输入缓冲区
     * @param byte_offset 字节偏移
     * @return 无符号8位整数
     */
    CC_API uint8_t CC_CALL cbuf_get_uint8(const void *buffer, int byte_offset);

    /**
     * @brief 获取有符号整型16位
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 有符号16位整数
     */
    CC_API int16_t CC_CALL cbuf_get_int16(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取无符号整型16位
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 无符号16位整数
     */
    CC_API uint16_t CC_CALL cbuf_get_uint16(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取有符号整型32位
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 有符号32位整数
     */
    CC_API int32_t CC_CALL cbuf_get_int32(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取无符号整型32位
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 无符号32位整数
     */
    CC_API uint32_t CC_CALL cbuf_get_uint32(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取有符号64位整数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 有符号64位整数
     */
    CC_API int64_t CC_CALL cbuf_get_int64(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取无符号64位整数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 无符号64位整数
     */
    CC_API uint64_t CC_CALL cbuf_get_uint64(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取单精度符点数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 单精度符点数
     */
    CC_API float CC_CALL cbuf_get_float(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 获取双精度浮点数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @return 双精度浮点数
     */
    CC_API double CC_CALL cbuf_get_double(const void *buffer, int is_big_endian, int byte_offset);

    /**
     * @brief 设置小端内存的某个bit位
     *
     * @param buffer 输入缓冲区
     * @param byte_offset 字节偏移
     * @param bit_offset bit位
     * @param v 布尔变量
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_bit_le(void *buffer, int byte_offset, int bit_offset, bool_t v);

    /**
     * @brief 设置大端内存的某个bit位
     *
     * @param buffer 输入缓冲区
     * @param byte_offset 字节偏移
     * @param type_byte_size 原始数据类型占多少个字节,如：char:1;short:2;int:4; 常用sizof(type)获取
     * @param bit_offset bit位
     * @param v 布尔变量
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_bit_be(void *buffer, int byte_offset,int type_byte_size, int bit_offset, bool_t v);

    /**
     * @brief 设置有符号整型8位数
     *
     * @param buffer 输出缓冲区
     * @param byte_offset 字节偏移
     * @param v 有符号8位整数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_int8(void *buffer, int byte_offset, int8_t v);

    /**
     * @brief 设置无符号整型8位数
     *
     * @param buffer 输出缓冲区
     * @param byte_offset 字节偏移
     * @param v 无符号8位整数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_uint8(void *buffer, int byte_offset, uint8_t v);

    /**
     * @brief 设置有符号整型16位数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 有符号16位数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_int16(void *buffer, int is_big_endian, int byte_offset, int16_t v);

    /**
     * @brief 设置无符号整型16位数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 无符号16位数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_uint16(void *buffer, int is_big_endian, int byte_offset, uint16_t v);

    /**
     * @brief 设置整型32位数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 有符号32位数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_int32(void *buffer, int is_big_endian, int byte_offset, int32_t v);

    /**
     * @brief 设置无符号32位数
     *
     * @param buffer 输出缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 无符号32位数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_uint32(void *buffer, int is_big_endian, int byte_offset, uint32_t v);

    /**
     * @brief 设置有符号64位整数
     *
     * @param buffer 输出缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 有符号64位数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_int64(void *buffer, int is_big_endian, int byte_offset, int64_t v);

    /**
     * @brief 设置无符号64位数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 无符号64位数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_uint64(void *buffer, int is_big_endian, int byte_offset, uint64_t v);

    /**
     * @brief 设置单精度浮点数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 单精度浮点数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_float(void *buffer, int is_big_endian, int byte_offset, float v);

    /**
     * @brief 设置双精度浮点数
     *
     * @param buffer 输入缓冲区
     * @param is_big_endian 是否是大端，0为小端、低字节序，1为大端、高字节序
     * @param byte_offset 字节偏移
     * @param v 双精度浮点数
     * @return 无
     */
    CC_API void CC_CALL cbuf_set_double(void *buffer, int is_big_endian, int byte_offset, double v);

/** @}*/
#ifdef __cplusplus
}
#endif /* end of __cplusplus */

#endif // HL__BIT_OPERATOR_H__

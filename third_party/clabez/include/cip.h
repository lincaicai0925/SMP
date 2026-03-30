#ifndef HL_IP_UTIL_H
#define HL_IP_UTIL_H

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
#ifndef CC_API
#define CC_API __attribute__((visibility("default")))
#endif
#define CC_CALL
#endif
    /** \addtogroup ip地址帮助函数
     * @{
     */

    /**
     * @brief 判断两个ip是否相同
     * @param ip0 第一个ip
     * @param ip1 第二个ip
     * @return 相同返回1，不同返回0
     */
    int cip_is_same(const char *ip0, const char *ip1);

    /**
     * @brief 判断IP是否有效
     * @param ip 存储提取的ip地址
     * @return 有效返回1， 无效返回0
     */
    int cip_is_valid(const char *ip);

    /**
     * @brief 提取本地ip地址
     * @param ip 本地ip地址
     * @param buf 存储提取的ip地址
     * @return 提取成功返回0，失败返回-1
     */
    int cip_extract(const char *ip,char * buf);

    /**
     * @brief 解析ip地址
     * @param ip 字符串形式的IP地址
     * @param r0 第一个字段的值
     * @param r1 第二个字段的值
     * @param r2 第三个字段的值
     * @param r3 第四个字段的值
     * @return 成功返回0，失败返回-1
     */
    int cip_parse(const char *ip, unsigned short *r0, unsigned short *r1, unsigned short *r2, unsigned short *r3);

    /** @}*/
#ifdef __cplusplus
}
#endif

#endif
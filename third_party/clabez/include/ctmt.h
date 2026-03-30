#ifndef C_CORE_TIME_UTIL_H_
#define C_CORE_TIME_UTIL_H_
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef CC_EXPORTS
#ifdef CC_STATIC
#undef CC_API
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

#if defined _MSC_VER && _MSC_VER <= 1200
#ifndef uint64_t
#define uint64_t __int64
#endif
#else
#include <stdint.h>
#endif

    /** \addtogroup LabEZ_Time 时间
     *
     * @{
     */

     typedef struct ctmt_str_t
     {
         char c_str[32];
     }ctmt_str_t;

    /**
     * @brief 获取系统当前的毫秒数
     * 
     * @return 毫秒
     */
    CC_API uint64_t CC_CALL ctmt_get_ms(void);

    /**
     * @brief 睡眠
     * 
     * @param ms 毫秒
     * @return 无 
     */
    CC_API void CC_CALL ctmt_sleep_ms(unsigned int ms);


    /**
     * @brief 设置默认定时器起点
     * 这是一个线程安全的函数，使用线程本地存储实现
     *
     * @return 起点时间
     */
	CC_API uint64_t CC_CALL ctmt_set_timer_start();


    /**
     * @brief 获取默认定时器经过了多少毫秒
     * 这是一个线程安全的函数，使用线程本地存储实现
     *
     * @return 经过的毫秒数
     */
	CC_API uint64_t CC_CALL ctmt_get_timer_elapsed();
   /**
     * @brief 获取系统当前时间
     * 
     * @param local_time 是否获取本地时间
     * @return 返回线程安全的字符串
     */
    CC_API ctmt_str_t CC_CALL ctmt_get_now_str(int local_time);
    /** @}*/

#ifdef __cplusplus
}
#endif

#endif
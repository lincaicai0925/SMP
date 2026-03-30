#ifndef HL_CORE_H_
#define HL_CORE_H_

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

#include "c2d.h"
#include "cbuf.h"
#include "clabez.h"
#include "cdebug.h"
#include "cdcv.h"
#include "cdpi.h"
#include "cfs.h"
#include "cicon.h"
#include "cipc.h"
#include "clist.h"
#include "clog.h"
#include "cmap.h"
#include "cmsg.h"
#include "cpktm.h"
#include "cpopt.h"
#include "cqueue.h"
#include "cset.h"
#include "cshm.h"
#include "cstack.h"
#include "cstr.h"
#include "ctmt.h"
#include "ctls.h"
#include "cvector.h"
#include "cserdes.h"

#if defined(_WIN32) || defined(_WIN64)
#include "cproc.h"
#include "cmbox.h"
#else

#endif


/** \addtogroup cCore 基础功能
     * @{
     */
/**   \struct cversion
     * 库版本
     */
typedef struct cversion
{
    unsigned short major;  //!< 主版本号
    unsigned short minor;  //!< 副版本号
} cversion;

/**   \struct cuuid
     * UUID
     */
typedef struct cuuid
{
    char str[64];  //!< 字符串
} cuuid;

/**
     * @brief 获取库版本
     * 
     * @return 返回版本号
     */
CC_API cversion CC_CALL c_get_version();

/**
     * @brief 获取库版本描述字符串
     *
     * @param utf8 是否输出utf8编码
     * @return 获取版本描述字符串
     */
CC_API const char *CC_CALL c_get_version_info_str(int utf8);

/**
     * @brief 产生UUID
     *
     * @return UUID
     */
CC_API cuuid CC_CALL c_make_uuid();

/**
     * @brief 设置当前线程最后的错误
     *
     * @param error_code 错误代码
     * @return 无
     */
CC_API void CC_CALL c_set_last_error(int error_code);

/**
     * @brief 获取最后的错误代码
     *
     * @return 错误代码
     */
CC_API int CC_CALL c_get_last_error();

/**
     * @brief 检查错误代码
     * 假定作用域中存在ret变量以及goto标签"EXIT"。
     * @param exp 表达式
     * @return 无
     */
#define C_CHECK_RET_ERR_CODE(exp) \
    error_code = (exp);               \
    if (error_code)                   \
    {                                 \
        goto EXIT;                     \
    }


/**
     * @brief 检查表达式结果是否为真
     * 假定作用域中存在ret变量以及goto标签"EXIT"。
     * @param ret_err_If_exp_not_true 如果前面的表达式exp不为真，设置将此参数设置为ret
     * @return 无
     */
        
#define C_ENSURE_EXP_IS_TRUE(exp, use_this_error_code_If_exp_result_not_true) \
    if (!(exp))                                                                   \
    {                                                                             \
        error_code = (use_this_error_code_If_exp_result_not_true);                \
        goto EXIT;                                                                 \
    }

/**
     * @brief 跳转到到"ERR"标签处，并设置错误代码
     * 假定作用域中存在ret变量以及goto标签"EXIT"。
     * @param ec 为ret设置错误代码
     * @return 无
     */
#define C_EXIT_WITH_CODE(ec) {error_code = (ec); goto EXIT;}
/**
     * @brief 比较浮点数是否相同
     * @param l =左边的变量
     * @param r =右边的变量
     * @param epsilon =精度
     * @return bool
     */
#define C_FLOAT_EQ(l, r, epsilon) (((l) > ((r) - (epsilon))) && ((l) < ((r) + (epsilon))))

/**
     * @brief 比较浮点数是否不同
     * @param l =左边的变量
     * @param r =右边的变量
     * @param epsilon =精度
     * @return bool
     */
#define C_FLOAT_NE(l, r, epsilon) (((l) < ((r) - (epsilon))) || ((l) > ((r) + (epsilon))))
/**
     * @brief 编译时确定数组大小
     * @param arr 数组
     * @return 整数
     */
#define C_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/**
     * @brief 释放指针，并将指针置0
     * @param ptr 指针
     * @return 无
     */
#define c_free_ptr(ptr) {free(ptr); ptr = 0;}

/**
     * @brief 分配一个数据结构，并将内容置0
     * @param ptr 结构类型
     * @return 无
     */
#define c_malloc_obj(type) (type *)calloc(1,sizeof(type));

/**
     * @brief 释放结构
     * @param ptr 结构类型
     * @return 无
     */
#define c_free_obj  free_ptr

/**
     * @brief 分配内存，内容预置为0。类似malloc
     * @param type 返回类似
     * @param bytes 字节数
     * @return 无
     */
#define c_malloc_mem(type, bytes) (type *)calloc(1, (bytes))

/**
     * @brief 释放内存
     * @param ptr 指针
     * @return 无
     */
#define c_free_mem  free_obj


#define c_min(a, b) ((a) < (b) ? (a) : (b))
#define c_max(a, b) ((a) > (b) ? (a) : (b))

#define C_PI 3.14159265358979323846f

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define c_deg_to_rad(a) ((a) * (M_PI / 180))
#define c_rad_to_deg(a) ((a) * (180 / M_PI))

#define c_swap(TYPE, a, b) \
    do                              \
    {                               \
        TYPE ___t;                  \
        ___t = (a);                 \
        (a)  = (b);                 \
        (b)  = ___t;                \
    } while (0)


/** @}*/

#ifdef __cplusplus
}
#endif

#endif

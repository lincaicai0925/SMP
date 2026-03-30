#ifndef C_CORE_BITSET_H_
#define C_CORE_BITSET_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif
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

/**
 * @brief 定义每个 unsigned long 的位数
 */
#define ULONG_BITS (sizeof(unsigned long) * 8)

/**
 * @brief 定义固定大小的位集合类型和相关操作函数
 * 
 * 该宏用于生成指定大小的位集合(bitset)类型及其所有操作函数。
 * 位集合是一个紧凑的数据结构，用于存储和操作固定数量的位(bit)。
 * 
 * @param N 位集合的位数(bit数量)，必须是正整数
 * 
 * @note 该宏会生成以下内容：
 *       1. 类型定义: cbitset_N 结构体
 *       2. 所有操作函数: cbitset_N_create, cbitset_N_destroy, cbitset_N_set 等
 * 
 * @note 生成的类型和函数名称会包含指定的位数 N
 *       例如: DEFINE_CBITSET(32) 会生成 cbitset_32 类型
 * 
 * @par 使用示例:
 * @code
 * // 定义一个 128 位的位集合类型
 * DEFINE_CBITSET(128)
 * 
 * // 使用生成的类型和函数
 * cbitset_128 bs;
 * cbitset_128_init(&bs);
 * cbitset_128_set(&bs, 10);  // 设置第10位
 * if (cbitset_128_test(&bs, 10)) {
 *     printf("Bit 10 is set\n");
 * }
 * cbitset_128_uninit(&bs);
 * @endcode
 */
#define DEFINE_CBITSET(N)                                                 \
    typedef struct cbitset_##N                                            \
    {                                                                     \
        unsigned long data[(N + ULONG_BITS - 1) / ULONG_BITS];            \
    } cbitset_##N;                                                        \
                                                                          \
    /**                                                                   \
     * @brief 动态创建位集合对象                                          \
     *                                                                    \
     * 在堆上分配并初始化一个新的位集合对象，所有位初始化为0。            \
     *                                                                    \
     * @return cbitset_##N* 成功返回指向新创建的位集合的指针，失败返回NULL \
     *                                                                    \
     * @note 使用完毕后必须调用 cbitset_##N##_destroy() 释放内存         \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N *pbs = cbitset_##N##_create();                         \
     * if (pbs) {                                                         \
     *     cbitset_##N##_set(pbs, 5);                                     \
     *     cbitset_##N##_destroy(&pbs);                                   \
     * }                                                                  \
     * @endcode                                                           \
     */                                                                   \
    cbitset_##N *cbitset_##N##_create()                                   \
    {                                                                     \
        return (cbitset_##N *)__cbitset_t_create(N);                      \
    }                                                                     \
    /**                                                                   \
     * @brief 销毁动态创建的位集合对象                                     \
     *                                                                    \
     * 释放位集合占用的内存，并将指针设置为NULL。                          \
     *                                                                    \
     * @param pbs 指向位集合指针的指针，不能为NULL                         \
     *                                                                    \
     * @note 该函数会自动将 *pbs 设置为 NULL                              \
     * @note 如果 pbs 为 NULL 或 *pbs 为 NULL，函数不执行任何操作         \
     * @note 只能销毁通过 cbitset_##N##_create() 创建的对象               \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N *pbs = cbitset_##N##_create();                         \
     * // ... 使用位集合 ...                                              \
     * cbitset_##N##_destroy(&pbs);  // pbs 将被设置为 NULL              \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_destroy(cbitset_##N **pbs)                         \
    {                                                                     \
        if (pbs && *pbs)                                                  \
        {                                                                 \
            __cbitset_t_destroy((__cbitset_t *)(*pbs));                   \
            *pbs = 0;                                                     \
        }                                                                 \
    }                                                                     \
    /**                                                                   \
     * @brief 初始化栈上分配的位集合对象                                   \
     *                                                                    \
     * 初始化一个在栈上或静态分配的位集合，所有位初始化为0。               \
     *                                                                    \
     * @param bs 指向待初始化的位集合对象的指针，不能为NULL                \
     *                                                                    \
     * @note 该函数用于栈上分配的对象，不需要动态内存分配                  \
     * @note 使用完毕后应调用 cbitset_##N##_uninit() 清理                 \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 3);                                         \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_init(cbitset_##N *bs)                              \
    {                                                                     \
        __cbitset_t_init((__cbitset_t *)bs, N);                           \
    }                                                                     \
    /**                                                                   \
     * @brief 清理栈上分配的位集合对象                                     \
     *                                                                    \
     * 清理通过 cbitset_##N##_init() 初始化的位集合对象。                  \
     *                                                                    \
     * @param bs 指向待清理的位集合对象的指针，不能为NULL                  \
     *                                                                    \
     * @note 该函数用于清理栈上分配的对象                                  \
     * @note 不能用于清理通过 cbitset_##N##_create() 创建的对象           \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * // ... 使用位集合 ...                                              \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_uninit(cbitset_##N *bs)                            \
    {                                                                     \
        __cbitset_t_uninit((__cbitset_t *)bs, N);                         \
    }                                                                     \
    /**                                                                   \
     * @brief 设置指定位置的位为1                                          \
     *                                                                    \
     * 将位集合中指定位置的位设置为1。                                     \
     *                                                                    \
     * @param bs  指向位集合对象的指针，不能为NULL                         \
     * @param pos 要设置的位的位置(从0开始)，必须小于N                     \
     *                                                                    \
     * @note 如果 pos >= N，行为未定义                                    \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 5);   // 设置第5位为1                      \
     * cbitset_##N##_set(&bs, 10);  // 设置第10位为1                     \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_set(cbitset_##N *bs, size_t pos)                   \
    {                                                                     \
        __cbitset_t_set((__cbitset_t *)bs, pos, N);                       \
    }                                                                     \
    /**                                                                   \
     * @brief 设置指定位置的位为0                                          \
     *                                                                    \
     * 将位集合中指定位置的位重置为0(清零)。                                \
     *                                                                    \
     * @param bs  指向位集合对象的指针，不能为NULL                         \
     * @param pos 要重置的位的位置(从0开始)，必须小于N                     \
     *                                                                    \
     * @note 如果 pos >= N，行为未定义                                    \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 5);      // 设置第5位为1                   \
     * cbitset_##N##_reset(&bs, 5);    // 重置第5位为0                   \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_reset(cbitset_##N *bs, size_t pos)                 \
    {                                                                     \
        __cbitset_t_reset((__cbitset_t *)bs, pos, N);                     \
    }                                                                     \
    /**                                                                   \
     * @brief 翻转指定位置的位                                            \
     *                                                                    \
     * 将位集合中指定位置的位取反(0变1，1变0)。                            \
     *                                                                    \
     * @param bs  指向位集合对象的指针，不能为NULL                         \
     * @param pos 要翻转的位的位置(从0开始)，必须小于N                     \
     *                                                                    \
     * @note 如果 pos >= N，行为未定义                                    \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_flip(&bs, 3);  // 第3位: 0->1                       \
     * cbitset_##N##_flip(&bs, 3);  // 第3位: 1->0                       \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_flip(cbitset_##N *bs, size_t pos)                  \
    {                                                                     \
        __cbitset_t_flip((__cbitset_t *)bs, pos, N);                      \
    }                                                                     \
    /**                                                                   \
     * @brief 测试指定位置的位是否为1                                      \
     *                                                                    \
     * 检查位集合中指定位置的位的值。                                      \
     *                                                                    \
     * @param bs  指向位集合对象的指针，不能为NULL                         \
     * @param pos 要测试的位的位置(从0开始)，必须小于N                     \
     *                                                                    \
     * @return int 如果指定位为1返回非0值，如果为0返回0                    \
     *                                                                    \
     * @note 如果 pos >= N，行为未定义                                    \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 7);                                         \
     * if (cbitset_##N##_test(&bs, 7)) {                                  \
     *     printf("Bit 7 is set\n");                                      \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    int cbitset_##N##_test(const cbitset_##N *bs, size_t pos)             \
    {                                                                     \
        return __cbitset_t_test((__cbitset_t *)bs, pos, N);               \
    }                                                                     \
    /**                                                                   \
     * @brief 设置所有位为1                                               \
     *                                                                    \
     * 将位集合中的所有位都设置为1。                                       \
     *                                                                    \
     * @param bs 指向位集合对象的指针，不能为NULL                          \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set_all(&bs);  // 所有位都变为1                     \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_set_all(cbitset_##N *bs)                           \
    {                                                                     \
        __cbitset_t_set_all((__cbitset_t *)bs, N);                        \
    }                                                                     \
    /**                                                                   \
     * @brief 重置所有位为0                                               \
     *                                                                    \
     * 将位集合中的所有位都重置为0(清零)。                                  \
     *                                                                    \
     * @param bs 指向位集合对象的指针，不能为NULL                          \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set_all(&bs);     // 所有位设为1                    \
     * cbitset_##N##_reset_all(&bs);   // 所有位重置为0                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_reset_all(cbitset_##N *bs)                         \
    {                                                                     \
        __cbitset_t_reset_all((__cbitset_t *)bs, N);                      \
    }                                                                     \
    /**                                                                   \
     * @brief 翻转所有位                                                  \
     *                                                                    \
     * 将位集合中的所有位都取反(0变1，1变0)。                              \
     *                                                                    \
     * @param bs 指向位集合对象的指针，不能为NULL                          \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 0);      // 第0位为1，其他为0              \
     * cbitset_##N##_flip_all(&bs);    // 第0位变为0，其他变为1          \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_flip_all(cbitset_##N *bs)                          \
    {                                                                     \
        __cbitset_t_flip_all((__cbitset_t *)bs, N);                       \
    }                                                                     \
    /**                                                                   \
     * @brief 执行位与(AND)操作                                            \
     *                                                                    \
     * 对两个位集合执行按位与操作，结果存储在 dst 中。                     \
     * 操作: dst = dst & src (每一位都执行与运算)                          \
     *                                                                    \
     * @param dst 目标位集合指针，同时也是第一个操作数，不能为NULL         \
     * @param src 源位集合指针，第二个操作数，不能为NULL                   \
     *                                                                    \
     * @note 该操作会修改 dst 的内容                                      \
     * @note dst 和 src 可以指向同一个对象                                \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs1, bs2;                                              \
     * cbitset_##N##_init(&bs1);                                          \
     * cbitset_##N##_init(&bs2);                                          \
     * cbitset_##N##_set(&bs1, 3);  // bs1: bit3=1                       \
     * cbitset_##N##_set(&bs2, 3);  // bs2: bit3=1                       \
     * cbitset_##N##_and(&bs1, &bs2);  // bs1: bit3=1                    \
     * cbitset_##N##_uninit(&bs1);                                        \
     * cbitset_##N##_uninit(&bs2);                                        \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_and(cbitset_##N *dst, const cbitset_##N *src)      \
    {                                                                     \
        __cbitset_t_and((__cbitset_t *)dst, (const __cbitset_t *)src, N); \
    }                                                                     \
    /**                                                                   \
     * @brief 执行位或(OR)操作                                             \
     *                                                                    \
     * 对两个位集合执行按位或操作，结果存储在 dst 中。                     \
     * 操作: dst = dst | src (每一位都执行或运算)                          \
     *                                                                    \
     * @param dst 目标位集合指针，同时也是第一个操作数，不能为NULL         \
     * @param src 源位集合指针，第二个操作数，不能为NULL                   \
     *                                                                    \
     * @note 该操作会修改 dst 的内容                                      \
     * @note dst 和 src 可以指向同一个对象                                \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs1, bs2;                                              \
     * cbitset_##N##_init(&bs1);                                          \
     * cbitset_##N##_init(&bs2);                                          \
     * cbitset_##N##_set(&bs1, 1);  // bs1: bit1=1                       \
     * cbitset_##N##_set(&bs2, 2);  // bs2: bit2=1                       \
     * cbitset_##N##_or(&bs1, &bs2);   // bs1: bit1=1, bit2=1            \
     * cbitset_##N##_uninit(&bs1);                                        \
     * cbitset_##N##_uninit(&bs2);                                        \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_or(cbitset_##N *dst, const cbitset_##N *src)       \
    {                                                                     \
        __cbitset_t_or((__cbitset_t *)dst, (const __cbitset_t *)src, N);  \
    }                                                                     \
    /**                                                                   \
     * @brief 执行位异或(XOR)操作                                          \
     *                                                                    \
     * 对两个位集合执行按位异或操作，结果存储在 dst 中。                   \
     * 操作: dst = dst ^ src (每一位都执行异或运算)                        \
     *                                                                    \
     * @param dst 目标位集合指针，同时也是第一个操作数，不能为NULL         \
     * @param src 源位集合指针，第二个操作数，不能为NULL                   \
     *                                                                    \
     * @note 该操作会修改 dst 的内容                                      \
     * @note dst 和 src 可以指向同一个对象                                \
     * @note 异或操作: 两位相同为0，不同为1                               \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs1, bs2;                                              \
     * cbitset_##N##_init(&bs1);                                          \
     * cbitset_##N##_init(&bs2);                                          \
     * cbitset_##N##_set(&bs1, 5);  // bs1: bit5=1                       \
     * cbitset_##N##_set(&bs2, 5);  // bs2: bit5=1                       \
     * cbitset_##N##_xor(&bs1, &bs2);  // bs1: bit5=0 (1^1=0)            \
     * cbitset_##N##_uninit(&bs1);                                        \
     * cbitset_##N##_uninit(&bs2);                                        \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_xor(cbitset_##N *dst, const cbitset_##N *src)      \
    {                                                                     \
        __cbitset_t_xor((__cbitset_t *)dst, (const __cbitset_t *)src, N); \
    }                                                                     \
    /**                                                                   \
     * @brief 执行位非(NOT)操作                                            \
     *                                                                    \
     * 对位集合执行按位取反操作，所有位都取反(0变1，1变0)。                \
     * 操作: bs = ~bs (每一位都执行非运算)                                 \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @note 该操作会修改 bs 的内容                                       \
     * @note 该函数等同于 cbitset_##N##_flip_all()                        \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 0);   // bit0=1, 其他为0                  \
     * cbitset_##N##_not(&bs);      // bit0=0, 其他为1                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_not(cbitset_##N *bs)                               \
    {                                                                     \
        __cbitset_t_not((__cbitset_t *)bs, N);                            \
    }                                                                     \
    /**                                                                   \
     * @brief 获取位集合的大小                                            \
     *                                                                    \
     * 返回位集合可以容纳的位数(bit数量)。                                 \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return size_t 位集合的大小(总位数)，固定返回 N                    \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * size_t size = cbitset_##N##_size(&bs);  // 返回 N                 \
     * printf("Bitset size: %zu\n", size);                                \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    size_t cbitset_##N##_size(cbitset_##N *bs)                            \
    {                                                                     \
        return __cbitset_t_size((__cbitset_t *)bs, N);                    \
    }                                                                     \
    /**                                                                   \
     * @brief 统计值为1的位的数量                                          \
     *                                                                    \
     * 计算位集合中有多少个位被设置为1。                                   \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return size_t 值为1的位的总数，范围 [0, N]                        \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 1);                                         \
     * cbitset_##N##_set(&bs, 5);                                         \
     * size_t cnt = cbitset_##N##_count(&bs);  // 返回 2                 \
     * printf("Set bits: %zu\n", cnt);                                    \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    size_t cbitset_##N##_count(const cbitset_##N *bs)                     \
    {                                                                     \
        return __cbitset_t_count((__cbitset_t *)bs, N);                   \
    }                                                                     \
    /**                                                                   \
     * @brief 检查是否至少有一位为1                                        \
     *                                                                    \
     * 判断位集合中是否存在至少一个位被设置为1。                           \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return int 如果至少有一位为1返回非0值，如果所有位都为0返回0       \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * if (!cbitset_##N##_any(&bs)) {                                     \
     *     printf("All bits are 0\n");                                    \
     * }                                                                  \
     * cbitset_##N##_set(&bs, 3);                                         \
     * if (cbitset_##N##_any(&bs)) {                                      \
     *     printf("At least one bit is 1\n");                             \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    int cbitset_##N##_any(const cbitset_##N *bs)                          \
    {                                                                     \
        return __cbitset_t_any((__cbitset_t *)bs, N);                     \
    }                                                                     \
    /**                                                                   \
     * @brief 检查是否所有位都为0                                          \
     *                                                                    \
     * 判断位集合中是否所有位都为0(没有任何位被设置)。                     \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return int 如果所有位都为0返回非0值，如果至少有一位为1返回0       \
     *                                                                    \
     * @note 该函数是 cbitset_##N##_any() 的逻辑取反                      \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * if (cbitset_##N##_none(&bs)) {                                     \
     *     printf("No bits are set\n");                                   \
     * }                                                                  \
     * cbitset_##N##_set(&bs, 7);                                         \
     * if (!cbitset_##N##_none(&bs)) {                                    \
     *     printf("At least one bit is set\n");                           \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    int cbitset_##N##_none(const cbitset_##N *bs)                         \
    {                                                                     \
        return __cbitset_t_none((__cbitset_t *)bs, N);                    \
    }                                                                     \
    /**                                                                   \
     * @brief 检查是否所有位都为1                                          \
     *                                                                    \
     * 判断位集合中是否所有位都被设置为1。                                 \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return int 如果所有位都为1返回非0值，如果至少有一位为0返回0       \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set_all(&bs);                                        \
     * if (cbitset_##N##_all(&bs)) {                                      \
     *     printf("All bits are set\n");                                  \
     * }                                                                  \
     * cbitset_##N##_reset(&bs, 0);                                       \
     * if (!cbitset_##N##_all(&bs)) {                                     \
     *     printf("Not all bits are set\n");                              \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    int cbitset_##N##_all(const cbitset_##N *bs)                          \
    {                                                                     \
        return __cbitset_t_all((__cbitset_t *)bs, N);                     \
    }                                                                     \
    /**                                                                   \
     * @brief 将位集合转换为 unsigned long                                 \
     *                                                                    \
     * 将位集合的低位部分转换为一个 unsigned long 整数。                   \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return unsigned long 位集合的 unsigned long 表示                  \
     *                                                                    \
     * @note 如果位集合大小超过 unsigned long 的位数，只转换低位部分       \
     * @note 位0对应返回值的最低位                                        \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 0);  // bit0 = 1                           \
     * cbitset_##N##_set(&bs, 2);  // bit2 = 1                           \
     * unsigned long val = cbitset_##N##_to_ulong(&bs);  // val = 5 (101b) \
     * printf("Value: %lu\n", val);                                       \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    unsigned long cbitset_##N##_to_ulong(const cbitset_##N *bs)           \
    {                                                                     \
        return __cbitset_t_to_ulong((__cbitset_t *)bs, N);                \
    }                                                                     \
    /**                                                                   \
     * @brief 将位集合转换为字符串                                         \
     *                                                                    \
     * 将位集合转换为由 '0' 和 '1' 字符组成的字符串表示。                  \
     * 字符串从高位到低位排列，最左边是最高位。                            \
     *                                                                    \
     * @param bs  位集合指针，不能为NULL                                   \
     * @param str 输出字符串缓冲区，不能为NULL，必须至少有 N+1 字节空间    \
     *                                                                    \
     * @note str 必须预先分配足够的空间(至少 N+1 字节)                    \
     * @note 生成的字符串会自动添加 '\0' 结束符                            \
     * @note 字符串格式: 索引N-1的位在最左边，索引0的位在最右边           \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * char str[N + 1];                                                   \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 0);                                         \
     * cbitset_##N##_set(&bs, 2);                                         \
     * cbitset_##N##_to_string(&bs, str);                                 \
     * printf("Bitset: %s\n", str);  // 输出类似 "00...00101"            \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_to_string(const cbitset_##N *bs, char *str)        \
    {                                                                     \
        __cbitset_t_to_string((__cbitset_t *)bs, str, N);                 \
    }                                                                     \
    /**                                                                   \
     * @brief 执行左移操作                                                \
     *                                                                    \
     * 将位集合的所有位向左(高位方向)移动指定的位数。                      \
     * 右侧(低位)空出的位填充为0，左侧(高位)溢出的位丢失。                 \
     *                                                                    \
     * @param bs    位集合指针，不能为NULL                                 \
     * @param shift 要移动的位数                                          \
     *                                                                    \
     * @note 如果 shift >= N，所有位都将变为0                             \
     * @note 该操作会修改 bs 的内容                                       \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 0);  // ...00001                           \
     * cbitset_##N##_left_shift(&bs, 2);  // ...00100                    \
     * // bit0 和 bit1 现在为0，原来的 bit0 移到了 bit2                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_left_shift(cbitset_##N *bs, size_t shift)          \
    {                                                                     \
        __cbitset_t_left_shift((__cbitset_t *)bs, shift, N);              \
    }                                                                     \
    /**                                                                   \
     * @brief 执行右移操作                                                \
     *                                                                    \
     * 将位集合的所有位向右(低位方向)移动指定的位数。                      \
     * 左侧(高位)空出的位填充为0，右侧(低位)溢出的位丢失。                 \
     *                                                                    \
     * @param bs    位集合指针，不能为NULL                                 \
     * @param shift 要移动的位数                                          \
     *                                                                    \
     * @note 如果 shift >= N，所有位都将变为0                             \
     * @note 该操作会修改 bs 的内容                                       \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 4);  // ...10000                           \
     * cbitset_##N##_right_shift(&bs, 2);  // ...00100                   \
     * // bit4 现在为0，原来的 bit4 移到了 bit2                           \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_right_shift(cbitset_##N *bs, size_t shift)         \
    {                                                                     \
        __cbitset_t_right_shift((__cbitset_t *)bs, shift, N);             \
    }                                                                     \
    /**                                                                   \
     * @brief 比较两个位集合是否相等                                       \
     *                                                                    \
     * 判断两个位集合的所有位是否完全相同。                                \
     *                                                                    \
     * @param lhs 第一个位集合指针，不能为NULL                             \
     * @param rhs 第二个位集合指针，不能为NULL                             \
     *                                                                    \
     * @return int 如果两个位集合完全相同返回非0值，否则返回0              \
     *                                                                    \
     * @note lhs 和 rhs 可以指向同一个对象(此时总是返回非0值)             \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs1, bs2;                                              \
     * cbitset_##N##_init(&bs1);                                          \
     * cbitset_##N##_init(&bs2);                                          \
     * cbitset_##N##_set(&bs1, 3);                                        \
     * cbitset_##N##_set(&bs2, 3);                                        \
     * if (cbitset_##N##_equal(&bs1, &bs2)) {                             \
     *     printf("Bitsets are equal\n");                                 \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs1);                                        \
     * cbitset_##N##_uninit(&bs2);                                        \
     * @endcode                                                           \
     */                                                                   \
    int cbitset_##N##_equal(const cbitset_##N *lhs, const cbitset_##N *rhs) \
    {                                                                     \
        return __cbitset_t_equal((const __cbitset_t *)lhs, (const __cbitset_t *)rhs, N); \
    }                                                                     \
    /**                                                                   \
     * @brief 查找第一个被设置的位                                         \
     *                                                                    \
     * 从低位到高位查找第一个值为1的位的索引。                             \
     *                                                                    \
     * @param bs 位集合指针，不能为NULL                                    \
     *                                                                    \
     * @return size_t 第一个值为1的位的索引，如果没有找到返回 N           \
     *                                                                    \
     * @note 如果所有位都为0，返回 N                                      \
     * @note 索引从0开始                                                  \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 5);                                         \
     * cbitset_##N##_set(&bs, 10);                                        \
     * size_t first = cbitset_##N##_find_first(&bs);  // 返回 5          \
     * printf("First set bit: %zu\n", first);                             \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    size_t cbitset_##N##_find_first(const cbitset_##N *bs)                \
    {                                                                     \
        return __cbitset_t_find_first((const __cbitset_t *)bs, N);        \
    }                                                                     \
    /**                                                                   \
     * @brief 查找下一个被设置的位                                         \
     *                                                                    \
     * 从指定位置之后开始查找，返回下一个值为1的位的索引。                 \
     *                                                                    \
     * @param bs  位集合指针，不能为NULL                                   \
     * @param pos 开始查找的位置(查找范围是 pos+1 到 N-1)                  \
     *                                                                    \
     * @return size_t 下一个值为1的位的索引，如果没有找到返回 N           \
     *                                                                    \
     * @note 如果从 pos+1 到 N-1 的所有位都为0，返回 N                    \
     * @note 该函数不检查位置 pos 本身                                    \
     * @note 如果 pos >= N-1，返回 N                                      \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set(&bs, 3);                                         \
     * cbitset_##N##_set(&bs, 7);                                         \
     * cbitset_##N##_set(&bs, 15);                                        \
     * // 遍历所有被设置的位                                              \
     * size_t pos = cbitset_##N##_find_first(&bs);                        \
     * while (pos < N) {                                                  \
     *     printf("Bit %zu is set\n", pos);                               \
     *     pos = cbitset_##N##_find_next(&bs, pos);                       \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    size_t cbitset_##N##_find_next(const cbitset_##N *bs, size_t pos)     \
    {                                                                     \
        return __cbitset_t_find_next((const __cbitset_t *)bs, pos, N);    \
    }                                                                     \
    /**                                                                   \
     * @brief 设置指定位置的位为指定值                                     \
     *                                                                    \
     * 将位集合中指定位置的位设置为给定的值(0或1)。                        \
     *                                                                    \
     * @param bs    位集合指针，不能为NULL                                 \
     * @param pos   要设置的位的位置(从0开始)，必须小于N                   \
     * @param value 要设置的值，非0值表示1，0表示0                         \
     *                                                                    \
     * @note 如果 pos >= N，行为未定义                                    \
     * @note 如果 value 非0，位被设置为1；如果 value 为0，位被设置为0     \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_set_value(&bs, 5, 1);   // 设置第5位为1             \
     * cbitset_##N##_set_value(&bs, 10, 0);  // 设置第10位为0            \
     * int flag = 1;                                                      \
     * cbitset_##N##_set_value(&bs, 3, flag);  // 根据flag设置第3位      \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_set_value(cbitset_##N *bs, size_t pos, int value)  \
    {                                                                     \
        __cbitset_t_set_value((__cbitset_t *)bs, pos, value, N);          \
    }                                                                     \
    /**                                                                   \
     * @brief 从 unsigned long 值初始化位集合                              \
     *                                                                    \
     * 使用一个 unsigned long 整数的位模式来设置位集合。                   \
     * 先清空位集合，然后将整数的每一位复制到位集合的对应位置。            \
     *                                                                    \
     * @param bs    位集合指针，不能为NULL                                 \
     * @param value 要转换的 unsigned long 值                             \
     *                                                                    \
     * @note value 的最低位(LSB)对应位集合的 bit0                         \
     * @note 如果位集合大小大于 unsigned long 的位数，高位部分填充0       \
     * @note 该操作会清空位集合的原有内容                                  \
     *                                                                    \
     * @par 使用示例:                                                     \
     * @code                                                              \
     * cbitset_##N bs;                                                    \
     * cbitset_##N##_init(&bs);                                           \
     * cbitset_##N##_from_ulong(&bs, 13);  // 13 = 1101b                 \
     * // 结果: bit0=1, bit2=1, bit3=1, 其他位=0                         \
     * if (cbitset_##N##_test(&bs, 0) &&                                  \
     *     cbitset_##N##_test(&bs, 2) &&                                  \
     *     cbitset_##N##_test(&bs, 3)) {                                  \
     *     printf("Bits correctly set\n");                                \
     * }                                                                  \
     * cbitset_##N##_uninit(&bs);                                         \
     * @endcode                                                           \
     */                                                                   \
    void cbitset_##N##_from_ulong(cbitset_##N *bs, unsigned long value)   \
    {                                                                     \
        __cbitset_t_from_ulong((__cbitset_t *)bs, value, N);              \
    }

    typedef struct __cbitset_t __cbitset_t;
    CC_API __cbitset_t *CC_CALL __cbitset_t_create(size_t N);
    CC_API void CC_CALL __cbitset_t_destroy(__cbitset_t *bs);
    CC_API void CC_CALL __cbitset_t_init(__cbitset_t *bs, size_t N);
    CC_API void CC_CALL __cbitset_t_uninit(__cbitset_t *bs, size_t N);
    /* 基础操作 */
    CC_API void CC_CALL __cbitset_t_set(__cbitset_t *bs, size_t pos, size_t N);
    CC_API void CC_CALL __cbitset_t_reset(__cbitset_t *bs, size_t pos, size_t N);
    CC_API void CC_CALL __cbitset_t_flip(__cbitset_t *bs, size_t pos, size_t N);
    CC_API int CC_CALL __cbitset_t_test(const __cbitset_t *bs, size_t pos, size_t N);
    /* 全量操作 */
    CC_API void CC_CALL __cbitset_t_set_all(__cbitset_t *bs, size_t N);
    CC_API void CC_CALL __cbitset_t_reset_all(__cbitset_t *bs, size_t N);
    CC_API void CC_CALL __cbitset_t_flip_all(__cbitset_t *bs, size_t N);
    /* 逻辑运算 */
    CC_API void CC_CALL __cbitset_t_and(__cbitset_t *dst, const __cbitset_t *src, size_t N);
    CC_API void CC_CALL __cbitset_t_or(__cbitset_t *dst, const __cbitset_t *src, size_t N);
    CC_API void CC_CALL __cbitset_t_xor(__cbitset_t *dst, const __cbitset_t *src, size_t N);
    CC_API void CC_CALL __cbitset_t_not(__cbitset_t *bs, size_t N);
    /* 查询操作 */
    CC_API size_t CC_CALL __cbitset_t_size(const __cbitset_t *bs, size_t N);
    CC_API size_t CC_CALL __cbitset_t_count(const __cbitset_t *bs, size_t N);
    CC_API int CC_CALL __cbitset_t_any(const __cbitset_t *bs, size_t N);
    CC_API int CC_CALL __cbitset_t_none(const __cbitset_t *bs, size_t N);
    CC_API int CC_CALL __cbitset_t_all(const __cbitset_t *bs, size_t N);
    /* 转换操作 */
    CC_API unsigned long CC_CALL __cbitset_t_to_ulong(const __cbitset_t *bs, size_t N);
    CC_API void CC_CALL __cbitset_t_to_string(const __cbitset_t *bs, char *str, size_t N);
    CC_API void CC_CALL __cbitset_t_from_ulong(__cbitset_t *bs, unsigned long value, size_t N);
    /* 新增功能：参考 C++23 std::bitset */
    /* 移位操作 */
    CC_API void CC_CALL __cbitset_t_left_shift(__cbitset_t *bs, size_t shift, size_t N);
    CC_API void CC_CALL __cbitset_t_right_shift(__cbitset_t *bs, size_t shift, size_t N);
    /* 比较操作 */
    CC_API int CC_CALL __cbitset_t_equal(const __cbitset_t *lhs, const __cbitset_t *rhs, size_t N);
    /* 查找操作 */
    CC_API size_t CC_CALL __cbitset_t_find_first(const __cbitset_t *bs, size_t N);
    CC_API size_t CC_CALL __cbitset_t_find_next(const __cbitset_t *bs, size_t pos, size_t N);
    /* 辅助操作 */
    CC_API void CC_CALL __cbitset_t_set_value(__cbitset_t *bs, size_t pos, int value, size_t N);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @file cset.h
 * @brief C 语言实现的泛型集合（Set）容器
 * 
 * 本模块提供基于红黑树的有序集合实现，支持任意类型的元素。
 * 集合中的元素自动排序且唯一，提供高效的插入、删除、查找操作。
 * 
 * @features
 * - 基于红黑树实现，保证 O(log n) 的插入/删除/查找性能
 * - 支持任意类型：基本类型、结构体、指针等
 * - 元素自动排序，支持自定义比较函数
 * - 元素唯一性：不允许重复元素
 * - 双向迭代：支持正向和反向遍历
 * - 内存高效：仅在需要时分配节点
 * - 跨平台：支持 Windows、Linux 等平台
 * - C99 标准：兼容性好，易于移植
 * 
 * @usage_basic
 * @code
 * // 1. 定义集合类型
 * DEFINE_CSET(int);
 * 
 * // 2. 创建集合
 * cset_int *set = cset_int_create();
 * 
 * // 3. 插入元素
 * cset_int_insert(set, 10);
 * cset_int_insert(set, 5);
 * cset_int_insert(set, 15);
 * 
 * // 4. 查找元素
 * if (cset_int_contains(set, 10)) {
 *     printf("找到 10\n");
 * }
 * 
 * // 5. 遍历集合
 * cset_foreach(set, int, value) {
 *     printf("%d ", value);  // 输出: 5 10 15（已排序）
 * }
 * 
 * // 6. 删除元素
 * cset_int_remove(set, 10);
 * 
 * // 7. 销毁集合
 * cset_int_destroy(&set);
 * @endcode
 * 
 * @usage_custom_type
 * @code
 * // 使用自定义结构体
 * typedef struct {
 *     int id;
 *     char name[32];
 * } Person;
 * 
 * // 自定义比较函数
 * int person_cmp(const Person *a, const Person *b) {
 *     return a->id - b->id;
 * }
 * 
 * DEFINE_CSET(Person);
 * cset_Person *set = cset_Person_create_with_cmp(person_cmp);
 * 
 * Person p1 = {1, "Alice"};
 * Person p2 = {2, "Bob"};
 * cset_Person_insert(set, p1);
 * cset_Person_insert(set, p2);
 * 
 * cset_Person_destroy(&set);
 * @endcode
 * 
 * @note 集合节点中的值不可直接修改，必须使用 insert/remove/erase/clear 等函数
 * @note 集合操作不是线程安全的，多线程访问需要外部同步
 * @note 比较函数必须满足严格弱序关系
 * @note 迭代器在集合修改后可能失效
 * 
 * @performance
 * - 插入: O(log n)
 * - 删除: O(log n)
 * - 查找: O(log n)
 * - 遍历: O(n)
 * - 空间: O(n)
 * 
 * @thread_safety 不是线程安全的
 * 
 * @author labez_core 团队
 * @date 2025
 */

#ifndef C_CORE_SET_H_
#define C_CORE_SET_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <string.h>
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
    /*注意：千万注意，本模块中的value_ptr指针指向的内容不可直接修改，只能使用set的insert,remove,erase,clear修改*/

/**
 * @brief 定义一个指定类型的集合(set)
 * 
 * 此宏用于定义一个基于红黑树实现的有序集合数据结构，支持插入、删除、查找等操作。
 * 集合内元素自动按照默认比较函数(memcmp)排序，不允许重复元素。
 * 
 * @param TYPE 集合中存储的元素类型
 * 
 * @note 此宏会生成 cset_TYPE 类型和相关操作函数
 * @note 元素使用 memcmp 进行比较，适用于基本数据类型
 * @note 生成的集合名称为 cset_TYPE，例如 DEFINE_CSET(int) 生成 cset_int
 * 
 * @code
 * // 定义一个存储 int 类型的集合
 * DEFINE_CSET(int);
 * 
 * // 创建集合
 * cset_int *set = cset_int_create();
 * 
 * // 插入元素
 * cset_int_insert(set, 10);
 * cset_int_insert(set, 20);
 * 
 * // 销毁集合
 * cset_int_destroy(&set);
 * @endcode
 */
#define DEFINE_CSET(TYPE) DEFINE_CSET_AS(TYPE, TYPE)

/**
 * @brief 定义一个存储指针类型的集合(set)
 * 
 * 此宏用于定义一个存储指针类型的集合，集合名称自动添加 _ptr 后缀。
 * 
 * @param TYPE 指针指向的数据类型（不包含*）
 * 
 * @note 生成的集合类型为 cset_TYPE_ptr
 * @note 例如 DEFINE_CSET_PTR(int) 生成 cset_int_ptr，存储 int* 类型
 * 
 * @code
 * // 定义一个存储 int* 类型的集合
 * DEFINE_CSET_PTR(int);
 * 
 * cset_int_ptr *set = cset_int_ptr_create();
 * int *p1 = malloc(sizeof(int));
 * *p1 = 100;
 * cset_int_ptr_insert(set, p1);
 * cset_int_ptr_destroy(&set);
 * @endcode
 */
#define DEFINE_CSET_PTR(TYPE) DEFINE_CSET_AS(TYPE *, TYPE##_ptr)

/**
 * @brief 定义一个存储指针类型的集合，并指定自定义名称
 * 
 * 此宏用于定义存储指针类型的集合，并允许用户自定义集合类型名称。
 * 
 * @param TYPE 指针指向的数据类型（不包含*）
 * @param NAME 自定义的集合名称
 * 
 * @note 生成的集合类型为 cset_NAME
 * 
 * @code
 * // 定义一个存储 char* 的集合，命名为 str_set
 * DEFINE_CSET_PTR_AS(char, str_set);
 * 
 * cset_str_set *set = cset_str_set_create();
 * cset_str_set_destroy(&set);
 * @endcode
 */
#define DEFINE_CSET_PTR_AS(TYPE, NAME) DEFINE_CSET_AS(TYPE *, NAME)

/**
 * @brief 定义一个完整的集合类型及其所有操作函数
 * 
 * 此宏是集合定义的核心宏，用于生成完整的集合数据结构和所有操作函数。
 * 集合基于红黑树实现，保证元素有序且不重复，支持高效的插入、删除、查找操作。
 * 
 * @param TYPE 集合中存储的元素类型（可以是基本类型、结构体或指针）
 * @param NAME 集合的自定义名称，最终生成的类型为 cset_NAME
 * 
 * @note 生成的主要类型：
 *       - cset_NAME: 集合类型
 *       - cset_NAME_node: 集合节点类型
 *       - cset_NAME_cmp_cb: 比较函数类型
 * 
 * @note 生成的操作函数包括：
 *       - 创建/初始化: create, init, create_with_cmp, init_with_cmp
 *       - 销毁/清理: destroy, uninit, clear
 *       - 插入/删除: insert, remove, erase
 *       - 查找: find, contains, lower_bound, upper_bound, equal_range
 *       - 访问: front, back, front_ptr, back_ptr, get_value
 *       - 迭代: begin, end, next, prev
 *       - 信息: size, empty, count, max_size
 *       - 其他: swap, merge, key_comp, value_comp
 * 
 * @note 时间复杂度：
 *       - 插入/删除/查找: O(log n)
 *       - 访问最小/最大元素: O(log n)
 *       - 遍历: O(n)
 * 
 * @code
 * // 定义一个存储 double 的集合，命名为 my_set
 * DEFINE_CSET_AS(double, my_set);
 * 
 * cset_my_set *set = cset_my_set_create();
 * cset_my_set_insert(set, 3.14);
 * cset_my_set_insert(set, 2.71);
 * 
 * if (cset_my_set_contains(set, 3.14)) {
 *     printf("Found 3.14\n");
 * }
 * 
 * cset_my_set_destroy(&set);
 * @endcode
 */
#define DEFINE_CSET_AS(TYPE, NAME)                                                                                                              \
    /** @brief 红黑树节点颜色枚举 */                                                                                                                   \
    typedef enum cset_##NAME##_color{                                                                                                           \
        cset_##NAME##_RB_RED,    /**< 红色节点 */                                                                                                \
        cset_##NAME##_RB_BLACK   /**< 黑色节点 */                                                                                                \
    } cset_##NAME##_color;                                                                                                                      \
                                                                                                                                                \
    /**                                                                                                                                         \
     * @brief 集合节点结构体                                                                                                                          \
     * @note 节点中的 value_ptr 指针及其指向的内容不可直接修改                                                                                              \
     * @note 只能通过 insert、remove、erase、clear 等函数修改集合                                                                                        \
     */                                                                                                                                         \
    typedef struct cset_##NAME##_node                                                                                                           \
    {                                                                                                                                           \
        const TYPE * const value_ptr; /**< 指向元素值的常量指针（不可修改） */                                                                              \
        struct cset_##NAME##_node *left;    /**< 左子节点指针 */                                                                                   \
        struct cset_##NAME##_node *right;   /**< 右子节点指针 */                                                                                   \
        struct cset_##NAME##_node *parent;  /**< 父节点指针 */                                                                                     \
        cset_##NAME##_color color;          /**< 节点颜色（红/黑） */                                                                               \
    } cset_##NAME##_node;                                                                                                                       \
                                                                                                                                                \
    /**                                                                                                                                         \
     * @brief 比较函数类型定义                                                                                                                         \
     * @param lhs 左操作数指针                                                                                                                       \
     * @param rhs 右操作数指针                                                                                                                       \
     * @return <0: lhs < rhs, =0: lhs == rhs, >0: lhs > rhs                                                                                     \
     */                                                                                                                                         \
    typedef int (*cset_##NAME##_cmp_cb)(const TYPE *lhs, const TYPE *rhs);                                                                      \
                                                                                                                                                \
    /**                                                                                                                                         \
     * @brief 集合结构体                                                                                                                             \
     */                                                                                                                                         \
    typedef struct cset_##NAME                                                                                                                  \
    {                                                                                                                                           \
        size_t size;                    /**< 集合中元素的数量 */                                                                                      \
        cset_##NAME##_node *root;       /**< 红黑树根节点指针 */                                                                                     \
        cset_##NAME##_node *nil;        /**< 哨兵节点（NIL），用于表示空节点和 end() */                                                                    \
        cset_##NAME##_cmp_cb cmp;       /**< 元素比较函数指针 */                                                                                      \
    } cset_##NAME;                                                                                                                              \
                                                                                                                                                \
    /** @brief 默认比较函数，使用 memcmp 比较两个元素 */                                                                                             \
    static INLINE int cset_##NAME##_cmp(const TYPE *lhs, const TYPE *rhs)                                                                       \
    {                                                                                                                                           \
        return memcmp(lhs, rhs, sizeof(TYPE));                                                                                                  \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 创建一个新的集合对象                                                                                                                  \
     *                                                                                                                                           \
     * 在堆上动态分配内存创建集合对象，使用默认的 memcmp 比较函数。                                                                                         \
     *                                                                                                                                           \
     * @return cset_##NAME* 返回新创建的集合指针，失败返回 NULL                                                                                          \
     *                                                                                                                                           \
     * @note 使用完毕后必须调用 cset_##NAME##_destroy() 释放内存                                                                                         \
     * @note 此函数使用 memcmp 作为默认比较函数，适用于基本数据类型                                                                                           \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * if (set) {                                                                                                                              \
     *     cset_int_insert(set, 42);                                                                                                            \
     *     cset_int_destroy(&set);                                                                                                              \
     * }                                                                                                                                        \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME *cset_##NAME##_create()                                                                                           \
    {                                                                                                                                           \
        return (cset_##NAME *)__cset_create((__cset_t_cmp_cb)cset_##NAME##_cmp, sizeof(cset_##NAME), sizeof(cset_##NAME##_node), sizeof(TYPE)); \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 使用自定义比较函数创建集合对象                                                                                                             \
     *                                                                                                                                           \
     * 在堆上动态分配内存创建集合对象，使用用户提供的自定义比较函数。                                                                                             \
     *                                                                                                                                           \
     * @param cb 自定义的比较函数，签名为 int (*cmp)(const TYPE *lhs, const TYPE *rhs)                                                                   \
     *           返回值: <0 表示 lhs < rhs, =0 表示 lhs == rhs, >0 表示 lhs > rhs                                                                      \
     *                                                                                                                                           \
     * @return cset_##NAME* 返回新创建的集合指针，失败返回 NULL                                                                                          \
     *                                                                                                                                           \
     * @note 使用完毕后必须调用 cset_##NAME##_destroy() 释放内存                                                                                         \
     * @note 适用于需要自定义排序规则的场景，如结构体、字符串等                                                                                                  \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * int my_cmp(const int *a, const int *b) {                                                                                                \
     *     return *b - *a;  // 降序排列                                                                                                           \
     * }                                                                                                                                        \
     * cset_int *set = cset_int_create_with_cmp(my_cmp);                                                                                        \
     * cset_int_insert(set, 1);                                                                                                                 \
     * cset_int_insert(set, 3);                                                                                                                 \
     * cset_int_insert(set, 2);                                                                                                                 \
     * // 集合中元素顺序为: 3, 2, 1                                                                                                                   \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME *cset_##NAME##_create_with_cmp(cset_##NAME##_cmp_cb cb)                                                           \
    {                                                                                                                                           \
        return (cset_##NAME *)__cset_create((__cset_t_cmp_cb)cb, sizeof(cset_##NAME), sizeof(cset_##NAME##_node), sizeof(TYPE));                \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 初始化一个已存在的集合对象                                                                                                                 \
     *                                                                                                                                           \
     * 在栈上或已分配的内存中初始化集合对象，使用默认的 memcmp 比较函数。                                                                                          \
     *                                                                                                                                           \
     * @param set 指向待初始化的集合对象的指针                                                                                                             \
     *                                                                                                                                           \
     * @return int 成功返回 0，失败返回非 0 值                                                                                                          \
     *                                                                                                                                           \
     * @note 使用完毕后必须调用 cset_##NAME##_uninit() 清理资源                                                                                           \
     * @note 此函数不分配集合对象本身的内存，仅初始化其内部结构                                                                                                    \
     * @note 适用于栈上分配或嵌入到其他结构体中的集合                                                                                                          \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int set;  // 栈上分配                                                                                                                 \
     * if (cset_int_init(&set) == 0) {                                                                                                          \
     *     cset_int_insert(&set, 100);                                                                                                          \
     *     cset_int_uninit(&set);  // 释放内部资源                                                                                                   \
     * }                                                                                                                                        \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_init(cset_##NAME *set)                                                                                      \
    {                                                                                                                                           \
        return __cset_init((__cset_t *)set, (__cset_t_cmp_cb)cset_##NAME##_cmp, sizeof(cset_##NAME), sizeof(cset_##NAME##_node), sizeof(TYPE)); \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 使用自定义比较函数初始化集合对象                                                                                                               \
     *                                                                                                                                           \
     * 在栈上或已分配的内存中初始化集合对象，使用用户提供的自定义比较函数。                                                                                              \
     *                                                                                                                                           \
     * @param set 指向待初始化的集合对象的指针                                                                                                             \
     * @param cb  自定义的比较函数，签名为 int (*cmp)(const TYPE *lhs, const TYPE *rhs)                                                                   \
     *            返回值: <0 表示 lhs < rhs, =0 表示 lhs == rhs, >0 表示 lhs > rhs                                                                      \
     *                                                                                                                                           \
     * @return int 成功返回 0，失败返回非 0 值                                                                                                          \
     *                                                                                                                                           \
     * @note 使用完毕后必须调用 cset_##NAME##_uninit() 清理资源                                                                                           \
     * @note 适用于需要自定义排序规则且在栈上分配的场景                                                                                                         \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * int desc_cmp(const int *a, const int *b) {                                                                                              \
     *     return *b - *a;                                                                                                                      \
     * }                                                                                                                                        \
     * cset_int set;                                                                                                                            \
     * cset_int_init_with_cmp(&set, desc_cmp);                                                                                                  \
     * cset_int_insert(&set, 5);                                                                                                                \
     * cset_int_uninit(&set);                                                                                                                   \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_init_with_cmp(cset_##NAME *set, cset_##NAME##_cmp_cb cb)                                                    \
    {                                                                                                                                           \
        return __cset_init((__cset_t *)set, (__cset_t_cmp_cb)cb, sizeof(cset_##NAME), sizeof(cset_##NAME##_node), sizeof(TYPE));                \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 销毁集合对象并释放所有内存                                                                                                                   \
     *                                                                                                                                           \
     * 销毁通过 cset_##NAME##_create() 或 cset_##NAME##_create_with_cmp() 创建的集合对象，                                                                \
     * 释放集合占用的所有内存，并将指针设置为 NULL。                                                                                                          \
     *                                                                                                                                           \
     * @param pset 指向集合指针的指针（二级指针）                                                                                                            \
     *                                                                                                                                           \
     * @return void 无返回值                                                                                                                       \
     *                                                                                                                                           \
     * @note 传入二级指针以便将原指针设置为 NULL，避免悬空指针                                                                                                  \
     * @note 如果 pset 或 *pset 为 NULL，函数安全返回，不做任何操作                                                                                            \
     * @note 销毁后指针被设置为 NULL，可以安全地多次调用                                                                                                       \
     * @note 此函数用于 create 创建的集合，init 初始化的集合应使用 uninit                                                                                       \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 10);                                                                                                                \
     * cset_int_destroy(&set);  // 销毁并置为 NULL                                                                                                  \
     * // set 现在为 NULL，可以安全地再次调用                                                                                                              \
     * cset_int_destroy(&set);  // 安全，不会出错                                                                                                      \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE void cset_##NAME##_destroy(cset_##NAME **pset)                                                                                \
    {                                                                                                                                           \
        if (pset && *pset)                                                                                                                      \
        {                                                                                                                                       \
            __cset_destroy((__cset_t *)(*pset));                                                                                                \
            *pset = 0;                                                                                                                          \
        }                                                                                                                                       \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 向集合中插入一个元素                                                                                                                       \
     *                                                                                                                                           \
     * 将指定的值插入到集合中。如果元素已存在（根据比较函数判断），则不插入。                                                                                            \
     * 插入操作保持集合的有序性和唯一性。                                                                                                                     \
     *                                                                                                                                           \
     * @param set   集合对象指针                                                                                                                     \
     * @param value 要插入的元素值                                                                                                                     \
     *                                                                                                                                           \
     * @return int 成功插入返回 0，元素已存在或失败返回非 0 值                                                                                                 \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)，其中 n 是集合中元素个数                                                                                                    \
     * @note 如果元素已存在，函数返回失败，集合不变                                                                                                            \
     * @note 集合中的元素按照比较函数定义的顺序自动排序                                                                                                          \
     * @note 插入操作会分配新节点的内存                                                                                                                     \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * if (cset_int_insert(set, 42) == 0) {                                                                                                     \
     *     printf("插入成功\n");                                                                                                                    \
     * }                                                                                                                                        \
     * if (cset_int_insert(set, 42) != 0) {                                                                                                     \
     *     printf("元素已存在\n");  // 重复插入失败                                                                                                       \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_insert(cset_##NAME *set, const TYPE value)                                                                  \
    {                                                                                                                                           \
        return __cset_insert((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE));                                  \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合中元素的数量                                                                                                                       \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return int 返回集合中元素的数量                                                                                                                  \
     *                                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                                       \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 1);                                                                                                                 \
     * cset_int_insert(set, 2);                                                                                                                 \
     * printf("元素数量: %d\n", cset_int_size(set));  // 输出: 2                                                                                      \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE size_t cset_##NAME##_size(cset_##NAME *set)                                                                                      \
    {                                                                                                                                           \
        return __cset_size((__cset_t *)set);                                                                                                    \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 检查集合是否为空                                                                                                                         \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return int 集合为空返回非 0 值，非空返回 0                                                                                                          \
     *                                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                                       \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * if (cset_int_empty(set)) {                                                                                                               \
     *     printf("集合为空\n");                                                                                                                    \
     * }                                                                                                                                        \
     * cset_int_insert(set, 10);                                                                                                                \
     * if (!cset_int_empty(set)) {                                                                                                              \
     *     printf("集合非空\n");                                                                                                                    \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_empty(cset_##NAME *set)                                                                                     \
    {                                                                                                                                           \
        return __cset_empty((__cset_t *)set);                                                                                                   \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 在集合中查找指定元素                                                                                                                       \
     *                                                                                                                                           \
     * 根据给定的值在集合中查找对应的节点。                                                                                                                   \
     *                                                                                                                                           \
     * @param set   集合对象指针                                                                                                                     \
     * @param value 要查找的元素值                                                                                                                     \
     *                                                                                                                                           \
     * @return cset_##NAME##_node* 找到返回节点指针，未找到返回 NULL（实际是 nil 节点）                                                                          \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 可以通过 cset_##NAME##_get_value() 获取节点中的值                                                                                            \
     * @note 返回的节点指针不应直接修改，应使用相应的集合操作函数                                                                                                    \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 42);                                                                                                                \
     * cset_int_node *node = cset_int_find(set, 42);                                                                                            \
     * if (node != cset_int_end(set)) {                                                                                                         \
     *     printf("找到元素: %d\n", cset_int_get_value(node));                                                                                       \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_find(cset_##NAME *set, const TYPE value)                                                    \
    {                                                                                                                                           \
        return (cset_##NAME##_node *)(__cset_find((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE)));            \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合中的第一个节点（最小元素）                                                                                                              \
     *                                                                                                                                           \
     * 返回指向集合中最小元素节点的指针，用于遍历集合。                                                                                                           \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return cset_##NAME##_node* 返回第一个节点指针，集合为空则返回 end()                                                                                    \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 与 end() 配合使用进行遍历                                                                                                                   \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 3);                                                                                                                 \
     * cset_int_insert(set, 1);                                                                                                                 \
     * cset_int_insert(set, 2);                                                                                                                 \
     * for (cset_int_node *it = cset_int_begin(set);                                                                                            \
     *      it != cset_int_end(set);                                                                                                            \
     *      it = cset_int_next(set, it)) {                                                                                                      \
     *     printf("%d ", cset_int_get_value(it));  // 输出: 1 2 3                                                                                 \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_begin(cset_##NAME *set)                                                                     \
    {                                                                                                                                           \
        return (cset_##NAME##_node *)__cset_begin((__cset_t *)set);                                                                             \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合的结束标记（哨兵节点）                                                                                                                  \
     *                                                                                                                                           \
     * 返回指向集合结束位置的哨兵节点，用于判断遍历是否结束。                                                                                                        \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return cset_##NAME##_node* 返回 end 哨兵节点指针                                                                                                \
     *                                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                                       \
     * @note end() 不指向有效元素，不能对其调用 get_value()                                                                                                 \
     * @note 通常用于循环终止条件                                                                                                                         \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 5);                                                                                                                 \
     * for (cset_int_node *it = cset_int_begin(set);                                                                                            \
     *      it != cset_int_end(set);  // 检查是否到达末尾                                                                                              \
     *      it = cset_int_next(set, it)) {                                                                                                      \
     *     // 处理元素                                                                                                                              \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_end(cset_##NAME *set)                                                                       \
    {                                                                                                                                           \
        return (cset_##NAME##_node *)__cset_end((__cset_t *)set);                                                                               \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取指定节点的下一个节点                                                                                                                     \
     *                                                                                                                                           \
     * 返回指向当前节点的下一个节点的指针，用于正向遍历集合。                                                                                                        \
     *                                                                                                                                           \
     * @param set  集合对象指针                                                                                                                      \
     * @param node 当前节点指针                                                                                                                       \
     *                                                                                                                                           \
     * @return cset_##NAME##_node* 返回下一个节点指针，如果是最后一个节点则返回 end()                                                                              \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)（平摊 O(1)）                                                                                                          \
     * @note 如果 node 是最后一个元素，返回 end()                                                                                                          \
     * @note 不应对 end() 节点调用 next()                                                                                                              \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 1);                                                                                                                 \
     * cset_int_insert(set, 2);                                                                                                                 \
     * cset_int_node *first = cset_int_begin(set);                                                                                              \
     * cset_int_node *second = cset_int_next(set, first);                                                                                       \
     * printf("%d\n", cset_int_get_value(second));  // 输出: 2                                                                                    \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_next(cset_##NAME *set, cset_##NAME##_node *node)                                            \
    {                                                                                                                                           \
        return (cset_##NAME##_node *)(__cset_next((__cset_t *)set, (__cset_node_t *)node));                                                     \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取指定节点的前一个节点                                                                                                                     \
     *                                                                                                                                           \
     * 返回指向当前节点的前一个节点的指针，用于反向遍历集合。                                                                                                        \
     *                                                                                                                                           \
     * @param set  集合对象指针                                                                                                                      \
     * @param node 当前节点指针                                                                                                                       \
     *                                                                                                                                           \
     * @return cset_##NAME##_node* 返回前一个节点指针，如果是第一个节点则返回 end()                                                                              \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)（平摊 O(1)）                                                                                                          \
     * @note 对 end() 调用 prev() 将返回最后一个元素                                                                                                       \
     * @note 对 begin() 调用 prev() 将返回 end()                                                                                                      \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 1);                                                                                                                 \
     * cset_int_insert(set, 2);                                                                                                                 \
     * cset_int_node *last = cset_int_prev(set, cset_int_end(set));                                                                             \
     * printf("最后一个元素: %d\n", cset_int_get_value(last));  // 输出: 2                                                                              \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_prev(cset_##NAME *set, cset_##NAME##_node *node)                                            \
    {                                                                                                                                           \
        return (cset_##NAME##_node *)(__cset_prev((__cset_t *)set, (__cset_node_t *)node));                                                     \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 从节点中获取元素值                                                                                                                         \
     *                                                                                                                                           \
     * 从给定的集合节点中提取存储的元素值（返回值拷贝）。                                                                                                          \
     *                                                                                                                                           \
     * @param node 集合节点指针                                                                                                                       \
     *                                                                                                                                           \
     * @return TYPE 返回节点中存储的元素值的拷贝                                                                                                             \
     *                                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                                       \
     * @note 不要对 end() 节点调用此函数                                                                                                                  \
     * @note 返回的是值的拷贝，对于大对象可能有性能开销，可考虑使用指针版本                                                                                              \
     * @note 节点中的值不应直接修改，应使用集合操作函数                                                                                                           \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 42);                                                                                                                \
     * cset_int_node *node = cset_int_find(set, 42);                                                                                            \
     * if (node != cset_int_end(set)) {                                                                                                         \
     *     int value = cset_int_get_value(node);                                                                                                \
     *     printf("值: %d\n", value);                                                                                                            \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE TYPE cset_##NAME##_get_value(cset_##NAME##_node *node)                                                                        \
    {                                                                                                                                           \
        TYPE *re = (TYPE *)(__cset_get_value((__cset_node_t *)node, sizeof(cset_##NAME##_node), sizeof(TYPE)));                                 \
        return *re;                                                                                                                             \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 检查集合中是否包含指定元素                                                                                                                    \
     *                                                                                                                                           \
     * 判断集合中是否存在与给定值相等的元素。                                                                                                                  \
     *                                                                                                                                           \
     * @param set   集合对象指针                                                                                                                     \
     * @param value 要检查的元素值                                                                                                                     \
     *                                                                                                                                           \
     * @return int 包含返回非 0 值，不包含返回 0                                                                                                           \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 等价于 find(value) != end()，但语义更清晰                                                                                                    \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 100);                                                                                                               \
     * if (cset_int_contains(set, 100)) {                                                                                                       \
     *     printf("集合包含 100\n");                                                                                                                \
     * }                                                                                                                                        \
     * if (!cset_int_contains(set, 200)) {                                                                                                      \
     *     printf("集合不包含 200\n");                                                                                                               \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_contains(cset_##NAME *set, const TYPE value)                                                                \
    {                                                                                                                                           \
        return __cset_contains((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE));                                \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 从集合中移除指定值的元素                                                                                                                     \
     *                                                                                                                                           \
     * 根据给定的值从集合中查找并删除对应的元素。                                                                                                                \
     *                                                                                                                                           \
     * @param set   集合对象指针                                                                                                                     \
     * @param value 要移除的元素值                                                                                                                     \
     *                                                                                                                                           \
     * @return int 成功移除返回 0，元素不存在或失败返回非 0 值                                                                                                  \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 如果元素不存在，函数返回失败                                                                                                                   \
     * @note 移除操作会释放节点占用的内存                                                                                                                   \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 10);                                                                                                                \
     * cset_int_insert(set, 20);                                                                                                                \
     * if (cset_int_remove(set, 10) == 0) {                                                                                                     \
     *     printf("成功移除 10\n");                                                                                                                 \
     * }                                                                                                                                        \
     * if (cset_int_remove(set, 10) != 0) {                                                                                                     \
     *     printf("10 已不存在\n");                                                                                                                 \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_remove(cset_##NAME *set, const TYPE value)                                                                  \
    {                                                                                                                                           \
        return __cset_remove((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE));                                  \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 通过节点指针从集合中删除元素                                                                                                                   \
     *                                                                                                                                           \
     * 直接通过节点指针删除集合中的元素，比 remove() 效率更高（无需查找）。                                                                                            \
     *                                                                                                                                           \
     * @param set  集合对象指针                                                                                                                      \
     * @param node 要删除的节点指针                                                                                                                     \
     *                                                                                                                                           \
     * @return int 成功返回 0，失败返回非 0 值                                                                                                           \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 节点指针必须是有效的且属于该集合                                                                                                                 \
     * @note 不要对 end() 节点调用此函数                                                                                                                  \
     * @note 删除后节点指针失效，不应再使用                                                                                                                   \
     * @note 如果在遍历中删除，应先获取下一个节点再删除当前节点                                                                                                       \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 15);                                                                                                                \
     * cset_int_node *node = cset_int_find(set, 15);                                                                                            \
     * if (node != cset_int_end(set)) {                                                                                                         \
     *     cset_int_erase(set, node);  // 直接删除节点，无需再次查找                                                                                        \
     *     printf("删除成功\n");                                                                                                                    \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE int cset_##NAME##_erase(cset_##NAME *set, cset_##NAME##_node *node)                                                           \
    {                                                                                                                                           \
        return __cset_erase((__cset_t *)set, (__cset_node_t *)node);                                                                            \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 清空集合中的所有元素                                                                                                                       \
     *                                                                                                                                           \
     * 删除集合中的所有元素，释放所有节点占用的内存，但保留集合对象本身。                                                                                                  \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return void 无返回值                                                                                                                       \
     *                                                                                                                                           \
     * @note 时间复杂度: O(n)                                                                                                                       \
     * @note 清空后集合大小变为 0，但集合对象仍然有效，可以继续使用                                                                                                   \
     * @note 所有指向集合节点的迭代器和指针都将失效                                                                                                             \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 1);                                                                                                                 \
     * cset_int_insert(set, 2);                                                                                                                 \
     * printf("清空前: %d\n", cset_int_size(set));  // 输出: 2                                                                                       \
     * cset_int_clear(set);                                                                                                                     \
     * printf("清空后: %d\n", cset_int_size(set));  // 输出: 0                                                                                       \
     * cset_int_insert(set, 3);  // 可以继续使用                                                                                                      \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE void cset_##NAME##_clear(cset_##NAME *set)                                                                                    \
    {                                                                                                                                           \
        __cset_clear((__cset_t *)set);                                                                                                          \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合中的最小元素（值拷贝）                                                                                                                 \
     *                                                                                                                                           \
     * 返回集合中最小元素的值拷贝（根据比较函数定义的顺序）。                                                                                                        \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return TYPE 返回最小元素的值拷贝                                                                                                                  \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 集合不能为空，否则行为未定义                                                                                                                   \
     * @note 使用前应先检查 !empty()                                                                                                                   \
     * @note 返回值拷贝，对于大对象可考虑使用 front_ptr()                                                                                                    \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 30);                                                                                                                \
     * cset_int_insert(set, 10);                                                                                                                \
     * cset_int_insert(set, 20);                                                                                                                \
     * if (!cset_int_empty(set)) {                                                                                                              \
     *     printf("最小元素: %d\n", cset_int_front(set));  // 输出: 10                                                                                \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE TYPE cset_##NAME##_front(cset_##NAME *set)                                                                                    \
    {                                                                                                                                           \
        return *((TYPE *)__cset_t_front((__cset_t *)set, sizeof(cset_##NAME##_node), sizeof(TYPE)));                                            \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合中的最大元素（值拷贝）                                                                                                                 \
     *                                                                                                                                           \
     * 返回集合中最大元素的值拷贝（根据比较函数定义的顺序）。                                                                                                        \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return TYPE 返回最大元素的值拷贝                                                                                                                  \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 集合不能为空，否则行为未定义                                                                                                                   \
     * @note 使用前应先检查 !empty()                                                                                                                   \
     * @note 返回值拷贝，对于大对象可考虑使用 back_ptr()                                                                                                     \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 30);                                                                                                                \
     * cset_int_insert(set, 10);                                                                                                                \
     * cset_int_insert(set, 20);                                                                                                                \
     * if (!cset_int_empty(set)) {                                                                                                              \
     *     printf("最大元素: %d\n", cset_int_back(set));  // 输出: 30                                                                                 \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE TYPE cset_##NAME##_back(cset_##NAME *set)                                                                                     \
    {                                                                                                                                           \
        return *((TYPE *)__cset_t_back((__cset_t *)set, sizeof(cset_##NAME##_node), sizeof(TYPE)));                                             \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合中最小元素的指针                                                                                                                     \
     *                                                                                                                                           \
     * 返回指向集合中最小元素的 const 指针，避免值拷贝开销。                                                                                                      \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return const TYPE* 返回指向最小元素的 const 指针                                                                                                  \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 返回的是 const 指针，不应修改指向的内容                                                                                                          \
     * @note 集合不能为空，否则行为未定义                                                                                                                   \
     * @note 指针在集合修改（插入/删除）后可能失效                                                                                                             \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 100);                                                                                                               \
     * if (!cset_int_empty(set)) {                                                                                                              \
     *     const int *ptr = cset_int_front_ptr(set);                                                                                            \
     *     printf("最小值: %d\n", *ptr);                                                                                                           \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE const TYPE *cset_##NAME##_front_ptr(cset_##NAME *set)                                                                               \
    {                                                                                                                                           \
        return ((const TYPE *)__cset_t_front((__cset_t *)set, sizeof(cset_##NAME##_node), sizeof(TYPE)));                                             \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 获取集合中最大元素的指针                                                                                                                     \
     *                                                                                                                                           \
     * 返回指向集合中最大元素的 const 指针，避免值拷贝开销。                                                                                                      \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return const TYPE* 返回指向最大元素的 const 指针                                                                                                  \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 返回的是 const 指针，不应修改指向的内容                                                                                                          \
     * @note 集合不能为空，否则行为未定义                                                                                                                   \
     * @note 指针在集合修改（插入/删除）后可能失效                                                                                                             \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 999);                                                                                                               \
     * if (!cset_int_empty(set)) {                                                                                                              \
     *     const int *ptr = cset_int_back_ptr(set);                                                                                             \
     *     printf("最大值: %d\n", *ptr);                                                                                                           \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE const TYPE *cset_##NAME##_back_ptr(cset_##NAME *set)                                                                                \
    {                                                                                                                                           \
        return ((const TYPE *)__cset_t_back((__cset_t *)set, sizeof(cset_##NAME##_node), sizeof(TYPE)));                                              \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 反初始化集合对象，释放内部资源                                                                                                                  \
     *                                                                                                                                           \
     * 清理通过 init 或 init_with_cmp 初始化的集合对象的内部资源，                                                                                             \
     * 但不释放集合对象本身的内存。                                                                                                                         \
     *                                                                                                                                           \
     * @param set 集合对象指针                                                                                                                       \
     *                                                                                                                                           \
     * @return void 无返回值                                                                                                                       \
     *                                                                                                                                           \
     * @note 此函数与 init 配对使用，用于栈上分配的集合                                                                                                        \
     * @note 与 destroy 不同，此函数不释放集合对象本身                                                                                                        \
     * @note 调用后集合对象不应再使用，除非重新 init                                                                                                          \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int set;  // 栈上分配                                                                                                                 \
     * cset_int_init(&set);                                                                                                                     \
     * cset_int_insert(&set, 1);                                                                                                                \
     * cset_int_insert(&set, 2);                                                                                                                \
     * cset_int_uninit(&set);  // 清理内部资源                                                                                                        \
     * // set 对象本身还在（栈上），但不应再使用                                                                                                              \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE void cset_##NAME##_uninit(cset_##NAME *set)                                                                                   \
    {                                                                                                                                           \
        __cset_uninit((__cset_t *)set);                                                                                                         \
    }                                                                                                                                           \
    /**                                                                                                                                         \
     * @brief 查找第一个不小于给定值的元素                                                                                                                   \
     *                                                                                                                                           \
     * 返回指向第一个不小于（大于或等于）给定值的元素节点。                                                                                                         \
     * 如果所有元素都小于给定值，返回 end()。                                                                                                                \
     *                                                                                                                                           \
     * @param set   集合对象指针                                                                                                                     \
     * @param value 用于比较的值                                                                                                                      \
     *                                                                                                                                           \
     * @return cset_##NAME##_node* 返回第一个 >= value 的节点，不存在则返回 end()                                                                            \
     *                                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                                   \
     * @note 类似 C++ STL 的 lower_bound                                                                                                           \
     * @note 可用于范围查询的起始位置                                                                                                                      \
     *                                                                                                                                           \
     * @code                                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                                       \
     * cset_int_insert(set, 10);                                                                                                                \
     * cset_int_insert(set, 20);                                                                                                                \
     * cset_int_insert(set, 30);                                                                                                                \
     * cset_int_node *node = cset_int_lower_bound(set, 15);                                                                                     \
     * if (node != cset_int_end(set)) {                                                                                                         \
     *     printf("第一个 >= 15 的元素: %d\n", cset_int_get_value(node));  // 输出: 20                                                                  \
     * }                                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                                  \
     * @endcode                                                                                                                                 \
     */                                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_lower_bound(cset_##NAME *set, const TYPE value)                            \
    {                                                                                                                           \
        return (cset_##NAME##_node *)(__cset_lower_bound((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE))); \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 查找第一个大于给定值的元素                                                                                                \
     *                                                                                                                           \
     * 返回指向第一个严格大于给定值的元素节点。                                                                                            \
     * 如果所有元素都小于或等于给定值，返回 end()。                                                                                       \
     *                                                                                                                           \
     * @param set   集合对象指针                                                                                                     \
     * @param value 用于比较的值                                                                                                      \
     *                                                                                                                           \
     * @return cset_##NAME##_node* 返回第一个 > value 的节点，不存在则返回 end()                                                             \
     *                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                   \
     * @note 类似 C++ STL 的 upper_bound                                                                                           \
     * @note 可用于范围查询的结束位置                                                                                                      \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                       \
     * cset_int_insert(set, 10);                                                                                                \
     * cset_int_insert(set, 20);                                                                                                \
     * cset_int_insert(set, 30);                                                                                                \
     * cset_int_node *node = cset_int_upper_bound(set, 20);                                                                     \
     * if (node != cset_int_end(set)) {                                                                                         \
     *     printf("第一个 > 20 的元素: %d\n", cset_int_get_value(node));  // 输出: 30                                                   \
     * }                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                  \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE cset_##NAME##_node *cset_##NAME##_upper_bound(cset_##NAME *set, const TYPE value)                            \
    {                                                                                                                           \
        return (cset_##NAME##_node *)(__cset_upper_bound((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE))); \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 统计集合中等于给定值的元素数量                                                                                                  \
     *                                                                                                                           \
     * 由于 set 中元素唯一，返回值只能是 0 或 1。                                                                                            \
     *                                                                                                                           \
     * @param set   集合对象指针                                                                                                     \
     * @param value 要统计的值                                                                                                        \
     *                                                                                                                           \
     * @return size_t 元素存在返回 1，不存在返回 0                                                                                         \
     *                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                   \
     * @note 对于 set，等价于 contains()，但返回类型不同                                                                                      \
     * @note 对于 multiset（如果实现），可能返回 > 1 的值                                                                                     \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                       \
     * cset_int_insert(set, 42);                                                                                                \
     * printf("42 的数量: %zu\n", cset_int_count(set, 42));  // 输出: 1                                                             \
     * printf("99 的数量: %zu\n", cset_int_count(set, 99));  // 输出: 0                                                             \
     * cset_int_destroy(&set);                                                                                                  \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE size_t cset_##NAME##_count(cset_##NAME *set, const TYPE value)                                               \
    {                                                                                                                           \
        return __cset_count((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE));                  \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 交换两个集合的内容                                                                                                         \
     *                                                                                                                           \
     * 高效地交换两个集合的所有元素和状态。                                                                                                    \
     *                                                                                                                           \
     * @param set1 第一个集合对象指针                                                                                                    \
     * @param set2 第二个集合对象指针                                                                                                    \
     *                                                                                                                           \
     * @return void 无返回值                                                                                                       \
     *                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                       \
     * @note 仅交换内部指针，不涉及元素拷贝，非常高效                                                                                             \
     * @note 两个集合必须是相同类型                                                                                                        \
     * @note 交换后迭代器/指针仍然有效，但指向了不同的集合                                                                                          \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set1 = cset_int_create();                                                                                      \
     * cset_int *set2 = cset_int_create();                                                                                      \
     * cset_int_insert(set1, 1);                                                                                                \
     * cset_int_insert(set2, 100);                                                                                              \
     * printf("交换前 set1: %d\n", cset_int_front(set1));  // 输出: 1                                                               \
     * cset_int_swap(set1, set2);                                                                                               \
     * printf("交换后 set1: %d\n", cset_int_front(set1));  // 输出: 100                                                             \
     * cset_int_destroy(&set1);                                                                                                 \
     * cset_int_destroy(&set2);                                                                                                 \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE void cset_##NAME##_swap(cset_##NAME *set1, cset_##NAME *set2)                                                \
    {                                                                                                                           \
        __cset_swap((__cset_t *)set1, (__cset_t *)set2);                                                                        \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 获取等于给定值的元素范围                                                                                                      \
     *                                                                                                                           \
     * 返回一对迭代器，表示所有等于给定值的元素范围 [first, second)。                                                                               \
     * 对于 set，由于元素唯一，范围最多包含一个元素。                                                                                            \
     *                                                                                                                           \
     * @param set    集合对象指针                                                                                                    \
     * @param value  要查找的值                                                                                                       \
     * @param first  输出参数，指向范围起始位置的指针（>= value）                                                                                \
     * @param second 输出参数，指向范围结束位置的指针（> value）                                                                                 \
     *                                                                                                                           \
     * @return void 无返回值，通过 first 和 second 参数返回结果                                                                              \
     *                                                                                                                           \
     * @note 时间复杂度: O(log n)                                                                                                   \
     * @note 等价于 [lower_bound(value), upper_bound(value))                                                                      \
     * @note 如果元素不存在，first == second                                                                                           \
     * @note 如果元素存在，first 指向该元素，second 指向下一个元素                                                                                \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                       \
     * cset_int_insert(set, 10);                                                                                                \
     * cset_int_insert(set, 20);                                                                                                \
     * cset_int_insert(set, 30);                                                                                                \
     * cset_int_node *first, *second;                                                                                           \
     * cset_int_equal_range(set, 20, &first, &second);                                                                          \
     * if (first != second) {                                                                                                   \
     *     printf("找到 20: %d\n", cset_int_get_value(first));                                                                    \
     * }                                                                                                                        \
     * cset_int_destroy(&set);                                                                                                  \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE void cset_##NAME##_equal_range(cset_##NAME *set, const TYPE value, cset_##NAME##_node **first, cset_##NAME##_node **second) \
    {                                                                                                                           \
        __cset_equal_range((__cset_t *)set, (const char *)&value, sizeof(cset_##NAME##_node), sizeof(TYPE),                    \
                          (__cset_node_t **)first, (__cset_node_t **)second);                                                   \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 获取集合的最大容量                                                                                                         \
     *                                                                                                                           \
     * 返回理论上集合可以容纳的最大元素数量。                                                                                                   \
     *                                                                                                                           \
     * @return size_t 返回最大容量值                                                                                                   \
     *                                                                                                                           \
     * @note 实际可用容量受限于系统内存                                                                                                      \
     * @note 这是一个理论值，通常非常大                                                                                                      \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * printf("集合最大容量: %zu\n", cset_int_max_size());                                                                           \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE size_t cset_##NAME##_max_size()                                                                               \
    {                                                                                                                           \
        return __cset_max_size();                                                                                               \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 获取集合使用的键比较函数                                                                                                      \
     *                                                                                                                           \
     * 返回集合创建时设置的比较函数指针。                                                                                                       \
     *                                                                                                                           \
     * @param set 集合对象指针                                                                                                       \
     *                                                                                                                           \
     * @return cset_##NAME##_cmp_cb 返回比较函数指针                                                                                     \
     *                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                       \
     * @note 可用于判断集合使用的排序规则                                                                                                     \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                       \
     * cset_int_cmp_cb cmp = cset_int_key_comp(set);                                                                            \
     * // 可以使用 cmp 函数进行自定义比较                                                                                                   \
     * cset_int_destroy(&set);                                                                                                  \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE cset_##NAME##_cmp_cb cset_##NAME##_key_comp(cset_##NAME *set)                                                \
    {                                                                                                                           \
        return (cset_##NAME##_cmp_cb)__cset_key_comp((__cset_t *)set);                                                          \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 获取集合使用的值比较函数                                                                                                      \
     *                                                                                                                           \
     * 对于 set，值比较函数与键比较函数相同。                                                                                                 \
     * 返回集合创建时设置的比较函数指针。                                                                                                       \
     *                                                                                                                           \
     * @param set 集合对象指针                                                                                                       \
     *                                                                                                                           \
     * @return cset_##NAME##_cmp_cb 返回比较函数指针                                                                                     \
     *                                                                                                                           \
     * @note 时间复杂度: O(1)                                                                                                       \
     * @note 与 key_comp() 功能相同                                                                                                  \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set = cset_int_create();                                                                                       \
     * cset_int_cmp_cb cmp = cset_int_value_comp(set);                                                                          \
     * cset_int_destroy(&set);                                                                                                  \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE cset_##NAME##_cmp_cb cset_##NAME##_value_comp(cset_##NAME *set)                                              \
    {                                                                                                                           \
        return (cset_##NAME##_cmp_cb)__cset_key_comp((__cset_t *)set);                                                          \
    }                                                                                                                           \
    /**                                                                                                                         \
     * @brief 合并两个集合                                                                                                             \
     *                                                                                                                           \
     * 将源集合中的所有元素移动到目标集合中，源集合中已存在于目标集合的元素保持不变。                                                                              \
     * 操作后，源集合中只保留那些在目标集合中已存在的元素。                                                                                             \
     *                                                                                                                           \
     * @param dst 目标集合对象指针                                                                                                      \
     * @param src 源集合对象指针                                                                                                        \
     *                                                                                                                           \
     * @return int 成功返回 0，失败返回非 0 值                                                                                            \
     *                                                                                                                           \
     * @note 时间复杂度: O(n log(n+m))，其中 n 和 m 分别是两个集合的大小                                                                          \
     * @note 两个集合必须使用相同的比较函数                                                                                                    \
     * @note 移动操作不涉及元素拷贝，直接转移节点                                                                                                 \
     * @note 操作后源集合可能变小但不一定为空                                                                                                   \
     *                                                                                                                           \
     * @code                                                                                                                    \
     * cset_int *set1 = cset_int_create();                                                                                      \
     * cset_int *set2 = cset_int_create();                                                                                      \
     * cset_int_insert(set1, 1);                                                                                                \
     * cset_int_insert(set1, 2);                                                                                                \
     * cset_int_insert(set2, 2);  // 重复元素                                                                                       \
     * cset_int_insert(set2, 3);                                                                                                \
     * cset_int_merge(set1, set2);                                                                                              \
     * // set1 包含: 1, 2, 3                                                                                                     \
     * // set2 包含: 2 (因为 1 中已有 2)                                                                                             \
     * cset_int_destroy(&set1);                                                                                                 \
     * cset_int_destroy(&set2);                                                                                                 \
     * @endcode                                                                                                                 \
     */                                                                                                                         \
    static INLINE int cset_##NAME##_merge(cset_##NAME *dst, cset_##NAME *src)                                                  \
    {                                                                                                                           \
        return __cset_merge((__cset_t *)dst, (__cset_t *)src, sizeof(cset_##NAME##_node), sizeof(TYPE));                       \
    }
/**
 * @brief 正向遍历集合的便捷宏（类型名与别名相同）
 * 
 * 此宏用于正向遍历集合中的所有元素，自动处理迭代器逻辑。
 * 遍历顺序为从小到大（按照比较函数定义的顺序）。
 * 
 * @param set_ptr   集合对象指针
 * @param item_type 元素类型
 * @param item_name 循环变量名，用于访问当前元素
 * 
 * @note 在循环体内，item_name 是当前元素的值（拷贝）
 * @note 可以安全地在循环中删除当前元素（使用 erase）
 * @note 自动生成 item_name##_index 变量表示当前索引（从 0 开始）
 * @note 自动生成 item_name##_iter 变量表示当前迭代器节点
 * @note 不要在循环中对集合进行大量插入操作，可能导致迭代器失效
 * 
 * @code
 * DEFINE_CSET(int);
 * cset_int *set = cset_int_create();
 * cset_int_insert(set, 3);
 * cset_int_insert(set, 1);
 * cset_int_insert(set, 2);
 * 
 * cset_foreach(set, int, num) {
 *     printf("[%zu] = %d\n", num_index, num);  // 输出: 1, 2, 3
 *     if (num == 2) {
 *         cset_int_erase(set, num_iter);  // 可以安全删除
 *     }
 * }
 * cset_int_destroy(&set);
 * @endcode
 */
#define cset_foreach(set_ptr, item_type, item_name) cset_as_foreach(set_ptr, item_type, item_name, item_type)

/**
 * @brief 正向遍历集合的通用宏（支持自定义类型别名）
 * 
 * 此宏用于正向遍历集合中的所有元素，支持自定义类型别名。
 * 当元素类型与集合定义名称不同时使用此宏。
 * 
 * @param set_ptr   集合对象指针
 * @param item_type 元素类型（实际的 C 类型）
 * @param item_name 循环变量名
 * @param alias     集合定义时使用的别名（DEFINE_CSET_AS 的 NAME 参数）
 * 
 * @note 用于处理 DEFINE_CSET_AS 或 DEFINE_CSET_PTR_AS 定义的集合
 * @note 其他特性与 cset_foreach 相同
 * 
 * @code
 * DEFINE_CSET_AS(char*, str_set);
 * cset_str_set *set = cset_str_set_create();
 * cset_str_set_insert(set, "apple");
 * cset_str_set_insert(set, "banana");
 * 
 * cset_as_foreach(set, char*, str, str_set) {
 *     printf("%s\n", str);
 * }
 * cset_str_set_destroy(&set);
 * @endcode
 */
#define cset_as_foreach(set_ptr, item_type, item_name, alias)                                                                    \
    for (int item_name##_scope_ = 1; item_name##_scope_; item_name##_scope_ = 0)                                                \
        for (item_type item_name = (item_type){0}; item_name##_scope_; item_name##_scope_ = 0)                                  \
            for (size_t item_name##_index = 0; item_name##_scope_; item_name##_scope_ = 0)                                      \
                for (cset_##alias##_node *item_name##_iter = 0, *__##item_name##_next_iter = 0; item_name##_scope_; item_name##_scope_ = 0) \
                    if (!cset_##alias##_empty((set_ptr)) ?                                                                      \
                        (item_name##_iter = cset_##alias##_begin((set_ptr)),                                                    \
                         item_name = cset_##alias##_front((set_ptr)),                                                           \
                         __##item_name##_next_iter = cset_##alias##_next((set_ptr), item_name##_iter), 1) : 0)                 \
                        for (; __cset_foreach_check((__cset_t *)(set_ptr), (__cset_node_t *)item_name##_iter,                  \
                                                    (char *)&item_name, sizeof(struct cset_##alias##_node), sizeof(item_type)) != 0; \
                             ++item_name##_index, item_name##_iter = __##item_name##_next_iter,                                \
                             __##item_name##_next_iter = cset_##alias##_next((set_ptr), __##item_name##_next_iter))

/**
 * @brief 反向遍历集合的便捷宏（类型名与别名相同）
 * 
 * 此宏用于反向遍历集合中的所有元素，自动处理迭代器逻辑。
 * 遍历顺序为从大到小（按照比较函数定义的逆序）。
 * 
 * @param set_ptr   集合对象指针
 * @param item_type 元素类型
 * @param item_name 循环变量名，用于访问当前元素
 * 
 * @note 在循环体内，item_name 是当前元素的值（拷贝）
 * @note 可以安全地在循环中删除当前元素（使用 erase）
 * @note 自动生成 item_name##_index 变量表示当前索引（从 size-1 递减到 0）
 * @note 自动生成 item_name##_iter 变量表示当前迭代器节点
 * @note 遍历方向与 cset_foreach 相反
 * 
 * @code
 * DEFINE_CSET(int);
 * cset_int *set = cset_int_create();
 * cset_int_insert(set, 1);
 * cset_int_insert(set, 2);
 * cset_int_insert(set, 3);
 * 
 * cset_foreach_reverse(set, int, num) {
 *     printf("[%zu] = %d\n", num_index, num);  // 输出: 3, 2, 1
 *     if (num == 2) {
 *         cset_int_erase(set, num_iter);  // 可以安全删除
 *     }
 * }
 * cset_int_destroy(&set);
 * @endcode
 */
#define cset_foreach_reverse(set_ptr, item_type, item_name) cset_as_foreach_reverse(set_ptr, item_type, item_name, item_type)

/**
 * @brief 反向遍历集合的通用宏（支持自定义类型别名）
 * 
 * 此宏用于反向遍历集合中的所有元素，支持自定义类型别名。
 * 当元素类型与集合定义名称不同时使用此宏。
 * 
 * @param set_ptr   集合对象指针
 * @param item_type 元素类型（实际的 C 类型）
 * @param item_name 循环变量名
 * @param alias     集合定义时使用的别名（DEFINE_CSET_AS 的 NAME 参数）
 * 
 * @note 用于处理 DEFINE_CSET_AS 或 DEFINE_CSET_PTR_AS 定义的集合
 * @note 遍历顺序从大到小
 * @note 其他特性与 cset_foreach_reverse 相同
 * 
 * @code
 * DEFINE_CSET_AS(double, real_set);
 * cset_real_set *set = cset_real_set_create();
 * cset_real_set_insert(set, 1.1);
 * cset_real_set_insert(set, 2.2);
 * cset_real_set_insert(set, 3.3);
 * 
 * cset_as_foreach_reverse(set, double, val, real_set) {
 *     printf("%.1f\n", val);  // 输出: 3.3, 2.2, 1.1
 * }
 * cset_real_set_destroy(&set);
 * @endcode
 */
#define cset_as_foreach_reverse(set_ptr, item_type, item_name, alias)                                                                       \
    for (int item_name##_scope_ = 1; item_name##_scope_; item_name##_scope_ = 0)                                                                \
        for (item_type item_name = (item_type){0}; item_name##_scope_; item_name##_scope_ = 0)                                                  \
            for (size_t item_name##_index = cset_##alias##_size((set_ptr)); item_name##_scope_; item_name##_scope_ = 0)                         \
                for (cset_##alias##_node *item_name##_iter = 0, *__##item_name##_next_iter = 0; item_name##_scope_; item_name##_scope_ = 0)     \
                    if (!cset_##alias##_empty((set_ptr)) ?                                                                                      \
                        (item_name##_iter = cset_##alias##_prev((set_ptr), cset_##alias##_end((set_ptr))),                                      \
                         item_name = cset_##alias##_back((set_ptr)),                                                                            \
                         __##item_name##_next_iter = cset_##alias##_prev((set_ptr), item_name##_iter), 1) : 0)                                  \
                        for (--item_name##_index; __cset_foreach_reverse_check((__cset_t *)(set_ptr), (__cset_node_t *)item_name##_iter,       \
                                                                               (char *)&item_name, sizeof(struct cset_##alias##_node), sizeof(item_type)) != 0; \
                             --item_name##_index, item_name##_iter = __##item_name##_next_iter,                                                 \
                             __##item_name##_next_iter = cset_##alias##_prev((set_ptr), __##item_name##_next_iter))

    // 注意：以下部分是库内部函数，用户不要调用-------------------

    typedef struct __cset_t __cset_t;
    typedef struct __cset_node_t __cset_node_t;

    typedef int (*__cset_t_cmp_cb)(const char *lhs, const char *rhs);

    CC_API __cset_t *CC_CALL __cset_create(__cset_t_cmp_cb cmp_cb, size_t set_size, size_t node_size, size_t item_size);
    CC_API int CC_CALL __cset_init(__cset_t *s, __cset_t_cmp_cb cmp_cb, size_t set_size, size_t node_size, size_t item_size);
    CC_API void CC_CALL __cset_uninit(__cset_t *s);
    CC_API void CC_CALL __cset_destroy(__cset_t *map);
    CC_API int CC_CALL __cset_insert(__cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API size_t CC_CALL __cset_size(const __cset_t *s);
    CC_API int CC_CALL __cset_empty(const __cset_t *s);
    CC_API __cset_node_t *CC_CALL __cset_find(const __cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API __cset_node_t *CC_CALL __cset_begin(const __cset_t *s);
    CC_API __cset_node_t *CC_CALL __cset_end(const __cset_t *s);
    CC_API __cset_node_t *CC_CALL __cset_next(__cset_t *s, __cset_node_t *n);
    CC_API __cset_node_t *CC_CALL __cset_prev(__cset_t *s, __cset_node_t *n);
    CC_API int CC_CALL __cset_contains(__cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API char *CC_CALL __cset_get_value(__cset_node_t *node, size_t node_size, size_t item_size);
    CC_API int CC_CALL __cset_remove(__cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API int CC_CALL __cset_erase(__cset_t *s, __cset_node_t *node);
    CC_API void CC_CALL __cset_clear(__cset_t *s);
    CC_API char *CC_CALL __cset_t_front(__cset_t *cset, size_t node_size, size_t item_size);
    CC_API char *CC_CALL __cset_t_back(__cset_t *cset, size_t node_size, size_t item_size);
    CC_API int CC_CALL __cset_foreach_check(__cset_t *cset, __cset_node_t *iter, char *item_ptr, size_t node_size, size_t item_size);
    CC_API int CC_CALL __cset_foreach_reverse_check(__cset_t *cset, __cset_node_t *iter, char *item_ptr, size_t node_size, size_t item_size);
    CC_API __cset_node_t *CC_CALL __cset_lower_bound(const __cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API __cset_node_t *CC_CALL __cset_upper_bound(const __cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API size_t CC_CALL __cset_count(const __cset_t *s, const char *value, size_t node_size, size_t item_size);
    CC_API void CC_CALL __cset_swap(__cset_t *s1, __cset_t *s2);
    CC_API void CC_CALL __cset_equal_range(const __cset_t *s, const char *value, size_t node_size, size_t item_size,
                                           __cset_node_t **first, __cset_node_t **second);
    CC_API size_t CC_CALL __cset_max_size();
    CC_API __cset_t_cmp_cb CC_CALL __cset_key_comp(const __cset_t *s);
    CC_API int CC_CALL __cset_merge(__cset_t *dst, __cset_t *src, size_t node_size, size_t item_size);

#ifdef __cplusplus
}
#endif

#endif

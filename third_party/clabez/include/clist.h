#ifndef C_CORE_LIST_H_
#define C_CORE_LIST_H_

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

/**
 * @brief 定义一个类型安全的双向链表
 * 
 * 此宏根据给定的类型生成一个完整的链表实现，包括所有相关的函数和类型定义。
 * 生成的链表名称为 clist_TYPE。
 * 
 * @param TYPE 链表元素的类型（如 int, double, struct MyStruct 等）
 * 
 * @par 简单用法:
 * @code
 * // 定义整数链表
 * DEFINE_CLIST(int)
 * 
 * // 使用生成的链表
 * clist_int *list = clist_int_create();
 * clist_int_push_back(list, 10);
 * clist_int_destroy(&list);
 * @endcode
 * 
 * @note 此宏应在全局作用域使用，通常放在头文件中
 * @note 对于同一类型，只能定义一次，否则会导致重复定义错误
 */
#define DEFINE_CLIST(TYPE) DEFINE_CLIST_AS(TYPE, TYPE)

/**
 * @brief 定义一个指针类型的双向链表
 * 
 * 此宏生成一个存储指针的链表实现，生成的链表名称为 clist_TYPE_ptr。
 * 适用于需要存储指针而非值的场景。
 * 
 * @param TYPE 指针指向的类型（生成的链表实际存储 TYPE* 类型）
 * 
 * @par 简单用法:
 * @code
 * // 定义指针链表
 * DEFINE_CLIST_PTR(MyStruct)
 * 
 * // 使用生成的指针链表
 * clist_MyStruct_ptr *list = clist_MyStruct_ptr_create();
 * MyStruct *obj = malloc(sizeof(MyStruct));
 * clist_MyStruct_ptr_push_back(list, obj);
 * clist_MyStruct_ptr_destroy(&list);
 * @endcode
 * 
 * @note 链表不负责指针指向对象的内存管理，需要用户自行管理
 */
#define DEFINE_CLIST_PTR(TYPE) DEFINE_CLIST_AS(TYPE *, TYPE##_ptr)

/**
 * @brief 定义一个指针类型的双向链表，并指定自定义名称
 * 
 * 与 DEFINE_CLIST_PTR 类似，但允许自定义生成的链表类型名称。
 * 
 * @param TYPE 指针指向的类型
 * @param NAME 自定义的链表名称（生成 clist_NAME）
 * 
 * @par 简单用法:
 * @code
 * // 定义自定义名称的指针链表
 * DEFINE_CLIST_PTR_AS(MyStruct, my_list)
 * 
 * // 使用生成的链表
 * clist_my_list *list = clist_my_list_create();
 * @endcode
 * 
 * @note 适用于需要避免命名冲突或使用更语义化名称的场景
 */
#define DEFINE_CLIST_PTR_AS(TYPE, NAME) DEFINE_CLIST_AS(TYPE *, NAME)

/**
 * @brief 定义一个自定义名称的双向链表
 * 
 * 这是所有 DEFINE_CLIST 宏的基础实现。根据指定的类型和名称生成完整的链表实现。
 * 生成的链表支持所有标准的双向链表操作。
 * 
 * @param TYPE 链表元素的类型
 * @param NAME 链表的名称标识符
 * 
 * @par 生成的类型:
 * - clist_NAME: 链表结构体
 * - clist_NAME_node: 链表节点结构体
 * - clist_NAME_iter: 迭代器类型（实际是节点指针）
 * - clist_NAME_cmp_cb: 比较函数回调类型
 * - clist_NAME_pred_cb: 断言函数回调类型
 * 
 * @par 简单用法:
 * @code
 * // 定义一个名为 myint 的整数链表
 * DEFINE_CLIST_AS(int, myint)
 * 
 * // 使用生成的链表
 * clist_myint *list = clist_myint_create();
 * clist_myint_push_back(list, 42);
 * int value = clist_myint_front(list);
 * clist_myint_destroy(&list);
 * @endcode
 * 
 * @note 此宏会生成数十个相关函数，请参阅下面的具体函数文档
 * @note 确保 TYPE 的大小在编译时已知（不能是不完整类型）
 */
#define DEFINE_CLIST_AS(TYPE, NAME)                                                                        \
    typedef struct clist_##NAME##_node                                                                     \
    {                                                                                                      \
        TYPE *value_ptr;                                                                                   \
        struct clist_##NAME##_node *next;                                                                  \
        struct clist_##NAME##_node *prev;                                                                  \
    } clist_##NAME##_node;                                                                                 \
                                                                                                           \
    typedef clist_##NAME##_node *clist_##NAME##_iter;                                                      \
    typedef int (*clist_##NAME##_cmp_cb)(const TYPE *lhs, const TYPE *rhs);                                \
    typedef struct clist_##NAME                                                                            \
    {                                                                                                      \
        unsigned int size;                                                                                 \
        clist_##NAME##_node *head;                                                                         \
        clist_##NAME##_node *tail;                                                                         \
    } clist_##NAME;                                                                                        \
                                                                                                           \
    /**                                                                                                    \
     * @brief 创建一个新的链表                                                                               \
     *                                                                                                     \
     * 在堆上分配并初始化一个新的空链表。                                                                       \
     *                                                                                                     \
     * @return 返回新创建的链表指针，失败返回 NULL                                                               \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * if (list) {                                                                                        \
     *     // 使用链表                                                                                       \
     *     clist_int_destroy(&list);                                                                       \
     * }                                                                                                  \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 使用完毕后必须调用 clist_##NAME##_destroy 释放内存                                                 \
     * @note 创建失败时返回 NULL，调用者应检查返回值                                                              \
     */                                                                                                    \
    static INLINE clist_##NAME *clist_##NAME##_create()                                                    \
    {                                                                                                      \
        return (clist_##NAME *)__clist_t_create(sizeof(clist_##NAME), sizeof(clist_##NAME##_node));        \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 销毁链表并释放所有内存                                                                           \
     *                                                                                                     \
     * 释放链表及其所有节点占用的内存，并将链表指针设置为 NULL。                                                      \
     *                                                                                                     \
     * @param pclist 指向链表指针的指针（二级指针）                                                               \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_destroy(&list);  // 销毁后 list 变为 NULL                                                   \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 如果 pclist 为 NULL 或 *pclist 为 NULL，函数安全返回，不执行任何操作                                    \
     * @note 销毁后，原指针会被设置为 NULL，防止悬空指针                                                            \
     * @note 如果链表元素是指针类型，此函数不会释放指针指向的内存，需要用户自行管理                                        \
     */                                                                                                    \
    static INLINE void clist_##NAME##_destroy(clist_##NAME **pclist)                                       \
    {                                                                                                      \
        if (pclist && *pclist)                                                                             \
        {                                                                                                  \
            __clist_t_destroy((__clist_t *)(*pclist));                                                     \
            *pclist = 0;                                                                                   \
        }                                                                                                  \
    }                                                                                                      \
    /**                                                                                                    \
     * @brief 初始化一个栈上分配的链表                                                                          \
     *                                                                                                     \
     * 初始化一个已经分配（如在栈上或作为结构体成员）的链表结构。                                                       \
     *                                                                                                     \
     * @param clist 指向要初始化的链表的指针                                                                     \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int list;                                                                                    \
     * if (clist_int_init(&list) == 0) {                                                                  \
     *     clist_int_push_back(&list, 10);                                                                \
     *     clist_int_uninit(&list);  // 使用完毕后必须反初始化                                                  \
     * }                                                                                                  \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 对于栈上分配的链表，使用完毕后必须调用 clist_##NAME##_uninit                                          \
     * @note 不要对同一个链表调用多次 init，除非先调用了 uninit                                                     \
     */                                                                                                    \
    static INLINE int clist_##NAME##_init(clist_##NAME *clist)                                             \
    {                                                                                                      \
        return __clist_t_init((__clist_t *)clist, sizeof(clist_##NAME), sizeof(clist_##NAME##_node));      \
    }                                                                                                      \
    /**                                                                                                    \
     * @brief 反初始化链表并释放其内部资源                                                                        \
     *                                                                                                     \
     * 释放链表的所有节点和内部资源，但不释放链表结构本身（适用于栈上分配的链表）。                                          \
     *                                                                                                     \
     * @param clist 指向要反初始化的链表的指针                                                                    \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int list;                                                                                    \
     * clist_int_init(&list);                                                                             \
     * clist_int_push_back(&list, 10);                                                                    \
     * clist_int_uninit(&list);  // 必须调用以释放内部资源                                                      \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 此函数与 clist_##NAME##_init 配对使用                                                             \
     * @note 调用后，链表结构本身仍然存在，但内部数据已释放                                                           \
     */                                                                                                    \
    static INLINE void clist_##NAME##_uninit(clist_##NAME *clist)                                          \
    {                                                                                                      \
        __clist_t_uninit((__clist_t *)clist);                                                              \
    }                                                                                                      \
    /**                                                                                                    \
     * @brief 获取链表中元素的数量                                                                             \
     *                                                                                                     \
     * 返回链表当前包含的元素个数。                                                                              \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 链表中的元素数量                                                                               \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * unsigned int size = clist_int_size(list);  // size = 2                                            \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     */                                                                                                    \
    static INLINE unsigned int clist_##NAME##_size(const clist_##NAME *clist)                              \
    {                                                                                                      \
        return __clist_t_size((__clist_t *)clist);                                                         \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 检查链表是否为空                                                                                \
     *                                                                                                     \
     * 判断链表是否不包含任何元素。                                                                               \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 如果链表为空返回非 0 值，否则返回 0                                                                 \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * if (clist_int_empty(list)) {                                                                       \
     *     printf("List is empty\n");                                                                     \
     * }                                                                                                  \
     * clist_int_push_back(list, 10);                                                                     \
     * if (!clist_int_empty(list)) {                                                                      \
     *     printf("List is not empty\n");                                                                 \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     */                                                                                                    \
    static INLINE int clist_##NAME##_empty(const clist_##NAME *clist)                                      \
    {                                                                                                      \
        return __clist_t_empty((__clist_t *)clist);                                                        \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 在链表尾部添加元素                                                                               \
     *                                                                                                     \
     * 在链表的末尾插入一个新元素。元素的值会被复制到链表中。                                                           \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param value 要添加的元素值                                                                            \
     * @return 成功返回 0，失败返回非 0 值（通常是内存分配失败）                                                       \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 30);                                                                     \
     * // 链表现在包含: 10 -> 20 -> 30                                                                      \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 失败时链表保持不变                                                                                \
     */                                                                                                    \
    static INLINE int clist_##NAME##_push_back(clist_##NAME *clist, TYPE value)                            \
    {                                                                                                      \
        return __clist_t_push_back((__clist_t *)clist, (const char *)&value, sizeof(TYPE));                \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 在链表头部添加元素                                                                               \
     *                                                                                                     \
     * 在链表的开头插入一个新元素。元素的值会被复制到链表中。                                                           \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param value 要添加的元素值                                                                            \
     * @return 成功返回 0，失败返回非 0 值（通常是内存分配失败）                                                       \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_front(list, 30);                                                                    \
     * clist_int_push_front(list, 20);                                                                    \
     * clist_int_push_front(list, 10);                                                                    \
     * // 链表现在包含: 10 -> 20 -> 30                                                                      \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 失败时链表保持不变                                                                                \
     */                                                                                                    \
    static INLINE int clist_##NAME##_push_front(clist_##NAME *clist, TYPE value)                           \
    {                                                                                                      \
        return __clist_t_push_front((__clist_t *)clist, (const char *)&value, sizeof(TYPE));               \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 在指定位置之前插入元素                                                                            \
     *                                                                                                     \
     * 在迭代器指定的位置之前插入一个新元素。                                                                       \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param iter 迭代器（节点指针），新元素将插入到此节点之前                                                        \
     * @param value 要插入的元素值                                                                            \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 30);                                                                     \
     * clist_int_node *iter = clist_int_find(list, 30);                                                   \
     * clist_int_insert(list, iter, 20);  // 在 30 之前插入 20                                               \
     * // 链表现在包含: 10 -> 20 -> 30                                                                      \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 如果 iter 为 NULL，元素将插入到链表尾部                                                              \
     * @note 时间复杂度 O(1)                                                                                 \
     */                                                                                                    \
    static INLINE int clist_##NAME##_insert(clist_##NAME *clist, clist_##NAME##_node *iter, TYPE value)    \
    {                                                                                                      \
        return __clist_t_insert((__clist_t *)clist, (__clist_node_t *)iter, (const char *)&value, sizeof(TYPE)); \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 删除链表尾部元素                                                                                 \
     *                                                                                                     \
     * 移除链表的最后一个元素。                                                                                  \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_pop_back(list);  // 删除 20                                                               \
     * // 链表现在只包含: 10                                                                                 \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，行为未定义（不进行检查以提高性能）                                                          \
     * @note 使用前应检查链表是否为空                                                                            \
     */                                                                                                    \
    static INLINE void clist_##NAME##_pop_back(clist_##NAME *clist)                                        \
    {                                                                                                      \
        __clist_t_pop_back((__clist_t *)clist);                                                            \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 删除链表头部元素                                                                                 \
     *                                                                                                     \
     * 移除链表的第一个元素。                                                                                   \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_pop_front(list);  // 删除 10                                                              \
     * // 链表现在只包含: 20                                                                                 \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，行为未定义（不进行检查以提高性能）                                                          \
     * @note 使用前应检查链表是否为空                                                                            \
     */                                                                                                    \
    static INLINE void clist_##NAME##_pop_front(clist_##NAME *clist)                                       \
    {                                                                                                      \
        __clist_t_pop_front((__clist_t *)clist);                                                           \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 清空链表                                                                                       \
     *                                                                                                     \
     * 删除链表中的所有元素，但保留链表结构本身。                                                                     \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_clear(list);  // 清空所有元素                                                               \
     * // 现在 clist_int_empty(list) 返回真                                                                 \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)，n 为元素数量                                                                      \
     * @note 如果元素是指针类型，此函数不会释放指针指向的内存                                                          \
     */                                                                                                    \
    static INLINE void clist_##NAME##_clear(clist_##NAME *clist)                                           \
    {                                                                                                      \
        __clist_t_clear((__clist_t *)clist);                                                               \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取链表头部元素（值拷贝）                                                                         \
     *                                                                                                     \
     * 返回链表第一个元素的拷贝。                                                                                 \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 链表头部元素的值                                                                               \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * int first = clist_int_front(list);  // first = 10                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，行为未定义                                                                           \
     * @note 返回值拷贝，不能用于修改元素                                                                         \
     */                                                                                                    \
    static INLINE TYPE clist_##NAME##_front(clist_##NAME *clist)                                           \
    {                                                                                                      \
        return *((TYPE *)__clist_t_front((__clist_t *)clist));                                             \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取链表尾部元素（值拷贝）                                                                         \
     *                                                                                                     \
     * 返回链表最后一个元素的拷贝。                                                                               \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 链表尾部元素的值                                                                               \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * int last = clist_int_back(list);  // last = 20                                                     \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，行为未定义                                                                           \
     * @note 返回值拷贝，不能用于修改元素                                                                         \
     */                                                                                                    \
    static INLINE TYPE clist_##NAME##_back(clist_##NAME *clist)                                            \
    {                                                                                                      \
        return *((TYPE *)__clist_t_back((__clist_t *)clist));                                              \
    }                                                                                                      \
    /**                                                                                                    \
     * @brief 获取链表头部元素的指针                                                                            \
     *                                                                                                     \
     * 返回指向链表第一个元素的指针，可用于直接修改元素。                                                              \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 指向头部元素的指针                                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * int *p = clist_int_front_ptr(list);                                                                \
     * *p = 99;  // 修改头部元素为 99                                                                        \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，行为未定义                                                                           \
     * @note 返回的指针在链表结构改变时可能失效                                                                     \
     */                                                                                                    \
    static INLINE TYPE *clist_##NAME##_front_ptr(clist_##NAME *clist)                                      \
    {                                                                                                      \
        return ((TYPE *)__clist_t_front((__clist_t *)clist));                                              \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取链表尾部元素的指针                                                                            \
     *                                                                                                     \
     * 返回指向链表最后一个元素的指针，可用于直接修改元素。                                                            \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 指向尾部元素的指针                                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * int *p = clist_int_back_ptr(list);                                                                 \
     * *p = 99;  // 修改尾部元素为 99                                                                        \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，行为未定义                                                                           \
     * @note 返回的指针在链表结构改变时可能失效                                                                     \
     */                                                                                                    \
    static INLINE TYPE *clist_##NAME##_back_ptr(clist_##NAME *clist)                                       \
    {                                                                                                      \
        return ((TYPE *)__clist_t_back((__clist_t *)clist));                                               \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 安全获取链表头部元素的指针                                                                         \
     *                                                                                                     \
     * 返回指向链表第一个元素的指针，如果链表为空则返回 NULL。                                                         \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 指向头部元素的指针，链表为空时返回 NULL                                                             \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * int *p = clist_int_front_safe(list);                                                               \
     * if (p) {                                                                                           \
     *     printf("First element: %d\n", *p);                                                             \
     * } else {                                                                                           \
     *     printf("List is empty\n");                                                                     \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 与 clist_##NAME##_front_ptr 相比，此函数会检查链表是否为空                                            \
     */                                                                                                    \
    static INLINE TYPE *clist_##NAME##_front_safe(clist_##NAME *clist)                                     \
    {                                                                                                      \
        return ((TYPE *)__clist_t_front_safe((__clist_t *)clist));                                         \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 安全获取链表尾部元素的指针                                                                         \
     *                                                                                                     \
     * 返回指向链表最后一个元素的指针，如果链表为空则返回 NULL。                                                       \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 指向尾部元素的指针，链表为空时返回 NULL                                                             \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 42);                                                                     \
     * int *p = clist_int_back_safe(list);                                                                \
     * if (p) {                                                                                           \
     *     printf("Last element: %d\n", *p);                                                              \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 与 clist_##NAME##_back_ptr 相比，此函数会检查链表是否为空                                             \
     */                                                                                                    \
    static INLINE TYPE *clist_##NAME##_back_safe(clist_##NAME *clist)                                      \
    {                                                                                                      \
        return ((TYPE *)__clist_t_back_safe((__clist_t *)clist));                                          \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取链表的开始迭代器                                                                             \
     *                                                                                                     \
     * 返回指向链表第一个节点的迭代器。                                                                           \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 指向第一个节点的迭代器                                                                          \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * for (clist_int_node *it = clist_int_begin(list);                                                   \
     *      it != clist_int_end(list);                                                                    \
     *      it = clist_int_next(list, it)) {                                                              \
     *     int value = *it->value_ptr;                                                                    \
     *     printf("%d ", value);                                                                          \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果链表为空，返回值等于 clist_##NAME##_end                                                        \
     */                                                                                                    \
    static INLINE clist_##NAME##_node *clist_##NAME##_begin(clist_##NAME *clist)                           \
    {                                                                                                      \
        return (clist_##NAME##_node *)__clist_t_begin((__clist_t *)clist);                                 \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取链表的结束迭代器                                                                             \
     *                                                                                                     \
     * 返回表示链表末尾之后位置的迭代器（哨兵值）。                                                                  \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 表示结束位置的迭代器                                                                            \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * for (clist_int_node *it = clist_int_begin(list);                                                   \
     *      it != clist_int_end(list);                                                                    \
     *      it = clist_int_next(list, it)) {                                                              \
     *     // 遍历所有元素                                                                                   \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 不能对返回的迭代器解引用                                                                           \
     * @note 用于判断遍历是否结束                                                                              \
     */                                                                                                    \
    static INLINE clist_##NAME##_node *clist_##NAME##_end(clist_##NAME *clist)                             \
    {                                                                                                      \
        return (clist_##NAME##_node *)__clist_t_end((__clist_t *)clist);                                   \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取下一个节点的迭代器                                                                            \
     *                                                                                                     \
     * 返回给定节点的下一个节点。                                                                                \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param node 当前节点                                                                                  \
     * @return 下一个节点的迭代器                                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_node *node = clist_int_begin(list);                                                      \
     * node = clist_int_next(list, node);  // 移动到下一个节点                                                \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果 node 是最后一个节点，返回 clist_##NAME##_end                                                  \
     */                                                                                                    \
    static INLINE clist_##NAME##_node *clist_##NAME##_next(clist_##NAME *clist, clist_##NAME##_node *node) \
    {                                                                                                      \
        return (clist_##NAME##_node *)__clist_t_next((__clist_t *)clist, (__clist_node_t *)node);          \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取前一个节点的迭代器                                                                            \
     *                                                                                                     \
     * 返回给定节点的前一个节点。                                                                                \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param node 当前节点                                                                                  \
     * @return 前一个节点的迭代器                                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_node *node = clist_int_prev(list, clist_int_end(list));  // 获取最后一个节点               \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 如果 node 是第一个节点，行为取决于实现（可能返回 end）                                                  \
     */                                                                                                    \
    static INLINE clist_##NAME##_node *clist_##NAME##_prev(clist_##NAME *clist, clist_##NAME##_node *node) \
    {                                                                                                      \
        return (clist_##NAME##_node *)__clist_t_prev((__clist_t *)clist, (__clist_node_t *)node);          \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 删除指定节点                                                                                   \
     *                                                                                                     \
     * 从链表中删除指定的节点。                                                                                  \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param node 要删除的节点                                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 30);                                                                     \
     * clist_int_node *node = clist_int_find(list, 20);                                                   \
     * if (node) {                                                                                        \
     *     clist_int_erase(list, node);  // 删除值为 20 的节点                                               \
     * }                                                                                                  \
     * // 链表现在包含: 10 -> 30                                                                            \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 删除后，传入的节点指针失效                                                                         \
     */                                                                                                    \
    static INLINE void clist_##NAME##_erase(clist_##NAME *clist, clist_##NAME##_node *node)                \
    {                                                                                                      \
        __clist_t_erase((__clist_t *)clist, (__clist_node_t *)node);                                       \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 删除所有匹配指定值的元素                                                                          \
     *                                                                                                     \
     * 遍历链表并删除所有与指定值相等的元素（使用 memcmp 比较）。                                                      \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param value 要删除的值                                                                               \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_remove(list, 10);  // 删除所有值为 10 的元素                                                 \
     * // 链表现在只包含: 20                                                                                 \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 会删除所有匹配的元素，不仅仅是第一个                                                                  \
     */                                                                                                    \
    static INLINE void clist_##NAME##_remove(clist_##NAME *clist, TYPE value)                              \
    {                                                                                                      \
        __clist_t_remove((__clist_t *)clist, (const char *)&value, sizeof(TYPE));                          \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 默认比较函数                                                                                    \
     *                                                                                                     \
     * 使用 memcmp 按字节比较两个元素。                                                                          \
     *                                                                                                     \
     * @param lhs 左操作数指针                                                                               \
     * @param rhs 右操作数指针                                                                               \
     * @return 负数表示 lhs < rhs，0 表示相等，正数表示 lhs > rhs                                                \
     *                                                                                                     \
     * @note 此函数通常不需要用户直接调用                                                                         \
     * @note 对于复杂类型或特殊排序需求，应提供自定义比较函数                                                         \
     */                                                                                                    \
    static INLINE int clist_##NAME##_cmp(const TYPE *lhs, const TYPE *rhs)                                 \
    {                                                                                                      \
        return memcmp(lhs, rhs, sizeof(TYPE));                                                             \
    }                                                                                                      \
    /**                                                                                                    \
     * @brief 对链表进行排序（使用默认比较函数）                                                                   \
     *                                                                                                     \
     * 使用归并排序对链表进行排序，默认使用 memcmp 比较元素。                                                         \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 30);                                                                     \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_sort(list);  // 排序后: 10 -> 20 -> 30                                                    \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n log n)                                                                           \
     * @note 对于自定义排序，使用 clist_##NAME##_sort_with_cmp                                                 \
     * @note 排序是稳定的                                                                                     \
     */                                                                                                    \
    static INLINE void clist_##NAME##_sort(clist_##NAME *clist)                                            \
    {                                                                                                      \
        __clist_t_sort((__clist_t *)clist, (__clist_t_cmp_cb)clist_##NAME##_cmp);                          \
    }                                                                                                      \
    /**                                                                                                    \
     * @brief 使用自定义比较函数对链表进行排序                                                                     \
     *                                                                                                     \
     * 使用归并排序和用户提供的比较函数对链表进行排序。                                                               \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param cmp 比较函数，返回负数表示第一个参数小于第二个参数                                                       \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * int cmp_desc(const int *a, const int *b) {                                                         \
     *     return (*b - *a);  // 降序排序                                                                   \
     * }                                                                                                  \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 30);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_sort_with_cmp(list, cmp_desc);  // 排序后: 30 -> 20 -> 10                                 \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n log n)                                                                           \
     * @note 排序是稳定的                                                                                     \
     */                                                                                                    \
    static INLINE void clist_##NAME##_sort_with_cmp(clist_##NAME *clist, clist_##NAME##_cmp_cb cmp)        \
    {                                                                                                      \
        __clist_t_sort((__clist_t *)clist, (__clist_t_cmp_cb)cmp);                                         \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 反转链表                                                                                       \
     *                                                                                                     \
     * 将链表中的元素顺序反转。                                                                                  \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 30);                                                                     \
     * clist_int_reverse(list);  // 反转后: 30 -> 20 -> 10                                                 \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 原地反转，不需要额外内存                                                                           \
     */                                                                                                    \
    static INLINE void clist_##NAME##_reverse(clist_##NAME *clist)                                         \
    {                                                                                                      \
        __clist_t_reverse((__clist_t *)clist);                                                             \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 删除链表中的连续重复元素                                                                          \
     *                                                                                                     \
     * 删除链表中连续的重复元素，只保留每组重复元素的第一个。使用 memcmp 比较。                                          \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_unique(list);  // 结果: 10 -> 20 -> 10                                                    \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 只删除连续的重复元素，不同位置的相同元素会保留                                                           \
     * @note 通常先调用 sort 再调用 unique 以删除所有重复元素                                                       \
     */                                                                                                    \
    static INLINE void clist_##NAME##_unique(clist_##NAME *clist)                                          \
    {                                                                                                      \
        __clist_t_unique((__clist_t *)clist, sizeof(TYPE));                                                \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 使用自定义比较函数删除连续重复元素                                                                    \
     *                                                                                                     \
     * 删除链表中连续的重复元素，使用用户提供的比较函数判断是否相等。                                                    \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param cmp 比较函数，返回 0 表示相等                                                                     \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * int cmp_abs(const int *a, const int *b) {                                                          \
     *     return abs(*a) - abs(*b);  // 按绝对值比较                                                        \
     * }                                                                                                  \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, -10);                                                                    \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_unique_with_cmp(list, cmp_abs);  // 结果: 10 -> 20                                        \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 只删除连续的重复元素                                                                              \
     */                                                                                                    \
    static INLINE void clist_##NAME##_unique_with_cmp(clist_##NAME *clist, clist_##NAME##_cmp_cb cmp)     \
    {                                                                                                      \
        __clist_t_unique_with_cmp((__clist_t *)clist, (__clist_t_cmp_cb)cmp);                              \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 合并两个已排序的链表                                                                             \
     *                                                                                                     \
     * 将另一个已排序的链表合并到当前链表中，合并后的链表保持有序。other 链表会被清空。                                    \
     *                                                                                                     \
     * @param clist 目标链表指针（已排序）                                                                      \
     * @param other 源链表指针（已排序，合并后会被清空）                                                            \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_push_back(list1, 30);                                                                    \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 20);                                                                    \
     * clist_int_push_back(list2, 40);                                                                    \
     * clist_int_merge(list1, list2);  // list1: 10 -> 20 -> 30 -> 40, list2: 空                          \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n + m)，n 和 m 分别为两个链表的长度                                                     \
     * @note 两个链表都必须已排序，否则结果未定义                                                                  \
     * @note other 链表在合并后会变为空                                                                         \
     */                                                                                                    \
    static INLINE void clist_##NAME##_merge(clist_##NAME *clist, clist_##NAME *other)                      \
    {                                                                                                      \
        __clist_t_merge((__clist_t *)clist, (__clist_t *)other, (__clist_t_cmp_cb)clist_##NAME##_cmp);    \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 使用自定义比较函数合并两个已排序的链表                                                                \
     *                                                                                                     \
     * 将另一个已排序的链表合并到当前链表中，使用用户提供的比较函数。other 链表会被清空。                                  \
     *                                                                                                     \
     * @param clist 目标链表指针（已排序）                                                                      \
     * @param other 源链表指针（已排序，合并后会被清空）                                                            \
     * @param cmp 比较函数                                                                                   \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * int cmp_desc(const int *a, const int *b) { return *b - *a; }                                       \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 30);                                                                    \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_sort_with_cmp(list1, cmp_desc);                                                          \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 40);                                                                    \
     * clist_int_push_back(list2, 20);                                                                    \
     * clist_int_sort_with_cmp(list2, cmp_desc);                                                          \
     * clist_int_merge_with_cmp(list1, list2, cmp_desc);  // list1: 40 -> 30 -> 20 -> 10                 \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n + m)                                                                             \
     * @note 两个链表都必须按照相同的比较函数排序                                                                   \
     */                                                                                                    \
    static INLINE void clist_##NAME##_merge_with_cmp(clist_##NAME *clist, clist_##NAME *other, clist_##NAME##_cmp_cb cmp) \
    {                                                                                                      \
        __clist_t_merge((__clist_t *)clist, (__clist_t *)other, (__clist_t_cmp_cb)cmp);                    \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 调整链表大小                                                                                   \
     *                                                                                                     \
     * 将链表调整为指定大小。如果新大小大于当前大小，使用指定值填充；如果小于当前大小，删除尾部元素。                         \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param new_size 新的大小                                                                             \
     * @param value 用于填充的值（当新大小大于当前大小时）                                                           \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_resize(list, 5, 0);  // 调整为 5 个元素: 10, 20, 0, 0, 0                                    \
     * clist_int_resize(list, 2, 0);  // 调整为 2 个元素: 10, 20                                             \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(|new_size - current_size|)                                                         \
     */                                                                                                    \
    static INLINE int clist_##NAME##_resize(clist_##NAME *clist, unsigned int new_size, TYPE value)       \
    {                                                                                                      \
        return __clist_t_resize((__clist_t *)clist, new_size, (const char *)&value, sizeof(TYPE));         \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 用指定值填充链表                                                                                \
     *                                                                                                     \
     * 清空链表并用 count 个指定值的副本填充。                                                                     \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param count 元素数量                                                                                \
     * @param value 填充值                                                                                  \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_assign(list, 3, 99);  // 清空并填充: 99, 99, 99                                             \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(count)                                                                             \
     * @note 原有的所有元素会被删除                                                                             \
     */                                                                                                    \
    static INLINE int clist_##NAME##_assign(clist_##NAME *clist, unsigned int count, TYPE value)          \
    {                                                                                                      \
        return __clist_t_assign((__clist_t *)clist, count, (const char *)&value, sizeof(TYPE));            \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 将另一个链表的所有元素拼接到指定位置                                                                  \
     *                                                                                                     \
     * 将 other 链表的所有元素移动到当前链表的 pos 位置之前。other 链表会被清空。                                       \
     *                                                                                                     \
     * @param clist 目标链表指针                                                                             \
     * @param pos 插入位置的迭代器（元素将插入到此位置之前）                                                           \
     * @param other 源链表指针（拼接后会被清空）                                                                   \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_push_back(list1, 40);                                                                    \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 20);                                                                    \
     * clist_int_push_back(list2, 30);                                                                    \
     * clist_int_node *pos = clist_int_find(list1, 40);                                                   \
     * clist_int_splice(list1, pos, list2);  // list1: 10 -> 20 -> 30 -> 40, list2: 空                    \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 不涉及元素复制，只是节点链接的改变                                                                     \
     * @note other 链表在操作后会变为空                                                                         \
     */                                                                                                    \
    static INLINE void clist_##NAME##_splice(clist_##NAME *clist, clist_##NAME##_node *pos, clist_##NAME *other) \
    {                                                                                                      \
        __clist_t_splice((__clist_t *)clist, (__clist_node_t *)pos, (__clist_t *)other, NULL, NULL);      \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 将另一个链表的单个元素拼接到指定位置                                                                  \
     *                                                                                                     \
     * 将 other 链表中的 it 节点移动到当前链表的 pos 位置之前。                                                      \
     *                                                                                                     \
     * @param clist 目标链表指针                                                                             \
     * @param pos 插入位置的迭代器（元素将插入到此位置之前）                                                           \
     * @param other 源链表指针                                                                               \
     * @param it 要移动的节点                                                                                \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_push_back(list1, 30);                                                                    \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 20);                                                                    \
     * clist_int_push_back(list2, 25);                                                                    \
     * clist_int_node *pos = clist_int_find(list1, 30);                                                   \
     * clist_int_node *it = clist_int_find(list2, 20);                                                    \
     * clist_int_splice_one(list1, pos, list2, it);  // list1: 10 -> 20 -> 30, list2: 25                 \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 不涉及元素复制                                                                                   \
     */                                                                                                    \
    static INLINE void clist_##NAME##_splice_one(clist_##NAME *clist, clist_##NAME##_node *pos, clist_##NAME *other, clist_##NAME##_node *it) \
    {                                                                                                      \
        __clist_t_splice((__clist_t *)clist, (__clist_node_t *)pos, (__clist_t *)other, (__clist_node_t *)it, (__clist_node_t *)it); \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 将另一个链表的指定范围元素拼接到指定位置                                                              \
     *                                                                                                     \
     * 将 other 链表中 [first, last] 范围内的元素移动到当前链表的 pos 位置之前。                                      \
     *                                                                                                     \
     * @param clist 目标链表指针                                                                             \
     * @param pos 插入位置的迭代器（元素将插入到此位置之前）                                                           \
     * @param other 源链表指针                                                                               \
     * @param first 范围起始节点（包含）                                                                        \
     * @param last 范围结束节点（包含）                                                                         \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_push_back(list1, 50);                                                                    \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 20);                                                                    \
     * clist_int_push_back(list2, 30);                                                                    \
     * clist_int_push_back(list2, 40);                                                                    \
     * clist_int_node *pos = clist_int_find(list1, 50);                                                   \
     * clist_int_node *first = clist_int_find(list2, 20);                                                 \
     * clist_int_node *last = clist_int_find(list2, 30);                                                  \
     * clist_int_splice_range(list1, pos, list2, first, last);  // list1: 10->20->30->50, list2: 40      \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)，n 为范围内的元素数量                                                                \
     */                                                                                                    \
    static INLINE void clist_##NAME##_splice_range(clist_##NAME *clist, clist_##NAME##_node *pos, clist_##NAME *other, clist_##NAME##_node *first, clist_##NAME##_node *last) \
    {                                                                                                      \
        __clist_t_splice((__clist_t *)clist, (__clist_node_t *)pos, (__clist_t *)other, (__clist_node_t *)first, (__clist_node_t *)last); \
    }                                                                                                      \
                                                                                                           \
    typedef int (*clist_##NAME##_pred_cb)(const TYPE *value);                                              \
    /**                                                                                                    \
     * @brief 删除满足条件的所有元素                                                                            \
     *                                                                                                     \
     * 遍历链表并删除所有使谓词函数返回真的元素。                                                                    \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param pred 谓词函数，返回非 0 值表示删除该元素                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * int is_even(const int *value) {                                                                    \
     *     return (*value % 2 == 0);                                                                      \
     * }                                                                                                  \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 15);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 25);                                                                     \
     * clist_int_remove_if(list, is_even);  // 删除所有偶数，结果: 15 -> 25                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 会删除所有满足条件的元素                                                                            \
     */                                                                                                    \
    static INLINE void clist_##NAME##_remove_if(clist_##NAME *clist, clist_##NAME##_pred_cb pred)         \
    {                                                                                                      \
        __clist_t_remove_if((__clist_t *)clist, (__clist_t_pred_cb)pred);                                  \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 在指定位置插入数组元素                                                                            \
     *                                                                                                     \
     * 将数组中的所有元素插入到指定位置之前。                                                                       \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param pos 插入位置的迭代器（NULL 表示在尾部插入）                                                           \
     * @param array 要插入的数组                                                                             \
     * @param count 数组元素数量                                                                             \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 40);                                                                     \
     * int arr[] = {20, 30};                                                                              \
     * clist_int_node *pos = clist_int_find(list, 40);                                                    \
     * clist_int_insert_array(list, pos, arr, 2);  // 结果: 10 -> 20 -> 30 -> 40                          \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(count)                                                                             \
     * @note 元素会被复制到链表中                                                                              \
     */                                                                                                    \
    static INLINE int clist_##NAME##_insert_array(clist_##NAME *clist, clist_##NAME##_node *pos, const TYPE *array, size_t count) \
    {                                                                                                      \
        return __clist_t_insert_array((__clist_t *)clist, (__clist_node_t *)pos, (const char *)array, count, sizeof(TYPE)); \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 在链表尾部追加数组元素                                                                            \
     *                                                                                                     \
     * 将数组中的所有元素添加到链表的末尾。                                                                         \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param array 要追加的数组                                                                             \
     * @param count 数组元素数量                                                                             \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * int arr[] = {20, 30, 40};                                                                          \
     * clist_int_append_array(list, arr, 3);  // 结果: 10 -> 20 -> 30 -> 40                               \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(count)                                                                             \
     * @note 元素会被复制到链表中                                                                              \
     */                                                                                                    \
    static INLINE int clist_##NAME##_append_array(clist_##NAME *clist, const TYPE *array, size_t count)   \
    {                                                                                                      \
        return __clist_t_insert_array((__clist_t *)clist, NULL, (const char *)array, count, sizeof(TYPE)); \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 在链表头部前置数组元素                                                                            \
     *                                                                                                     \
     * 将数组中的所有元素添加到链表的开头。                                                                         \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param array 要前置的数组                                                                             \
     * @param count 数组元素数量                                                                             \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 40);                                                                     \
     * int arr[] = {10, 20, 30};                                                                          \
     * clist_int_prepend_array(list, arr, 3);  // 结果: 10 -> 20 -> 30 -> 40                              \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(count)                                                                             \
     * @note 元素会被复制到链表中                                                                              \
     * @note 数组元素会按原顺序添加到链表头部                                                                      \
     */                                                                                                    \
    static INLINE int clist_##NAME##_prepend_array(clist_##NAME *clist, const TYPE *array, size_t count)  \
    {                                                                                                      \
        return __clist_t_prepend_array((__clist_t *)clist, (const char *)array, count, sizeof(TYPE));     \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 用数组元素替换链表内容                                                                            \
     *                                                                                                     \
     * 清空链表并用数组中的元素填充。                                                                              \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param array 源数组                                                                                  \
     * @param count 数组元素数量                                                                             \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * int arr[] = {100, 200, 300};                                                                       \
     * clist_int_assign_array(list, arr, 3);  // 清空并填充: 100 -> 200 -> 300                             \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n + count)，n 为原链表大小                                                             \
     * @note 原有的所有元素会被删除                                                                             \
     */                                                                                                    \
    static INLINE int clist_##NAME##_assign_array(clist_##NAME *clist, const TYPE *array, size_t count)   \
    {                                                                                                      \
        __clist_t_clear((__clist_t *)clist);                                                               \
        return __clist_t_insert_array((__clist_t *)clist, NULL, (const char *)array, count, sizeof(TYPE)); \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 交换两个链表的内容                                                                               \
     *                                                                                                     \
     * 交换两个链表的所有元素。                                                                                   \
     *                                                                                                     \
     * @param list1 第一个链表指针                                                                            \
     * @param list2 第二个链表指针                                                                            \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_push_back(list1, 20);                                                                    \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 30);                                                                    \
     * clist_int_push_back(list2, 40);                                                                    \
     * clist_int_swap(list1, list2);  // list1: 30->40, list2: 10->20                                     \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(1)                                                                                 \
     * @note 不涉及元素复制，只交换内部指针                                                                       \
     */                                                                                                    \
    static INLINE void clist_##NAME##_swap(clist_##NAME *list1, clist_##NAME *list2)                      \
    {                                                                                                      \
        __clist_t_swap((__clist_t *)list1, (__clist_t *)list2);                                            \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 获取链表的最大可能大小                                                                            \
     *                                                                                                     \
     * 返回链表理论上可以容纳的最大元素数量。                                                                       \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @return 最大可能的元素数量                                                                              \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * size_t max = clist_int_max_size(list);                                                             \
     * printf("Max size: %zu\n", max);                                                                    \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 实际可用大小受限于系统可用内存                                                                       \
     * @note 此函数主要用于与 STL 兼容                                                                          \
     */                                                                                                    \
    static INLINE size_t clist_##NAME##_max_size(const clist_##NAME *clist)                                \
    {                                                                                                      \
        (void)clist;                                                                                       \
        return __clist_t_max_size();                                                                       \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 检查链表是否包含指定值                                                                            \
     *                                                                                                     \
     * 判断链表中是否存在等于指定值的元素（使用 memcmp 比较）。                                                       \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param value 要查找的值                                                                               \
     * @return 找到返回非 0 值，未找到返回 0                                                                     \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * if (clist_int_contains(list, 10)) {                                                                \
     *     printf("Found 10\n");                                                                          \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 使用 memcmp 进行字节比较                                                                          \
     */                                                                                                    \
    static INLINE int clist_##NAME##_contains(const clist_##NAME *clist, TYPE value)                       \
    {                                                                                                      \
        return __clist_t_contains((__clist_t *)clist, (const char *)&value, sizeof(TYPE));                 \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 查找指定值的节点                                                                                 \
     *                                                                                                     \
     * 在链表中查找第一个等于指定值的元素节点（使用 memcmp 比较）。                                                    \
     *                                                                                                     \
     * @param clist 链表指针                                                                                \
     * @param value 要查找的值                                                                               \
     * @return 找到的节点指针，未找到返回 NULL                                                                   \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int_push_back(list, 30);                                                                     \
     * clist_int_node *node = clist_int_find(list, 20);                                                   \
     * if (node) {                                                                                        \
     *     printf("Found value: %d\n", *node->value_ptr);                                                 \
     * }                                                                                                  \
     * clist_int_destroy(&list);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 只返回第一个匹配的节点                                                                             \
     * @note 返回的节点指针可用于 erase、insert 等操作                                                            \
     */                                                                                                    \
    static INLINE clist_##NAME##_node *clist_##NAME##_find(clist_##NAME *clist, TYPE value)                \
    {                                                                                                      \
        return (clist_##NAME##_node *)__clist_t_find((__clist_t *)clist, (const char *)&value, sizeof(TYPE)); \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 比较两个链表是否相等                                                                             \
     *                                                                                                     \
     * 判断两个链表的大小和所有对应元素是否相等（使用 memcmp 比较）。                                                  \
     *                                                                                                     \
     * @param lhs 第一个链表指针                                                                             \
     * @param rhs 第二个链表指针                                                                             \
     * @return 相等返回非 0 值，不相等返回 0                                                                     \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list1 = clist_int_create();                                                              \
     * clist_int_push_back(list1, 10);                                                                    \
     * clist_int_push_back(list1, 20);                                                                    \
     * clist_int *list2 = clist_int_create();                                                              \
     * clist_int_push_back(list2, 10);                                                                    \
     * clist_int_push_back(list2, 20);                                                                    \
     * if (clist_int_equal(list1, list2)) {                                                               \
     *     printf("Lists are equal\n");                                                                   \
     * }                                                                                                  \
     * clist_int_destroy(&list1);                                                                         \
     * clist_int_destroy(&list2);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 比较元素的顺序和值                                                                                \
     */                                                                                                    \
    static INLINE int clist_##NAME##_equal(const clist_##NAME *lhs, const clist_##NAME *rhs)               \
    {                                                                                                      \
        return __clist_t_equal((__clist_t *)lhs, (__clist_t *)rhs, sizeof(TYPE));                          \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 克隆链表                                                                                       \
     *                                                                                                     \
     * 创建源链表的一个完整副本。                                                                                 \
     *                                                                                                     \
     * @param src 源链表指针                                                                                 \
     * @return 新创建的链表指针，失败返回 NULL                                                                   \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *list = clist_int_create();                                                               \
     * clist_int_push_back(list, 10);                                                                     \
     * clist_int_push_back(list, 20);                                                                     \
     * clist_int *clone = clist_int_clone(list);                                                          \
     * // clone 包含与 list 相同的元素                                                                        \
     * clist_int_destroy(&list);                                                                          \
     * clist_int_destroy(&clone);                                                                         \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 执行深拷贝，新链表与原链表独立                                                                       \
     * @note 如果元素是指针类型，只复制指针值，不复制指针指向的对象                                                    \
     * @note 使用完毕后需要调用 destroy 释放                                                                    \
     */                                                                                                    \
    static INLINE clist_##NAME *clist_##NAME##_clone(const clist_##NAME *src)                              \
    {                                                                                                      \
        return (clist_##NAME *)__clist_t_clone((__clist_t *)src, sizeof(TYPE));                            \
    }                                                                                                      \
                                                                                                           \
    /**                                                                                                    \
     * @brief 复制链表内容到另一个链表                                                                          \
     *                                                                                                     \
     * 将源链表的所有元素复制到目标链表，目标链表原有内容会被清空。                                                      \
     *                                                                                                     \
     * @param dest 目标链表指针（必须已初始化）                                                                   \
     * @param src 源链表指针                                                                                 \
     * @return 成功返回 0，失败返回非 0 值                                                                      \
     *                                                                                                     \
     * @par 简单用法:                                                                                        \
     * @code                                                                                               \
     * clist_int *src = clist_int_create();                                                                \
     * clist_int_push_back(src, 10);                                                                      \
     * clist_int_push_back(src, 20);                                                                      \
     * clist_int *dest = clist_int_create();                                                               \
     * clist_int_copy(dest, src);  // dest 现在包含: 10 -> 20                                              \
     * clist_int_destroy(&src);                                                                           \
     * clist_int_destroy(&dest);                                                                          \
     * @endcode                                                                                           \
     *                                                                                                     \
     * @note 时间复杂度 O(n)                                                                                 \
     * @note 目标链表必须已经初始化或创建                                                                         \
     * @note 目标链表的原有内容会被清空                                                                           \
     * @note 如果元素是指针类型，只复制指针值                                                                      \
     */                                                                                                    \
    static INLINE int clist_##NAME##_copy(clist_##NAME *dest, const clist_##NAME *src)                     \
    {                                                                                                      \
        return __clist_t_copy((__clist_t *)dest, (__clist_t *)src, sizeof(TYPE));                          \
    }

/**
 * @brief 安全遍历链表（支持删除操作）
 * 
 * 提供一个支持在遍历过程中删除元素的 for 循环宏。
 * 
 * @param list_ptr 链表指针
 * @param item_type 元素类型
 * @param item_name 循环变量名
 * 
 * @par 简单用法:
 * @code
 * clist_int *list = clist_int_create();
 * clist_int_push_back(list, 10);
 * clist_int_push_back(list, 20);
 * clist_int_push_back(list, 30);
 * 
 * clist_foreach(list, int, value) {
 *     printf("%d ", value);
 *     if (value == 20) {
 *         clist_int_erase(list, value_iter);  // 可以安全删除当前元素
 *     }
 * }
 * clist_int_destroy(&list);
 * @endcode
 * 
 * @note 可以在循环体内访问以下变量：
 *       - item_name: 当前元素的值
 *       - item_name##_iter: 当前元素的迭代器（节点指针）
 *       - item_name##_index: 当前元素的索引（从 0 开始）
 * @note 支持在遍历过程中删除当前元素
 * @note 不要在循环体内修改链表结构（除了删除当前元素）
 */
#define clist_foreach(list_ptr, item_type, item_name) clist_as_foreach(list_ptr, item_type, item_name, item_type)

/**
 * @brief 安全遍历链表（支持删除操作，可指定类型别名）
 * 
 * 与 clist_foreach 类似，但允许指定类型别名。用于处理复杂类型名称或指针类型。
 * 
 * @param list_ptr 链表指针
 * @param item_type 元素类型
 * @param item_name 循环变量名
 * @param alias 类型别名（用于生成函数名）
 * 
 * @par 简单用法:
 * @code
 * // 对于指针类型链表
 * DEFINE_CLIST_PTR(MyStruct)
 * clist_MyStruct_ptr *list = clist_MyStruct_ptr_create();
 * // 添加元素...
 * 
 * clist_as_foreach(list, MyStruct*, ptr, MyStruct_ptr) {
 *     printf("Value: %d\n", ptr->field);
 *     // 可以安全删除
 *     if (should_remove(ptr)) {
 *         clist_MyStruct_ptr_erase(list, ptr_iter);
 *     }
 * }
 * clist_MyStruct_ptr_destroy(&list);
 * @endcode
 * 
 * @note 可访问 item_name、item_name##_iter、item_name##_index
 * @note 支持在遍历过程中删除当前元素
 */
#define clist_as_foreach(list_ptr, item_type, item_name, alias)                                                                            \
    for (int item_name##_scope_ = 1; item_name##_scope_; item_name##_scope_ = 0)                                                       \
        for (item_type item_name = (item_type){0}; item_name##_scope_; item_name##_scope_ = 0)                                         \
            for (size_t item_name##_index = 0; item_name##_scope_; item_name##_scope_ = 0)                                             \
                for (clist_##alias##_node *item_name##_iter = 0, *__##item_name##_next_iter = 0; item_name##_scope_; item_name##_scope_ = 0) \
                    if (!clist_##alias##_empty((list_ptr)) ?                                                                           \
                        (item_name##_iter = clist_##alias##_begin((list_ptr)),                                                         \
                         item_name = clist_##alias##_front((list_ptr)),                                                                \
                         __##item_name##_next_iter = clist_##alias##_next((list_ptr), item_name##_iter), 1) : 0)                      \
                        for (; __clist_foreach_check((__clist_t *)(list_ptr), (__clist_node_t *)item_name##_iter, (char *)&item_name, sizeof(item_type)) != 0; \
                             ++item_name##_index, item_name##_iter = __##item_name##_next_iter,                                        \
                             __##item_name##_next_iter = clist_##alias##_next((list_ptr), __##item_name##_next_iter))

/**
 * @brief 反向安全遍历链表（支持删除操作）
 * 
 * 从链表尾部向头部遍历，支持在遍历过程中删除当前元素。
 * 
 * @param list_ptr 链表指针
 * @param item_type 元素类型
 * @param item_name 循环变量名
 * 
 * @par 简单用法:
 * @code
 * clist_int *list = clist_int_create();
 * clist_int_push_back(list, 10);
 * clist_int_push_back(list, 20);
 * clist_int_push_back(list, 30);
 * 
 * clist_foreach_reverse(list, int, value) {
 *     printf("%d ", value);  // 输出: 30 20 10
 *     if (value == 20) {
 *         clist_int_erase(list, value_iter);  // 可以安全删除
 *     }
 * }
 * clist_int_destroy(&list);
 * @endcode
 * 
 * @note 可以在循环体内访问以下变量：
 *       - item_name: 当前元素的值
 *       - item_name##_iter: 当前元素的迭代器（节点指针）
 *       - item_name##_index: 当前元素的索引（从 size-1 递减到 0）
 * @note 支持在遍历过程中删除当前元素
 * @note 遍历顺序从尾到头
 */
#define clist_foreach_reverse(list_ptr, item_type, item_name) clist_as_foreach_reverse(list_ptr, item_type, item_name, item_type)

/**
 * @brief 反向安全遍历链表（支持删除操作，可指定类型别名）
 * 
 * 与 clist_foreach_reverse 类似，但允许指定类型别名。用于处理复杂类型名称或指针类型。
 * 
 * @param list_ptr 链表指针
 * @param item_type 元素类型
 * @param item_name 循环变量名
 * @param alias 类型别名（用于生成函数名）
 * 
 * @par 简单用法:
 * @code
 * // 对于指针类型链表
 * DEFINE_CLIST_PTR(MyStruct)
 * clist_MyStruct_ptr *list = clist_MyStruct_ptr_create();
 * // 添加元素...
 * 
 * clist_as_foreach_reverse(list, MyStruct*, ptr, MyStruct_ptr) {
 *     printf("Index: %zu, Value: %d\n", ptr_index, ptr->field);
 *     // 从尾到头遍历
 *     if (should_remove(ptr)) {
 *         clist_MyStruct_ptr_erase(list, ptr_iter);
 *     }
 * }
 * clist_MyStruct_ptr_destroy(&list);
 * @endcode
 * 
 * @note 可访问 item_name、item_name##_iter、item_name##_index
 * @note 支持在遍历过程中删除当前元素
 * @note 遍历顺序从尾到头，索引从 size-1 递减到 0
 */
#define clist_as_foreach_reverse(list_ptr, item_type, item_name, alias)                                                                       \
    for (int item_name##_scope_ = 1; item_name##_scope_; item_name##_scope_ = 0)                                                               \
        for (item_type item_name = (item_type){0}; item_name##_scope_; item_name##_scope_ = 0)                                                 \
            for (size_t item_name##_index = clist_##alias##_size((list_ptr)); item_name##_scope_; item_name##_scope_ = 0)                      \
                for (clist_##alias##_node *item_name##_iter = 0, *__##item_name##_next_iter = 0; item_name##_scope_; item_name##_scope_ = 0)   \
                    if (!clist_##alias##_empty((list_ptr)) ?                                                                                   \
                        (item_name##_index = clist_##alias##_size((list_ptr)) - 1,                                                             \
                         item_name##_iter = clist_##alias##_prev((list_ptr), clist_##alias##_end((list_ptr))),                                 \
                         __##item_name##_next_iter = clist_##alias##_prev((list_ptr), item_name##_iter),                                       \
                         item_name = clist_##alias##_back((list_ptr)), 1) : 0)                                                                 \
                        for (; __clist_foreach_reverse_check((__clist_t *)(list_ptr), (__clist_node_t *)item_name##_iter, (char *)&item_name, sizeof(item_type)) != 0; \
                             item_name##_index = (item_name##_index > 0) ? item_name##_index - 1 : 0, item_name##_iter = __##item_name##_next_iter, \
                             __##item_name##_next_iter = clist_##alias##_prev((list_ptr), __##item_name##_next_iter))

    // 注意：以下部分是库内部函数，用户不要调用-------------------

    typedef struct __clist_node_t __clist_node_t;
    typedef struct __clist_t __clist_t;
    typedef int (*__clist_t_cmp_cb)(const void *lhs, const void *rhs);
    typedef int (*__clist_t_pred_cb)(const void *value);

    CC_API __clist_t *CC_CALL __clist_t_create(size_t list_size, size_t item_size);
    CC_API void CC_CALL __clist_t_destroy(__clist_t *clist);
    CC_API int CC_CALL __clist_t_init(__clist_t *clist, size_t list_size, size_t item_size);
    CC_API void CC_CALL __clist_t_uninit(__clist_t *clist);
    CC_API unsigned int CC_CALL __clist_t_size(const __clist_t *clist);
    CC_API int CC_CALL __clist_t_empty(const __clist_t *clist);
    CC_API int CC_CALL __clist_t_push_back(__clist_t *clist, const char *item_ptr, size_t item_size);
    CC_API int CC_CALL __clist_t_push_front(__clist_t *clist, const char *item_ptr, size_t item_size);
    CC_API int CC_CALL __clist_t_insert(__clist_t *clist, __clist_node_t *iter, const char *item_ptr, size_t item_size);
    CC_API void CC_CALL __clist_t_pop_back(__clist_t *clist);
    CC_API void CC_CALL __clist_t_pop_front(__clist_t *clist);
    CC_API void CC_CALL __clist_t_clear(__clist_t *clist);
    CC_API char *CC_CALL __clist_t_front(__clist_t *clist);
    CC_API char *CC_CALL __clist_t_back(__clist_t *clist);
    CC_API char *CC_CALL __clist_t_front_safe(__clist_t *clist);
    CC_API char *CC_CALL __clist_t_back_safe(__clist_t *clist);
    CC_API __clist_node_t *CC_CALL __clist_t_begin(__clist_t *clist);
    CC_API __clist_node_t *CC_CALL __clist_t_end(__clist_t *clist);
    CC_API __clist_node_t *CC_CALL __clist_t_next(__clist_t *clist, __clist_node_t *node);
    CC_API __clist_node_t *CC_CALL __clist_t_prev(__clist_t *clist, __clist_node_t *node);
    CC_API void CC_CALL __clist_t_erase(__clist_t *clist, __clist_node_t *node);
    CC_API void CC_CALL __clist_t_remove(__clist_t *clist, const char *item_ptr, size_t item_size);
    CC_API int CC_CALL __clist_foreach_check(__clist_t *clist, __clist_node_t *iter, char *item_ptr, size_t item_size);
    CC_API int CC_CALL __clist_foreach_reverse_check(__clist_t *clist, __clist_node_t *iter, char *item_ptr, size_t item_size);
    CC_API void CC_CALL __clist_t_sort(__clist_t *clist, __clist_t_cmp_cb cmp);
    CC_API void CC_CALL __clist_t_reverse(__clist_t *clist);
    CC_API void CC_CALL __clist_t_unique(__clist_t *clist, size_t item_size);
    CC_API void CC_CALL __clist_t_unique_with_cmp(__clist_t *clist, __clist_t_cmp_cb cmp);
    CC_API void CC_CALL __clist_t_merge(__clist_t *clist, __clist_t *other, __clist_t_cmp_cb cmp);
    CC_API int CC_CALL __clist_t_resize(__clist_t *clist, unsigned int new_size, const char *value_ptr, size_t item_size);
    CC_API int CC_CALL __clist_t_assign(__clist_t *clist, unsigned int count, const char *value_ptr, size_t item_size);
    CC_API void CC_CALL __clist_t_splice(__clist_t *clist, __clist_node_t *pos, __clist_t *other, __clist_node_t *first, __clist_node_t *last);
    CC_API void CC_CALL __clist_t_remove_if(__clist_t *clist, __clist_t_pred_cb pred);
    CC_API int CC_CALL __clist_t_insert_array(__clist_t *clist, __clist_node_t *pos, const char *array, size_t count, size_t item_size);
    CC_API int CC_CALL __clist_t_prepend_array(__clist_t *clist, const char *array, size_t count, size_t item_size);
    CC_API void CC_CALL __clist_t_swap(__clist_t *list1, __clist_t *list2);
    CC_API size_t CC_CALL __clist_t_max_size(void);
    CC_API int CC_CALL __clist_t_contains(const __clist_t *clist, const char *item_ptr, size_t item_size);
    CC_API __clist_node_t *CC_CALL __clist_t_find(__clist_t *clist, const char *item_ptr, size_t item_size);
    CC_API int CC_CALL __clist_t_equal(const __clist_t *lhs, const __clist_t *rhs, size_t item_size);
    CC_API __clist_t *CC_CALL __clist_t_clone(const __clist_t *src, size_t item_size);
    CC_API int CC_CALL __clist_t_copy(__clist_t *dest, const __clist_t *src, size_t item_size);

#ifdef __cplusplus
}
#endif

#endif

#ifndef C_CORE_QUEUE_H_
#define C_CORE_QUEUE_H_

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
 * @brief 定义一个类型安全的队列
 * 
 * 此宏为指定类型生成队列的完整实现，包括数据结构定义和所有操作函数。
 * 队列名称将以类型名作为后缀。
 * 
 * @param TYPE 队列中存储的元素类型
 * 
 * @par 使用示例:
 * @code
 * DEFINE_CQUEUE(int);  // 定义 cqueue_int 队列
 * 
 * cqueue_int *q = cqueue_int_create();
 * cqueue_int_push(q, 42);
 * int val = cqueue_int_front(q);
 * cqueue_int_pop(q);
 * cqueue_int_destroy(&q);
 * @endcode
 */
#define DEFINE_CQUEUE(TYPE) DEFINE_CQUEUE_AS(TYPE, TYPE)

/**
 * @brief 定义一个指针类型的队列
 * 
 * 此宏为指定的指针类型生成队列实现。队列名称将自动添加 "_ptr" 后缀。
 * 
 * @param TYPE 队列中存储的指针的基础类型（不含*）
 * 
 * @par 使用示例:
 * @code
 * struct MyData { int id; char name[32]; };
 * DEFINE_CQUEUE_PTR(struct MyData);  // 定义 cqueue_struct_MyData_ptr 队列
 * 
 * cqueue_struct_MyData_ptr *q = cqueue_struct_MyData_ptr_create();
 * struct MyData *data = malloc(sizeof(struct MyData));
 * cqueue_struct_MyData_ptr_push(q, data);
 * cqueue_struct_MyData_ptr_destroy(&q);
 * @endcode
 */
#define DEFINE_CQUEUE_PTR(TYPE) DEFINE_CQUEUE_AS(TYPE *, TYPE##_ptr)

/**
 * @brief 定义一个指针类型的队列并指定名称
 * 
 * 此宏为指定的指针类型生成队列实现，允许自定义队列名称后缀。
 * 
 * @param TYPE 队列中存储的指针的基础类型（不含*）
 * @param NAME 队列名称后缀（将生成 cqueue_##NAME 形式的队列）
 * 
* @par 使用示例:
 * @code
 * struct MyData { int id; };
 * DEFINE_CQUEUE_PTR_AS(struct MyData, my_data);  // 定义 cqueue_my_data 队列
 * 
 * cqueue_my_data *q = cqueue_my_data_create();
 * struct MyData *data = malloc(sizeof(struct MyData));
 * cqueue_my_data_push(q, data);
 * cqueue_my_data_destroy(&q);
 * @endcode
 */
#define DEFINE_CQUEUE_PTR_AS(TYPE, NAME) DEFINE_CQUEUE_AS(TYPE *, NAME)

/**
 * @brief 定义一个类型安全的队列并指定名称
 * 
 * 这是最通用的队列定义宏，可以为任意类型生成队列实现并自定义名称。
 * 生成的队列遵循 FIFO（先进先出）原则。
 * 
 * @param TYPE 队列中存储的元素类型（可以是基础类型、结构体、指针等）
 * @param NAME 队列名称后缀（将生成 cqueue_##NAME 形式的队列）
 * 
 * @par 使用示例:
 * @code
 * typedef struct { double x, y; } Point;
 * DEFINE_CQUEUE_AS(Point, point);  // 定义 cqueue_point 队列
 * 
 * cqueue_point *q = cqueue_point_create();
 * Point p = {3.14, 2.71};
 * cqueue_point_push(q, p);
 * Point front = cqueue_point_front(q);
 * cqueue_point_destroy(&q);
 * @endcode
 */
#define DEFINE_CQUEUE_AS(TYPE, NAME)                                                                    \
    typedef struct cqueue_##NAME##_node                                                                 \
    {                                                                                                   \
        TYPE *value_ptr;                                                                                \
        struct cqueue_##NAME##_node *next;                                                              \
    } cqueue_##NAME##_node;                                                                             \
                                                                                                        \
    typedef cqueue_##NAME##_node *cqueue_##NAME##_iter;                                                 \
                                                                                                        \
    typedef struct cqueue_##NAME                                                                        \
    {                                                                                                   \
        unsigned int size;                                                                              \
        cqueue_##NAME##_node *head;                                                                     \
        cqueue_##NAME##_node *tail;                                                                     \
    } cqueue_##NAME;                                                                                    \
                                                                                                        \
    /**                                                                                                 \
     * @brief 创建一个新的队列实例（堆分配）                                                              \
     *                                                                                                  \
     * 在堆上动态分配内存并初始化一个新的队列。使用完毕后需要调用 destroy 函数释放资源。                     \
     *                                                                                                  \
     * @return 成功返回队列指针，失败返回 NULL                                                             \
     *                                                                                                  \
     * @note 使用完毕后必须调用 cqueue_##NAME##_destroy() 释放内存                                        \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * if (q != NULL) {                                                                                \
     *     // 使用队列...                                                                               \
     *     cqueue_##NAME##_destroy(&q);                                                                \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE cqueue_##NAME *cqueue_##NAME##_create()                                               \
    {                                                                                                   \
        return (cqueue_##NAME *)__cqueue_t_create(sizeof(cqueue_##NAME), sizeof(cqueue_##NAME##_node)); \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 销毁队列并释放所有资源                                                                       \
     *                                                                                                  \
     * 销毁由 create 函数创建的队列，释放队列及其所有节点占用的内存。                                          \
     * 函数执行后会将队列指针设置为 NULL。                                                                  \
     *                                                                                                  \
     * @param pq 指向队列指针的指针（二级指针）                                                              \
     *                                                                                                  \
     * @note 此函数只能用于 create 创建的队列，不能用于 init 初始化的队列                                      \
     * @note 如果队列中存储的是指针，需要在销毁队列前手动释放指针指向的内存                                       \
     * @note pq 为 NULL 或 *pq 为 NULL 时函数安全返回                                                       \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * cqueue_##NAME##_destroy(&q);  // q 将被设置为 NULL                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE void cqueue_##NAME##_destroy(cqueue_##NAME **pq)                                      \
    {                                                                                                   \
        if (pq && *pq)                                                                                  \
        {                                                                                               \
            __cqueue_t_destroy((__cqueue_t *)(*pq));                                                    \
            *pq = 0;                                                                                    \
        }                                                                                               \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 初始化栈上或静态分配的队列                                                                    \
     *                                                                                                  \
     * 初始化一个已分配内存的队列结构体，适用于栈分配或静态分配的队列。                                           \
     * 使用完毕后需要调用 uninit 函数清理资源。                                                              \
     *                                                                                                  \
     * @param q 指向待初始化的队列结构体的指针                                                               \
     * @return 成功返回 0，失败返回非 0 值                                                                  \
     *                                                                                                  \
     * @note 使用完毕后必须调用 cqueue_##NAME##_uninit() 清理资源                                           \
     * @note 不要对同一个队列重复调用 init，除非先调用了 uninit                                               \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME q;                                                                                \
     * if (cqueue_##NAME##_init(&q) == 0) {                                                            \
     *     cqueue_##NAME##_push(&q, value);                                                            \
     *     // 使用队列...                                                                               \
     *     cqueue_##NAME##_uninit(&q);                                                                 \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE int cqueue_##NAME##_init(struct cqueue_##NAME *q)                                     \
    {                                                                                                   \
        return __cqueue_t_init((__cqueue_t *)q, sizeof(cqueue_##NAME), sizeof(cqueue_##NAME##_node));   \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 清理队列资源                                                                               \
     *                                                                                                  \
     * 清理由 init 函数初始化的队列，释放队列内所有节点占用的内存，但不释放队列结构体本身。                        \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     *                                                                                                  \
     * @note 此函数只能用于 init 初始化的队列，不能用于 create 创建的队列                                      \
     * @note 如果队列中存储的是指针，需要在 uninit 前手动释放指针指向的内存                                      \
     * @note 调用后队列结构体仍然存在，但不应再使用，除非重新调用 init                                          \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME q;                                                                                \
     * cqueue_##NAME##_init(&q);                                                                       \
     * cqueue_##NAME##_push(&q, value);                                                                \
     * cqueue_##NAME##_uninit(&q);  // 清理资源                                                         \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE void cqueue_##NAME##_uninit(struct cqueue_##NAME *q)                                  \
    {                                                                                                   \
        __cqueue_t_uninit((__cqueue_t *)q);                                                             \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 将元素插入队列尾部                                                                           \
     *                                                                                                  \
     * 将指定元素的副本添加到队列的尾部。元素通过值传递，函数内部会复制元素内容。                                  \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @param val 要插入的元素值                                                                          \
     * @return 成功返回 0，失败返回非 0 值（通常是内存分配失败）                                               \
     *                                                                                                  \
     * @note 元素是按值复制的，对于大型结构体可能影响性能                                                      \
     * @note 如果内存分配失败，队列状态保持不变                                                               \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * TYPE value = ...;                                                                               \
     * if (cqueue_##NAME##_push(q, value) == 0) {                                                      \
     *     // 插入成功                                                                                  \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE int cqueue_##NAME##_push(struct cqueue_##NAME *q, TYPE val)                           \
    {                                                                                                   \
        return __cqueue_t_push((__cqueue_t *)(q), (const char *)&val, sizeof(TYPE));                    \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 移除队列头部元素                                                                            \
     *                                                                                                  \
     * 从队列头部移除一个元素。移除的元素会被销毁，无法再访问。                                                 \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     *                                                                                                  \
     * @warning 在空队列上调用此函数是未定义行为，使用前应先检查队列是否为空                                      \
     * @note 如果需要获取元素值，应在 pop 前调用 front 函数                                                  \
     * @note 此函数不返回被移除的元素，只是删除它                                                             \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * if (!cqueue_##NAME##_empty(q)) {                                                                \
     *     TYPE val = cqueue_##NAME##_front(q);  // 先获取值                                            \
     *     cqueue_##NAME##_pop(q);               // 再移除                                              \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE void cqueue_##NAME##_pop(struct cqueue_##NAME *q)                                     \
    {                                                                                                   \
        __cqueue_t_pop((__cqueue_t *)(q));                                                              \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 获取队列头部元素（按值返回）                                                                  \
     *                                                                                                  \
     * 返回队列头部元素的副本，不移除该元素。                                                                 \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 队列头部元素的副本                                                                          \
     *                                                                                                  \
     * @warning 在空队列上调用此函数是未定义行为，使用前应先检查队列是否为空                                      \
     * @note 此函数返回元素的副本，对于大型结构体可能影响性能                                                   \
     * @note 如果需要避免复制，使用 front_ptr 函数                                                          \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * if (!cqueue_##NAME##_empty(q)) {                                                                \
     *     TYPE front_val = cqueue_##NAME##_front(q);  // 获取头部元素副本                               \
     *     // 使用 front_val...                                                                         \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE TYPE cqueue_##NAME##_front(struct cqueue_##NAME *q)                                   \
    {                                                                                                   \
        return *((TYPE *)__cqueue_t_front((__cqueue_t *)q));                                            \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 获取队列尾部元素（按值返回）                                                                  \
     *                                                                                                  \
     * 返回队列尾部元素的副本，不移除该元素。                                                                 \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 队列尾部元素的副本                                                                          \
     *                                                                                                  \
     * @warning 在空队列上调用此函数是未定义行为，使用前应先检查队列是否为空                                      \
     * @note 此函数返回元素的副本，对于大型结构体可能影响性能                                                   \
     * @note 如果需要避免复制，使用 back_ptr 函数                                                           \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value1);                                                                \
     * cqueue_##NAME##_push(q, value2);                                                                \
     * if (!cqueue_##NAME##_empty(q)) {                                                                \
     *     TYPE back_val = cqueue_##NAME##_back(q);  // 获取尾部元素副本（value2）                        \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE TYPE cqueue_##NAME##_back(struct cqueue_##NAME *q)                                    \
    {                                                                                                   \
        return *((TYPE *)__cqueue_t_back((__cqueue_t *)q));                                             \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 获取队列头部元素的指针                                                                       \
     *                                                                                                  \
     * 返回指向队列头部元素的指针，可以通过该指针直接访问或修改元素，无需复制。                                     \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 指向队列头部元素的指针                                                                       \
     *                                                                                                  \
     * @warning 在空队列上调用此函数是未定义行为，使用前应先检查队列是否为空                                      \
     * @warning 返回的指针在下次队列操作（如 push、pop）后可能失效，不应长期保存                                  \
     * @note 可以通过返回的指针直接修改元素值                                                                 \
     * @note 比 front 函数更高效，避免了元素复制                                                             \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * if (!cqueue_##NAME##_empty(q)) {                                                                \
     *     TYPE *ptr = cqueue_##NAME##_front_ptr(q);                                                   \
     *     ptr->field = new_value;  // 直接修改元素                                                      \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE TYPE *cqueue_##NAME##_front_ptr(struct cqueue_##NAME *q)                              \
    {                                                                                                   \
        return ((TYPE *)__cqueue_t_front((__cqueue_t *)q));                                             \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 获取队列尾部元素的指针                                                                       \
     *                                                                                                  \
     * 返回指向队列尾部元素的指针，可以通过该指针直接访问或修改元素，无需复制。                                     \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 指向队列尾部元素的指针                                                                       \
     *                                                                                                  \
     * @warning 在空队列上调用此函数是未定义行为，使用前应先检查队列是否为空                                      \
     * @warning 返回的指针在下次队列操作（如 push、pop）后可能失效，不应长期保存                                  \
     * @note 可以通过返回的指针直接修改元素值                                                                 \
     * @note 比 back 函数更高效，避免了元素复制                                                              \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * if (!cqueue_##NAME##_empty(q)) {                                                                \
     *     TYPE *ptr = cqueue_##NAME##_back_ptr(q);                                                    \
     *     ptr->field = new_value;  // 直接修改尾部元素                                                   \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE TYPE *cqueue_##NAME##_back_ptr(struct cqueue_##NAME *q)                               \
    {                                                                                                   \
        return ((TYPE *)__cqueue_t_back((__cqueue_t *)q));                                              \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 获取队列中元素的数量                                                                         \
     *                                                                                                  \
     * 返回队列当前包含的元素个数。                                                                         \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 队列中的元素数量                                                                           \
     *                                                                                                  \
     * @note 时间复杂度为 O(1)                                                                           \
     * @note 空队列返回 0                                                                                \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * printf("队列大小: %zu\n", cqueue_##NAME##_size(q));  // 输出: 0                                   \
     * cqueue_##NAME##_push(q, value1);                                                                \
     * cqueue_##NAME##_push(q, value2);                                                                \
     * printf("队列大小: %zu\n", cqueue_##NAME##_size(q));  // 输出: 2                                   \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE size_t cqueue_##NAME##_size(struct cqueue_##NAME *q)                                  \
    {                                                                                                   \
        return __cqueue_t_size((__cqueue_t *)q);                                                        \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 检查队列是否为空                                                                            \
     *                                                                                                  \
     * 判断队列是否不包含任何元素。                                                                         \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 队列为空返回非 0 值（真），不为空返回 0（假）                                                   \
     *                                                                                                  \
     * @note 时间复杂度为 O(1)                                                                           \
     * @note 在调用 front、back、pop 等操作前，应先检查队列是否为空                                            \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * if (cqueue_##NAME##_empty(q)) {                                                                 \
     *     printf("队列为空\n");                                                                         \
     * }                                                                                               \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * if (!cqueue_##NAME##_empty(q)) {                                                                \
     *     TYPE val = cqueue_##NAME##_front(q);  // 安全访问                                            \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE int cqueue_##NAME##_empty(struct cqueue_##NAME *q)                                    \
    {                                                                                                   \
        return __cqueue_t_empty((__cqueue_t *)q);                                                       \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 清空队列中的所有元素                                                                         \
     *                                                                                                  \
     * 移除队列中的所有元素，释放所有节点占用的内存，但保留队列结构体本身。                                        \
     * 调用后队列大小变为 0。                                                                              \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     *                                                                                                  \
     * @note 如果队列中存储的是指针，需要在 clear 前手动释放指针指向的内存                                       \
     * @note 清空后队列仍可继续使用                                                                         \
     * @note 时间复杂度为 O(n)，n 为队列元素数量                                                             \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value1);                                                                \
     * cqueue_##NAME##_push(q, value2);                                                                \
     * cqueue_##NAME##_clear(q);  // 清空所有元素                                                        \
     * // 队列现在为空，但仍可继续使用                                                                      \
     * cqueue_##NAME##_push(q, value3);                                                                \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE void cqueue_##NAME##_clear(struct cqueue_##NAME *q)                                   \
    {                                                                                                   \
        __cqueue_t_clear((__cqueue_t *)q);                                                              \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 安全获取队列头部元素的指针                                                                    \
     *                                                                                                  \
     * 返回指向队列头部元素的指针。与 front_ptr 不同，此函数在队列为空时安全返回 NULL，                           \
     * 而不会导致未定义行为。                                                                              \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 队列不为空时返回指向头部元素的指针，队列为空时返回 NULL                                           \
     *                                                                                                  \
     * @note 使用前应检查返回值是否为 NULL                                                                  \
     * @note 返回的指针在下次队列操作后可能失效，不应长期保存                                                   \
     * @note 比 front_ptr 更安全，但略有性能开销（需要检查队列是否为空）                                        \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value);                                                                 \
     * TYPE *ptr = cqueue_##NAME##_front_safe(q);                                                      \
     * if (ptr != NULL) {                                                                              \
     *     // 安全使用 ptr                                                                              \
     *     printf("头部元素: %d\n", *ptr);                                                               \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE TYPE *cqueue_##NAME##_front_safe(struct cqueue_##NAME *q)                            \
    {                                                                                                   \
        return ((TYPE *)__cqueue_t_front_safe((__cqueue_t *)q));                                        \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 安全获取队列尾部元素的指针                                                                    \
     *                                                                                                  \
     * 返回指向队列尾部元素的指针。与 back_ptr 不同，此函数在队列为空时安全返回 NULL，                            \
     * 而不会导致未定义行为。                                                                              \
     *                                                                                                  \
     * @param q 指向队列结构体的指针                                                                       \
     * @return 队列不为空时返回指向尾部元素的指针，队列为空时返回 NULL                                           \
     *                                                                                                  \
     * @note 使用前应检查返回值是否为 NULL                                                                  \
     * @note 返回的指针在下次队列操作后可能失效，不应长期保存                                                   \
     * @note 比 back_ptr 更安全，但略有性能开销（需要检查队列是否为空）                                         \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q = cqueue_##NAME##_create();                                                    \
     * cqueue_##NAME##_push(q, value1);                                                                \
     * cqueue_##NAME##_push(q, value2);                                                                \
     * TYPE *ptr = cqueue_##NAME##_back_safe(q);                                                       \
     * if (ptr != NULL) {                                                                              \
     *     // 安全使用 ptr（指向 value2）                                                                \
     *     printf("尾部元素: %d\n", *ptr);                                                               \
     * }                                                                                               \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE TYPE *cqueue_##NAME##_back_safe(struct cqueue_##NAME *q)                             \
    {                                                                                                   \
        return ((TYPE *)__cqueue_t_back_safe((__cqueue_t *)q));                                         \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 交换两个队列的内容                                                                           \
     *                                                                                                  \
     * 高效交换两个队列的所有内容，包括所有元素和内部状态。                                                      \
     * 交换后，q1 将包含原 q2 的内容，q2 将包含原 q1 的内容。                                                  \
     *                                                                                                  \
     * @param q1 指向第一个队列结构体的指针                                                                  \
     * @param q2 指向第二个队列结构体的指针                                                                  \
     *                                                                                                  \
     * @note 时间复杂度为 O(1)，仅交换指针，不复制元素                                                         \
     * @note 两个队列必须是相同类型                                                                         \
     * @note 交换后，原有的迭代器和指针可能失效                                                               \
     *                                                                                                  \
     * @par 使用示例:                                                                                    \
     * @code                                                                                            \
     * cqueue_##NAME *q1 = cqueue_##NAME##_create();                                                   \
     * cqueue_##NAME *q2 = cqueue_##NAME##_create();                                                   \
     * cqueue_##NAME##_push(q1, value1);                                                               \
     * cqueue_##NAME##_push(q2, value2);                                                               \
     * cqueue_##NAME##_swap(q1, q2);                                                                   \
     * // 现在 q1 包含 value2，q2 包含 value1                                                            \
     * @endcode                                                                                         \
     */                                                                                                 \
    static INLINE void cqueue_##NAME##_swap(struct cqueue_##NAME *q1, struct cqueue_##NAME *q2)        \
    {                                                                                                   \
        __cqueue_t_swap((__cqueue_t *)q1, (__cqueue_t *)q2);                                            \
    }

    // 注意：以下部分是库内部函数，用户不要调用-------------------
    
    typedef struct __cqueue_node_t __cqueue_node_t;
    typedef struct __cqueue_t __cqueue_t;

    CC_API __cqueue_t *CC_CALL __cqueue_t_create(size_t queue_size, size_t node_size);
    CC_API void CC_CALL __cqueue_t_destroy(__cqueue_t *q);
    CC_API int CC_CALL __cqueue_t_init(__cqueue_t *q, size_t queue_size, size_t node_size);
    CC_API void CC_CALL __cqueue_t_uninit(__cqueue_t *q);
    CC_API int CC_CALL __cqueue_t_push(__cqueue_t *q, const char *item_ptr, size_t item_size);
    CC_API void CC_CALL __cqueue_t_pop(__cqueue_t *q);
    CC_API char *CC_CALL __cqueue_t_front(__cqueue_t *q);
    CC_API char *CC_CALL __cqueue_t_back(__cqueue_t *q);
    CC_API char *CC_CALL __cqueue_t_front_safe(__cqueue_t *q);
    CC_API char *CC_CALL __cqueue_t_back_safe(__cqueue_t *q);
    CC_API size_t CC_CALL __cqueue_t_size(__cqueue_t *q);
    CC_API int CC_CALL __cqueue_t_empty(__cqueue_t *q);
    CC_API void CC_CALL __cqueue_t_clear(__cqueue_t *q);
    CC_API void CC_CALL __cqueue_t_swap(__cqueue_t *q1, __cqueue_t *q2);

#ifdef __cplusplus
}
#endif

#endif

#ifndef C_CORE_STACK_H_
#define C_CORE_STACK_H_

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
 * @brief 定义一个栈类型
 * 
 * 该宏用于快速定义一个基于TYPE类型的栈数据结构。
 * 生成的栈类型名为 cstack_TYPE，相关函数前缀也为 cstack_TYPE。
 * 
 * @param TYPE 栈中存储的元素类型
 * 
 * @par 使用示例:
 * @code
 * DEFINE_CSTACK(int);  // 定义int类型的栈
 * 
 * cstack_int *stack = cstack_int_create();
 * cstack_int_push(stack, 10);
 * cstack_int_push(stack, 20);
 * int value = cstack_int_top(stack);  // value = 20
 * cstack_int_destroy(&stack);
 * @endcode
 */
#define DEFINE_CSTACK(TYPE) DEFINE_CSTACK_AS(TYPE, TYPE)

/**
 * @brief 定义一个指针类型的栈
 * 
 * 该宏用于快速定义一个存储TYPE指针的栈数据结构。
 * 生成的栈类型名为 cstack_TYPE_ptr，相关函数前缀也为 cstack_TYPE_ptr。
 * 
 * @param TYPE 栈中存储的指针所指向的类型
 * 
 * @par 使用示例:
 * @code
 * typedef struct MyData { int id; } MyData;
 * DEFINE_CSTACK_PTR(MyData);  // 定义MyData*类型的栈
 * 
 * cstack_MyData_ptr *stack = cstack_MyData_ptr_create();
 * MyData *data = malloc(sizeof(MyData));
 * data->id = 100;
 * cstack_MyData_ptr_push(stack, data);
 * MyData *top = cstack_MyData_ptr_top(stack);
 * cstack_MyData_ptr_destroy(&stack);
 * @endcode
 */
#define DEFINE_CSTACK_PTR(TYPE) DEFINE_CSTACK_AS(TYPE *, TYPE##_ptr)

/**
 * @brief 定义一个指针类型的栈，并指定名称
 * 
 * 该宏用于定义一个存储TYPE指针的栈数据结构，并可以自定义栈类型的名称。
 * 生成的栈类型名为 cstack_NAME，相关函数前缀也为 cstack_NAME。
 * 
 * @param TYPE 栈中存储的指针所指向的类型
 * @param NAME 自定义的栈类型名称后缀
 * 
 * @par 使用示例:
 * @code
 * typedef struct MyData { int id; } MyData;
 * DEFINE_CSTACK_PTR_AS(MyData, my_ptr_stack);  // 定义MyData*类型的栈，名为my_ptr_stack
 * 
 * cstack_my_ptr_stack *stack = cstack_my_ptr_stack_create();
 * MyData *data = malloc(sizeof(MyData));
 * cstack_my_ptr_stack_push(stack, data);
 * cstack_my_ptr_stack_destroy(&stack);
 * @endcode
 */
#define DEFINE_CSTACK_PTR_AS(TYPE, NAME) DEFINE_CSTACK_AS(TYPE *, NAME)

/**
 * @brief 定义一个栈类型，并指定名称
 * 
 * 该宏是栈定义的核心实现，用于定义一个基于TYPE类型的栈数据结构，并可以自定义栈类型的名称。
 * 该宏会生成完整的栈结构体定义和所有相关的操作函数。
 * 
 * @param TYPE 栈中存储的元素类型
 * @param NAME 自定义的栈类型名称后缀
 * 
 * @par 生成的类型和函数:
 * - cstack_NAME: 栈结构体类型
 * - cstack_NAME_create(): 创建栈
 * - cstack_NAME_destroy(): 销毁栈
 * - cstack_NAME_init(): 初始化栈
 * - cstack_NAME_uninit(): 反初始化栈
 * - cstack_NAME_push(): 压栈
 * - cstack_NAME_pop(): 出栈
 * - cstack_NAME_top(): 获取栈顶元素
 * - cstack_NAME_top_ptr(): 获取栈顶元素指针
 * - cstack_NAME_top_safe(): 安全获取栈顶元素指针
 * - cstack_NAME_size(): 获取栈大小
 * - cstack_NAME_empty(): 判断栈是否为空
 * - cstack_NAME_clear(): 清空栈
 * - cstack_NAME_swap(): 交换两个栈
 * 
 * @par 使用示例:
 * @code
 * DEFINE_CSTACK_AS(double, real_stack);  // 定义double类型的栈，名为real_stack
 * 
 * cstack_real_stack *stack = cstack_real_stack_create();
 * cstack_real_stack_push(stack, 3.14);
 * cstack_real_stack_push(stack, 2.71);
 * double value = cstack_real_stack_top(stack);  // value = 2.71
 * cstack_real_stack_pop(stack);
 * cstack_real_stack_destroy(&stack);
 * @endcode
 */
#define DEFINE_CSTACK_AS(TYPE, NAME)                                                                    \
    typedef struct cstack_##NAME##_node                                                                 \
    {                                                                                                   \
        TYPE *value_ptr;                                                                                \
        struct cstack_##NAME##_node *next;                                                              \
    } cstack_##NAME##_node;                                                                             \
                                                                                                        \
    typedef cstack_##NAME##_node *cstack_##NAME##_iter;                                                 \
                                                                                                        \
    typedef struct cstack_##NAME                                                                        \
    {                                                                                                   \
        unsigned int size;                                                                              \
        cstack_##NAME##_node *top;                                                                      \
    } cstack_##NAME;                                                                                    \
                                                                                                        \
    /**                                                                                                 \
     * @brief 创建一个栈对象                                                                            \
     *                                                                                                  \
     * 动态分配内存并初始化一个新的栈对象。使用完毕后必须调用destroy函数释放内存。                        \
     *                                                                                                  \
     * @return cstack_##NAME* 成功返回指向新栈的指针，失败返回NULL                                       \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * if (stack) {                                                                                     \
     *     // 使用栈                                                                                    \
     *     cstack_int_destroy(&stack);                                                                  \
     * }                                                                                                \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 创建失败通常是由于内存不足                                                                  \
     * @see cstack_##NAME##_destroy                                                                     \
     */                                                                                                 \
    static INLINE cstack_##NAME *cstack_##NAME##_create()                                               \
    {                                                                                                   \
        return (cstack_##NAME *)__cstack_t_create(sizeof(cstack_##NAME), sizeof(cstack_##NAME##_node)); \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 销毁栈对象并释放内存                                                                       \
     *                                                                                                  \
     * 释放栈占用的所有内存资源，包括栈中的所有节点。销毁后会将栈指针设置为NULL。                          \
     * 该函数会自动清空栈中的所有元素。                                                                  \
     *                                                                                                  \
     * @param pq 指向栈指针的指针，销毁后会被设置为NULL                                                   \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * cstack_int_push(stack, 10);                                                                      \
     * cstack_int_destroy(&stack);  // stack现在为NULL                                                  \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 如果传入NULL或指向NULL的指针，函数不会执行任何操作                                            \
     * @warning 销毁后不要再使用该栈指针                                                                 \
     * @see cstack_##NAME##_create                                                                      \
     */                                                                                                 \
    static INLINE void cstack_##NAME##_destroy(cstack_##NAME **pq)                                      \
    {                                                                                                   \
        if (pq && *pq)                                                                                  \
        {                                                                                               \
            __cstack_t_destroy((__cstack_t *)(*pq));                                                    \
            *pq = 0;                                                                                    \
        }                                                                                               \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 初始化一个栈对象                                                                          \
     *                                                                                                  \
     * 初始化一个已分配的栈对象（如栈上分配或嵌入在其他结构中的栈）。                                      \
     * 与create函数不同，init不分配栈本身的内存，只初始化其内部状态。                                     \
     * 使用完毕后应调用uninit函数清理资源。                                                             \
     *                                                                                                  \
     * @param q 指向待初始化的栈对象的指针                                                               \
     * @return int 成功返回0，失败返回非0值                                                              \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int stack;  // 栈上分配                                                                   \
     * if (cstack_int_init(&stack) == 0) {                                                              \
     *     cstack_int_push(&stack, 10);                                                                 \
     *     cstack_int_uninit(&stack);                                                                   \
     * }                                                                                                \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 适用于栈对象在栈上分配或嵌入其他结构体的情况                                                  \
     * @see cstack_##NAME##_uninit, cstack_##NAME##_create                                              \
     */                                                                                                 \
    static INLINE int cstack_##NAME##_init(struct cstack_##NAME *q)                                     \
    {                                                                                                   \
        return __cstack_t_init((__cstack_t *)(q), sizeof(cstack_##NAME), sizeof(cstack_##NAME##_node)); \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 反初始化栈对象并释放内部资源                                                                \
     *                                                                                                  \
     * 清理栈占用的内部资源（如动态分配的节点），但不释放栈对象本身的内存。                                \
     * 与destroy函数不同，uninit不释放栈结构体本身的内存。                                                \
     * 该函数应与init函数配对使用。                                                                      \
     *                                                                                                  \
     * @param q 指向待反初始化的栈对象的指针                                                              \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int stack;                                                                                \
     * cstack_int_init(&stack);                                                                         \
     * cstack_int_push(&stack, 10);                                                                     \
     * cstack_int_uninit(&stack);  // 清理内部资源，stack本身仍然有效（但内部已清空）                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 适用于通过init初始化的栈对象                                                                 \
     * @see cstack_##NAME##_init, cstack_##NAME##_destroy                                               \
     */                                                                                                 \
    static INLINE void cstack_##NAME##_uninit(struct cstack_##NAME *q)                                  \
    {                                                                                                   \
        __cstack_t_uninit((__cstack_t *)(q));                                                           \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 将元素压入栈顶                                                                            \
     *                                                                                                  \
     * 将一个元素添加到栈的顶部。该操作会复制元素的值到栈中。                                             \
     * 压栈操作的时间复杂度为O(1)。                                                                      \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     * @param val 要压入栈的元素值                                                                       \
     * @return int 成功返回0，失败返回非0值（通常是内存分配失败）                                          \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * if (cstack_int_push(stack, 10) == 0) {                                                           \
     *     printf("压栈成功\n");                                                                        \
     * }                                                                                                \
     * cstack_int_push(stack, 20);                                                                      \
     * cstack_int_push(stack, 30);  // 栈顶现在是30                                                     \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 元素是值拷贝，不是引用                                                                       \
     * @warning 如果内存不足，压栈操作会失败                                                              \
     * @see cstack_##NAME##_pop, cstack_##NAME##_top                                                    \
     */                                                                                                 \
    static INLINE int cstack_##NAME##_push(struct cstack_##NAME *q, TYPE val)                          \
    {                                                                                                   \
        return __cstack_t_push((__cstack_t *)(q), (const char *)&val, sizeof(TYPE));                    \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 弹出栈顶元素                                                                              \
     *                                                                                                  \
     * 移除栈顶的元素并释放其占用的内存。该操作不返回被弹出的元素值。                                      \
     * 如果需要获取栈顶元素的值，应在调用pop之前先调用top函数。                                           \
     * 弹栈操作的时间复杂度为O(1)。                                                                      \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * cstack_int_push(stack, 10);                                                                      \
     * cstack_int_push(stack, 20);                                                                      \
     * int val = cstack_int_top(stack);  // 先获取栈顶值：20                                            \
     * cstack_int_pop(stack);  // 弹出20                                                                \
     * val = cstack_int_top(stack);  // 现在栈顶是10                                                    \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @warning 对空栈调用pop会导致未定义行为，使用前应先检查栈是否为空                                    \
     * @see cstack_##NAME##_push, cstack_##NAME##_top, cstack_##NAME##_empty                            \
     */                                                                                                 \
    static INLINE void cstack_##NAME##_pop(struct cstack_##NAME *q)                                     \
    {                                                                                                   \
        __cstack_t_pop((__cstack_t *)(q));                                                              \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 获取栈顶元素的值                                                                          \
     *                                                                                                  \
     * 返回栈顶元素的值（副本），不移除该元素。该操作的时间复杂度为O(1)。                                  \
     * 该函数会解引用栈顶元素指针并返回其值。                                                            \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     * @return TYPE 栈顶元素的值                                                                         \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * cstack_int_push(stack, 10);                                                                      \
     * cstack_int_push(stack, 20);                                                                      \
     * int val = cstack_int_top(stack);  // val = 20                                                    \
     * cstack_int_pop(stack);                                                                           \
     * val = cstack_int_top(stack);  // val = 10                                                        \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @warning 对空栈调用top会导致未定义行为（访问空指针），使用前应先检查栈是否为空                      \
     * @note 返回的是元素的副本，不是引用                                                                 \
     * @see cstack_##NAME##_top_ptr, cstack_##NAME##_top_safe, cstack_##NAME##_empty                    \
     */                                                                                                 \
    static INLINE TYPE cstack_##NAME##_top(struct cstack_##NAME *q)                                     \
    {                                                                                                   \
        return *((TYPE *)__cstack_t_top((__cstack_t *)q));                                              \
    }                                                                                                   \
    /**                                                                                                 \
     * @brief 获取栈顶元素的指针                                                                         \
     *                                                                                                  \
     * 返回指向栈顶元素的指针，可通过该指针直接访问或修改栈顶元素。                                        \
     * 该操作不移除栈顶元素，时间复杂度为O(1)。                                                           \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     * @return TYPE* 指向栈顶元素的指针                                                                  \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * typedef struct Point { int x, y; } Point;                                                        \
     * DEFINE_CSTACK(Point);                                                                            \
     * cstack_Point *stack = cstack_Point_create();                                                     \
     * Point p = {10, 20};                                                                              \
     * cstack_Point_push(stack, p);                                                                     \
     * Point *ptr = cstack_Point_top_ptr(stack);                                                        \
     * ptr->x = 30;  // 直接修改栈顶元素                                                                \
     * cstack_Point_destroy(&stack);                                                                    \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @warning 对空栈调用此函数会导致未定义行为，使用前应先检查栈是否为空                                 \
     * @warning 返回的指针在pop操作后会失效，不要保存该指针长期使用                                       \
     * @note 可以通过返回的指针直接修改栈中的元素                                                         \
     * @see cstack_##NAME##_top, cstack_##NAME##_top_safe                                               \
     */                                                                                                 \
    static INLINE TYPE *cstack_##NAME##_top_ptr(struct cstack_##NAME *q)                                \
    {                                                                                                   \
        return ((TYPE *)__cstack_t_top((__cstack_t *)q));                                               \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 安全地获取栈顶元素的指针                                                                   \
     *                                                                                                  \
     * 返回指向栈顶元素的指针，如果栈为空则返回NULL。                                                     \
     * 这是top_ptr函数的安全版本，会检查栈是否为空。                                                     \
     * 该操作的时间复杂度为O(1)。                                                                        \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     * @return TYPE* 指向栈顶元素的指针，如果栈为空则返回NULL                                             \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * int *ptr = cstack_int_top_safe(stack);                                                           \
     * if (ptr == NULL) {                                                                               \
     *     printf("栈为空\n");                                                                          \
     * }                                                                                                \
     * cstack_int_push(stack, 10);                                                                      \
     * ptr = cstack_int_top_safe(stack);                                                                \
     * if (ptr) {                                                                                       \
     *     printf("栈顶元素: %d\n", *ptr);                                                              \
     * }                                                                                                \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 使用前总是应该检查返回值是否为NULL                                                           \
     * @warning 返回的指针在pop操作后会失效，不要保存该指针长期使用                                       \
     * @see cstack_##NAME##_top_ptr, cstack_##NAME##_top, cstack_##NAME##_empty                         \
     */                                                                                                 \
    static INLINE TYPE *cstack_##NAME##_top_safe(struct cstack_##NAME *q)                               \
    {                                                                                                   \
        return ((TYPE *)__cstack_t_top_safe((__cstack_t *)q));                                          \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 获取栈中元素的个数                                                                         \
     *                                                                                                  \
     * 返回栈中当前存储的元素数量。空栈返回0。                                                            \
     * 该操作的时间复杂度为O(1)。                                                                        \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     * @return size_t 栈中元素的个数                                                                     \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * printf("size: %zu\n", cstack_int_size(stack));  // 输出: size: 0                                \
     * cstack_int_push(stack, 10);                                                                      \
     * cstack_int_push(stack, 20);                                                                      \
     * printf("size: %zu\n", cstack_int_size(stack));  // 输出: size: 2                                \
     * cstack_int_pop(stack);                                                                           \
     * printf("size: %zu\n", cstack_int_size(stack));  // 输出: size: 1                                \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 该函数总是安全的，不会失败                                                                   \
     * @see cstack_##NAME##_empty                                                                       \
     */                                                                                                 \
    static INLINE size_t cstack_##NAME##_size(struct cstack_##NAME *q)                                  \
    {                                                                                                   \
        return __cstack_t_size((__cstack_t *)q);                                                        \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 检查栈是否为空                                                                            \
     *                                                                                                  \
     * 检查栈中是否没有任何元素。该操作的时间复杂度为O(1)。                                               \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     * @return int 栈为空返回非0值（真），栈不为空返回0（假）                                             \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * if (cstack_int_empty(stack)) {                                                                   \
     *     printf("栈为空\n");                                                                          \
     * }                                                                                                \
     * cstack_int_push(stack, 10);                                                                      \
     * if (!cstack_int_empty(stack)) {                                                                  \
     *     printf("栈不为空\n");                                                                        \
     * }                                                                                                \
     * // 安全的pop操作                                                                                 \
     * while (!cstack_int_empty(stack)) {                                                               \
     *     int val = cstack_int_top(stack);                                                             \
     *     cstack_int_pop(stack);                                                                       \
     * }                                                                                                \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 在调用top或pop之前应先使用此函数检查栈是否为空                                               \
     * @see cstack_##NAME##_size                                                                        \
     */                                                                                                 \
    static INLINE int cstack_##NAME##_empty(struct cstack_##NAME *q)                                    \
    {                                                                                                   \
        return __cstack_t_empty((__cstack_t *)q);                                                       \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 清空栈中的所有元素                                                                         \
     *                                                                                                  \
     * 移除栈中的所有元素并释放它们占用的内存，但保留栈对象本身。                                          \
     * 清空后栈的大小变为0。该操作的时间复杂度为O(n)，n为栈中元素个数。                                   \
     *                                                                                                  \
     * @param q 指向栈对象的指针                                                                         \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack = cstack_int_create();                                                         \
     * cstack_int_push(stack, 10);                                                                      \
     * cstack_int_push(stack, 20);                                                                      \
     * cstack_int_push(stack, 30);                                                                      \
     * printf("size: %zu\n", cstack_int_size(stack));  // 输出: size: 3                                \
     * cstack_int_clear(stack);                                                                         \
     * printf("size: %zu\n", cstack_int_size(stack));  // 输出: size: 0                                \
     * // 清空后可以继续使用                                                                            \
     * cstack_int_push(stack, 40);                                                                      \
     * cstack_int_destroy(&stack);                                                                      \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 清空后栈对象仍然有效，可以继续使用                                                           \
     * @see cstack_##NAME##_destroy, cstack_##NAME##_pop                                                \
     */                                                                                                 \
    static INLINE void cstack_##NAME##_clear(struct cstack_##NAME *q)                                   \
    {                                                                                                   \
        __cstack_t_clear((__cstack_t *)q);                                                              \
    }                                                                                                   \
                                                                                                        \
    /**                                                                                                 \
     * @brief 交换两个栈的内容                                                                          \
     *                                                                                                  \
     * 交换两个栈中的所有元素和状态，不进行元素的复制。                                                   \
     * 交换后，q1包含原q2的内容，q2包含原q1的内容。                                                      \
     * 该操作的时间复杂度为O(1)，非常高效。                                                              \
     *                                                                                                  \
     * @param q1 指向第一个栈对象的指针                                                                  \
     * @param q2 指向第二个栈对象的指针                                                                  \
     *                                                                                                  \
     * @par 使用示例:                                                                                   \
     * @code                                                                                            \
     * cstack_int *stack1 = cstack_int_create();                                                        \
     * cstack_int *stack2 = cstack_int_create();                                                        \
     * cstack_int_push(stack1, 10);                                                                     \
     * cstack_int_push(stack1, 20);  // stack1: [10, 20]                                               \
     * cstack_int_push(stack2, 30);                                                                     \
     * cstack_int_push(stack2, 40);                                                                     \
     * cstack_int_push(stack2, 50);  // stack2: [30, 40, 50]                                           \
     * cstack_int_swap(stack1, stack2);                                                                 \
     * // 交换后：stack1: [30, 40, 50], stack2: [10, 20]                                               \
     * printf("stack1 size: %zu\n", cstack_int_size(stack1));  // 输出: 3                              \
     * printf("stack2 size: %zu\n", cstack_int_size(stack2));  // 输出: 2                              \
     * cstack_int_destroy(&stack1);                                                                     \
     * cstack_int_destroy(&stack2);                                                                     \
     * @endcode                                                                                         \
     *                                                                                                  \
     * @note 这是一个非常高效的操作，只交换内部指针，不复制元素                                          \
     * @note 两个栈必须是相同类型的                                                                      \
     */                                                                                                 \
    static INLINE void cstack_##NAME##_swap(struct cstack_##NAME *q1, struct cstack_##NAME *q2)        \
    {                                                                                                   \
        __cstack_t_swap((__cstack_t *)q1, (__cstack_t *)q2);                                            \
    }


    // 注意：以下部分是库内部函数，用户不要调用-------------------

    
    typedef struct __cstack_node_t __cstack_node_t;
    typedef struct __cstack_t __cstack_t;

    CC_API __cstack_t *CC_CALL __cstack_t_create(size_t stack_size, size_t node_size);
    CC_API void CC_CALL __cstack_t_destroy(__cstack_t *q);
    CC_API int CC_CALL __cstack_t_init(__cstack_t *q, size_t stack_size, size_t node_size);
    CC_API void CC_CALL __cstack_t_uninit(__cstack_t *q);

    CC_API int CC_CALL __cstack_t_push(__cstack_t *q, const char *item_ptr, size_t item_size);
    CC_API void CC_CALL __cstack_t_pop(__cstack_t *q);
    CC_API char *CC_CALL __cstack_t_top(__cstack_t *q);
    CC_API char *CC_CALL __cstack_t_top_safe(__cstack_t *q);
    CC_API size_t CC_CALL __cstack_t_size(__cstack_t *q);
    CC_API int CC_CALL __cstack_t_empty(__cstack_t *q);
    CC_API void CC_CALL __cstack_t_clear(__cstack_t *q);
    CC_API void CC_CALL __cstack_t_swap(__cstack_t *q1, __cstack_t *q2);

#ifdef __cplusplus
}
#endif

#endif

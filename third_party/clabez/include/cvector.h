#ifndef C_CORE_VECTOR_H_
#define C_CORE_VECTOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <string.h>
#include "cvector_config.h"
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
 * @def DEFINE_CVECTOR(TYPE)
 * @brief 定义一个类型为TYPE的cvector动态数组容器
 * @details 这是最常用的宏，用于为指定类型生成完整的cvector容器定义。
 *          使用 DEFINE_CVECTOR(int) 将生成 cvector_int 容器类型，
 *          以及配套的所有函数：创建、销毁、插入、删除、查找、排序等。
 *          这是一个功能完整的动态数组实现，自动管理内存，支持动态扩容。
 * @param TYPE 元素类型，支持基本类型（int, float, double等）和自定义结构体
 * @note 容器名称自动为 cvector_TYPE，函数名为 cvector_TYPE_xxx
 * @see DEFINE_CVECTOR_AS, DEFINE_CVECTOR_PTR
 *
 * 使用示例：
 * @code
 * // 定义int类型的容器
 * DEFINE_CVECTOR(int)
 *
 * // 使用容器
 * cvector_int* vec = cvector_int_create();        // 创建容器
 * cvector_int_push_back(vec, 10);                 // 添加元素
 * cvector_int_push_back(vec, 20);
 * int val = cvector_int_at(vec, 0);               // 访问元素，val = 10
 * size_t sz = cvector_int_size(vec);              // 获取大小，sz = 2
 * cvector_int_destroy(&vec);                      // 销毁容器
 *
 * // 定义自定义结构体的容器
 * typedef struct { int x; int y; } Point;
 * DEFINE_CVECTOR(Point)
 *
 * cvector_Point* points = cvector_Point_create();
 * Point p1 = {1, 2};
 * cvector_Point_push_back(points, p1);
 * cvector_Point_destroy(&points);
 * @endcode
 */
#define DEFINE_CVECTOR(TYPE) DEFINE_CVECTOR_AS(TYPE, TYPE)

/**
 * @def DEFINE_CVECTOR_PTR(TYPE)
 * @brief 定义一个存储TYPE指针的cvector容器
 * @details 这是一个便捷宏，用于快速定义存储指针类型的容器。
 *          例如 DEFINE_CVECTOR_PTR(int) 将定义一个 cvector_int_ptr 容器，用于存储 int* 指针。
 *          然后就可以使用 cvector_int_ptr_create, cvector_int_ptr_push_back 等函数。
 * @param TYPE 基础类型，如：int, float, 自定义结构体等（不包含*号）
 * @note 容器名称会自动添加 _ptr 后缀，例如：TYPE=int -> cvector_int_ptr
 * @see DEFINE_CVECTOR, DEFINE_CVECTOR_PTR_AS
 *
 * 使用示例：
 * @code
 * DEFINE_CVECTOR_PTR(int)  // 定义 cvector_int_ptr 容器
 *
 * cvector_int_ptr* vec = cvector_int_ptr_create();
 * int value = 42;
 * cvector_int_ptr_push_back(vec, &value);  // 存储int*指针
 * int* ptr = cvector_int_ptr_at(vec, 0);   // 获取指针
 * cvector_int_ptr_destroy(&vec);
 * @endcode
 */
#define DEFINE_CVECTOR_PTR(TYPE) DEFINE_CVECTOR_AS(TYPE *, TYPE##_ptr)

/**
 * @def DEFINE_CVECTOR_PTR_AS(TYPE, NAME)
 * @brief 定义一个存储TYPE指针的cvector容器，并自定义容器名称
 * @details 这是一个灵活的宏，用于定义存储指针类型的容器并指定自定义名称。
 *          与 DEFINE_CVECTOR_PTR 不同，此宏允许完全自定义容器名称，而不是自动添加 _ptr 后缀。
 * @param TYPE 基础类型，如：int, float, 自定义结构体等（不包含*号）
 * @param NAME 自定义容器名称，将生成 cvector_NAME 类型和相关函数
 * @note 如果不需要自定义名称，推荐使用更简洁的 DEFINE_CVECTOR_PTR 宏
 * @see DEFINE_CVECTOR_PTR, DEFINE_CVECTOR_AS
 *
 * 使用示例：
 * @code
 * DEFINE_CVECTOR_PTR_AS(int, int_pointer_vec)  // 定义 cvector_int_pointer_vec 容器
 *
 * cvector_int_pointer_vec* vec = cvector_int_pointer_vec_create();
 * int value = 42;
 * cvector_int_pointer_vec_push_back(vec, &value);
 * int* ptr = cvector_int_pointer_vec_at(vec, 0);
 * cvector_int_pointer_vec_destroy(&vec);
 * @endcode
 */
#define DEFINE_CVECTOR_PTR_AS(TYPE, NAME) DEFINE_CVECTOR_AS(TYPE *, NAME)

/**
 * @def DEFINE_CVECTOR_AS(TYPE, NAME)
 * @brief 定义一个类型为TYPE的cvector容器，并自定义容器名称
 * @details 这是最底层、最灵活的宏，允许完全自定义元素类型和容器名称。
 *          会生成 cvector_NAME 结构体类型以及完整的函数集：
 *          - 创建/销毁：create, destroy, init, uninit
 *          - 容量管理：size, capacity, reserve, shrink_to_fit
 *          - 元素访问：at, front, back, data, begin, end
 *          - 修改操作：push_back, pop_back, insert, erase, clear
 *          - 算法：sort, find, contains, reverse
 *          - 其他：copy, swap, assign, append, equals
 *          这是一个功能完整的动态数组实现，提供自动扩容和多种操作。
 * @param TYPE 元素类型，可以是任意类型（包括指针类型如 int*）
 * @param NAME 容器名称，将生成 cvector_NAME 类型和 cvector_NAME_xxx 函数
 * @note 对于简单场景，推荐使用 DEFINE_CVECTOR 或 DEFINE_CVECTOR_PTR
 * @see DEFINE_CVECTOR, DEFINE_CVECTOR_PTR
 *
 * 使用示例：
 * @code
 * // 为int类型定义名为my_int_vec的容器
 * DEFINE_CVECTOR_AS(int, my_int_vec)
 *
 * cvector_my_int_vec* vec = cvector_my_int_vec_create();
 * cvector_my_int_vec_push_back(vec, 100);
 * cvector_my_int_vec_destroy(&vec);
 *
 * // 为指针类型定义容器
 * typedef struct Node { int data; struct Node* next; } Node;
 * DEFINE_CVECTOR_AS(Node*, node_ptr_list)
 *
 * cvector_node_ptr_list* list = cvector_node_ptr_list_create();
 * Node* node = malloc(sizeof(Node));
 * cvector_node_ptr_list_push_back(list, node);
 * cvector_node_ptr_list_destroy(&list);
 * @endcode
 */
#define DEFINE_CVECTOR_AS(TYPE, NAME)                                                                                                       \
                                                                                                                                            \
    typedef struct cvector_##NAME                                                                                                           \
    {                                                                                                                                       \
        TYPE *data;                  /**< 数据指针，指向连续存储的元素数组 */                                                               \
        size_t size;                 /**< 当前元素数量 */                                                                                   \
        size_t capacity;             /**< 当前已分配的容量 */                                                                               \
        size_t item_size;            /* 保存元素大小，用于内存跟踪 */                                                                       \
        cvector_error_t last_error;  /* 每个向量独立的错误码（线程安全） */                                                                 \
        unsigned char is_fixed : 1;  /* 固定模式标志：1=使用外部固定缓冲区，0=动态分配 */                                                   \
        unsigned char owns_data : 1; /* 所有权标志：1=拥有data的所有权，0=不拥有（外部管理） */                                             \
        unsigned char reserved : 6;  /* 保留位 */                                                                                           \
    } cvector_##NAME;                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 比较回调函数类型定义                                                                                                \
     * @details 用于自定义排序时的元素比较函数                                                                               \
     * @param lhs 左操作数指针                                                                                                        \
     * @param rhs 右操作数指针                                                                                                        \
     * @return 返回值< 0表示lhs < rhs，返回0表示相等，返回值> 0表示lhs > rhs                                              \
     */                                                                                                                                     \
    typedef int (*cvector_##NAME##_cmp_cb)(const TYPE *lhs, const TYPE *rhs);                                                               \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 在堆上创建一个新的动态数组容器                                                                                 \
     * @details 使用malloc在堆上分配内存并初始化容器，默认初始容量为4个元素。                                    \
     *          使用完毕后必须调用对应的destroy函数释放内存，避免内存泄漏。                                       \
     * @return 返回指向新容器的指针，如果分配失败返回NULL                                                                \
     * @note 该函数分配的内存必须使用cvector_##NAME##_destroy释放                                                             \
     * @see cvector_##NAME##_create_with_capacity, cvector_##NAME##_destroy                                                                 \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * if (vec != NULL) {                                                                                                                   \
     *     cvector_int_push_back(vec, 10);                                                                                                  \
     *     cvector_int_destroy(&vec);                                                                                                       \
     * }                                                                                                                                    \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE cvector_##NAME *cvector_##NAME##_create()                                                                                 \
    {                                                                                                                                       \
        return (cvector_##NAME *)__vector_create(sizeof(TYPE));                                                                             \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 在堆上创建一个指定初始容量的动态数组容器                                                                  \
     * @details 使用malloc在堆上分配内存，可以预先分配足够的容量避免频繁扩容。                                  \
     *          这对于预知元素数量的场景特别有用，可以提高性能。                                                    \
     *          使用完毕后必须调用对应的destroy函数释放内存。                                                            \
     * @param capacity 初始容量大小，如果为0则自动设置为1                                                                   \
     * @return 返回指向新容器的指针，如果分配失败返回NULL                                                                \
     * @note 该函数分配的内存必须使用cvector_##NAME##_destroy释放                                                             \
     * @see cvector_##NAME##_create, cvector_##NAME##_destroy                                                                               \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create_with_capacity(100);  // 预分配100个元素的空间                                         \
     * for (int i = 0; i < 100; i++) {                                                                                                      \
     *     cvector_int_push_back(vec, i);  // 不会触发扩容，性能更好                                                             \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE cvector_##NAME *cvector_##NAME##_create_with_capacity(                                                                    \
        size_t capacity)                                                                                                                    \
    {                                                                                                                                       \
        return (cvector_##NAME *)__vector_create_with_capacity(capacity,                                                                    \
                                                               sizeof(TYPE));                                                               \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 初始化栈上或静态分配的容器                                                                                       \
     * @details 用于初始化已分配空间的容器对象（如栈上变量或静态变量）。                                        \
     *          容器本身的内存由用户管理，但容器内部数据会在堆上分配，默认初始容量为4。                  \
     *          使用完毕后必须调用uninit清理内部资源。                                                                      \
     * @param vec 指向待初始化的容器指针，不能为NULL                                                                         \
     * @note 必须与cvector_##NAME##_uninit配对使用，不能用destroy                                                                \
     * @see cvector_##NAME##_init_with_capacity, cvector_##NAME##_uninit                                                                    \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int vec;  // 栈上分配容器结构                                                                                        \
     * cvector_int_init(&vec);                                                                                                              \
     * cvector_int_push_back(&vec, 42);                                                                                                     \
     * cvector_int_uninit(&vec);  // 必须调用uninit清理，不能调用destroy                                                         \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_init(cvector_##NAME *vec)                                                                           \
    {                                                                                                                                       \
        __vector_init((__cvector_t *)vec, sizeof(TYPE));                                                                                    \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 初始化栈上或静态分配的容器，并指定初始容量                                                               \
     * @details 用于初始化已分配空间的容器对象并预留指定容量。                                                       \
     *          容器本身的内存由用户管理（如栈上变量），但容器内部数据会在堆上分配。                      \
     *          使用完毕后必须调用uninit清理内部资源。                                                                      \
     * @param vec 指向待初始化的容器指针，不能为NULL                                                                         \
     * @param capacity 初始容量大小，如果为0则自动设置为1                                                                   \
     * @note 必须与cvector_##NAME##_uninit配对使用，不能用destroy                                                                \
     * @see cvector_##NAME##_init, cvector_##NAME##_uninit                                                                                  \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int vec;  // 栈上分配                                                                                                    \
     * cvector_int_init_with_capacity(&vec, 1000);  // 预分配1000个元素空间                                                         \
     * for (int i = 0; i < 1000; i++) {                                                                                                     \
     *     cvector_int_push_back(&vec, i);                                                                                                  \
     * }                                                                                                                                    \
     * cvector_int_uninit(&vec);                                                                                                            \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_init_with_capacity(cvector_##NAME *vec,                                                             \
                                                           size_t capacity)                                                                 \
    {                                                                                                                                       \
        __vector_init_with_capacity((__cvector_t *)vec, capacity, sizeof(TYPE));                                                            \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 反初始化容器，释放内部资源                                                                                       \
     * @details 释放容器内部数据占用的堆内存，但不释放容器本身的内存。                                           \
     *          容器本身的内存由用户管理（如栈上或静态变量）。调用后容器不能再使用。                      \
     * @param vec 指向待反初始化的容器指针                                                                                      \
     * @note 仅用于init初始化的容器，对于create创建的容器应使用destroy                                                  \
     * @see cvector_##NAME##_init, cvector_##NAME##_destroy                                                                                 \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * void some_function() {                                                                                                               \
     *     cvector_int vec;                                                                                                                 \
     *     cvector_int_init(&vec);                                                                                                          \
     *     cvector_int_push_back(&vec, 100);                                                                                                \
     *     cvector_int_uninit(&vec);  // 释放内部数据，vec本身在函数结束时自动回收                                       \
     * }                                                                                                                                    \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_uninit(cvector_##NAME *vec)                                                                         \
    {                                                                                                                                       \
        __vector_uninit((__cvector_t *)(vec));                                                                                              \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 销毁容器并释放所有资源                                                                                             \
     * @details 使用free释放容器内部数据和容器本身占用的所有堆内存。                                                \
     *          调用后会将指针置为NULL，避免悬空指针问题。                                                               \
     * @param vec 指向容器指针的指针（二级指针），允许为NULL或*vec为NULL                                               \
     * @note 仅用于create创建的容器，对于init初始化的容器应使用uninit                                                   \
     * @see cvector_##NAME##_create, cvector_##NAME##_uninit                                                                                \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_destroy(&vec);  // 释放所有资源，vec被置为NULL                                                                 \
     * // vec现在是NULL，可以安全地再次调用destroy                                                                             \
     * cvector_int_destroy(&vec);  // 安全，不会有问题                                                                              \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_destroy(cvector_##NAME **vec)                                                                       \
    {                                                                                                                                       \
        if (vec && *vec)                                                                                                                    \
        {                                                                                                                                   \
            __vector_destroy((__cvector_t *)(*vec));                                                                                        \
            *vec = 0;                                                                                                                       \
        }                                                                                                                                   \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 获取容器中当前元素的数量                                                                                          \
     * @details 返回容器中实际存储的元素个数。这是一个快速操作，时间复杂度O(1)。                              \
     * @param vec 容器指针，允许为NULL                                                                                              \
     * @return 返回当前元素数量，如果vec为NULL则返回0                                                                        \
     * @see cvector_##NAME##_capacity, cvector_##NAME##_empty                                                                               \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 0                                                                  \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 2                                                                  \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE size_t cvector_##NAME##_size(cvector_##NAME *vec)                                                                         \
    {                                                                                                                                       \
        return __vector_size((__cvector_t *)vec);                                                                                           \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 获取容器当前的容量                                                                                                   \
     * @details 返回容器当前已分配的可容纳元素数量。                                                                      \
     *          容量始终大于或等于实际元素数量（size）。时间复杂度O(1)。                                            \
     * @param vec 容器指针，允许为NULL                                                                                              \
     * @return 返回当前容量，如果vec为NULL则返回0                                                                              \
     * @see cvector_##NAME##_size, cvector_##NAME##_reserve                                                                                 \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * printf("capacity = %zu\n", cvector_int_capacity(vec));  // 输出: capacity = 4 (默认)                                             \
     * for (int i = 0; i < 10; i++) {                                                                                                       \
     *     cvector_int_push_back(vec, i);                                                                                                   \
     * }                                                                                                                                    \
     * printf("capacity = %zu\n", cvector_int_capacity(vec));  // 输出: capacity = 16 (自动扩容)                                      \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE size_t cvector_##NAME##_capacity(cvector_##NAME *vec)                                                                     \
    {                                                                                                                                       \
        return __vector_capacity((__cvector_t *)vec);                                                                                       \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 检查容器是否为空                                                                                                      \
     * @details 判断容器中是否有元素。时间复杂度O(1)。                                                                     \
     * @param vec 容器指针，允许为NULL                                                                                              \
     * @return 如果容器为空或vec为NULL返回非0值（真），否则返回0（假）                                               \
     * @see cvector_##NAME##_size, cvector_##NAME##_clear                                                                                   \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * if (cvector_int_empty(vec)) {                                                                                                        \
     *     printf("容器为空\n");                                                                                                        \
     * }                                                                                                                                    \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * if (!cvector_int_empty(vec)) {                                                                                                       \
     *     printf("容器不为空\n");                                                                                                     \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE int cvector_##NAME##_empty(cvector_##NAME *vec)                                                                           \
    {                                                                                                                                       \
        return __vector_empty((__cvector_t *)vec);                                                                                          \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 预留容器容量                                                                                                            \
     * @details 预先分配足够的内存空间以容纳至少new_cap个元素。                                                         \
     *          如果new_cap大于当前容量，会触发内存重新分配；否则不做任何操作。                                 \
     *          可以避免频繁的内存分配和元素拷贝，提高性能。                                                          \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param new_cap 需要预留的容量大小                                                                                           \
     * @note 该操作不改变size，只改变capacity。预留操作失败时容器保持原状。                                        \
     * @see cvector_##NAME##_capacity, cvector_##NAME##_shrink_to_fit                                                                       \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_reserve(vec, 10000);  // 预留10000个元素的空间                                                                   \
     * printf("size = %zu, capacity = %zu\n",                                                                                               \
     *        cvector_int_size(vec), cvector_int_capacity(vec));  // size=0, capacity=10000                                                 \
     * for (int i = 0; i < 10000; i++) {                                                                                                    \
     *     cvector_int_push_back(vec, i);  // 不会触发扩容                                                                            \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_reserve(cvector_##NAME *vec,                                                                        \
                                                size_t new_cap)                                                                             \
    {                                                                                                                                       \
        __vector_reserve((__cvector_t *)vec, new_cap, sizeof(TYPE));                                                                        \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 释放多余的容量，使容量与大小相等                                                                              \
     * @details 将容量缩减到刚好等于当前元素数量，回收未使用的内存。                                              \
     *          这个操作可能触发内存重新分配。                                                                               \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @note 该操作可能使所有指向容器元素的指针失效                                                                      \
     * @see cvector_##NAME##_reserve, cvector_##NAME##_capacity                                                                             \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create_with_capacity(1000);                                                                           \
     * for (int i = 0; i < 10; i++) {                                                                                                       \
     *     cvector_int_push_back(vec, i);                                                                                                   \
     * }                                                                                                                                    \
     * printf("size = %zu, capacity = %zu\n",                                                                                               \
     *        cvector_int_size(vec), cvector_int_capacity(vec));  // size=10, capacity=1000                                                 \
     * cvector_int_shrink_to_fit(vec);  // 释放多余的内存                                                                            \
     * printf("size = %zu, capacity = %zu\n",                                                                                               \
     *        cvector_int_size(vec), cvector_int_capacity(vec));  // size=10, capacity=10                                                   \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_shrink_to_fit(cvector_##NAME *vec)                                                                  \
    {                                                                                                                                       \
        __vector_shrink_to_fit((__cvector_t *)vec, sizeof(TYPE));                                                                           \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 访问指定位置的元素（返回值拷贝）                                                                              \
     * @details 返回指定位置索引处元素的副本。                                                                               \
     *          会检查边界，越界时触发断言终止程序。时间复杂度O(1)。                                                \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param pos 元素位置索引，必须在[0, size)范围内                                                                          \
     * @return 返回指定位置元素的值（拷贝）                                                                                   \
     * @note 越界访问会触发断言终止程序。对于大对象建议使用at_ptr避免拷贝。                                     \
     * @see cvector_##NAME##_at_ptr, cvector_##NAME##_front,                                                                                \
     * cvector_##NAME##_back                                                                                                                \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * int val = cvector_int_at(vec, 1);  // val = 20                                                                                       \
     * printf("val = %d\n", val);  // 输出: val = 20                                                                                      \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE cvector_##NAME##_at(cvector_##NAME *vec, size_t pos)                                                                 \
    {                                                                                                                                       \
        return *(TYPE *)__vector_at((__cvector_t *)vec, pos, sizeof(TYPE));                                                                 \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 访问指定位置的元素（返回指针）                                                                                 \
     * @details 返回指定位置索引处元素的指针，避免值拷贝，可用于直接修改元素。                               \
     *          会检查边界，越界时触发断言终止程序。时间复杂度O(1)。                                                \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param pos 元素位置索引，必须在[0, size)范围内                                                                          \
     * @return 返回指向指定位置元素的指针                                                                                      \
     * @note 越界访问会触发断言。容器扩容或重新分配内存时，返回的指针可能失效。                            \
     * @see cvector_##NAME##_at, cvector_##NAME##_front_ptr,                                                                                \
     * cvector_##NAME##_back_ptr                                                                                                            \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * int* ptr = cvector_int_at_ptr(vec, 1);                                                                                               \
     * *ptr = 200;  // 直接修改元素值                                                                                                \
     * printf("val = %d\n", cvector_int_at(vec, 1));  // 输出: val = 200                                                                  \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_at_ptr(cvector_##NAME *vec,                                                                        \
                                                size_t pos)                                                                                 \
    {                                                                                                                                       \
        return (TYPE *)__vector_at((__cvector_t *)vec, pos, sizeof(TYPE));                                                                  \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 访问第一个元素（返回值拷贝）                                                                                    \
     * @details 返回容器中第一个元素（索引0位置）的副本。                                                               \
     *          容器不能为空，否则触发断言终止程序。时间复杂度O(1)。                                                \
     * @param vec 容器指针，不能为NULL且size必须> 0                                                                              \
     * @return 返回第一个元素的值（拷贝）                                                                                      \
     * @note 在空容器上调用会触发断言。对于大对象建议使用front_ptr。                                                 \
     * @see cvector_##NAME##_front_ptr, cvector_##NAME##_back,                                                                              \
     * cvector_##NAME##_at                                                                                                                  \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 100);                                                                                                     \
     * cvector_int_push_back(vec, 200);                                                                                                     \
     * int first = cvector_int_front(vec);  // first = 100                                                                                  \
     * printf("first = %d\n", first);  // 输出: first = 100                                                                               \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE cvector_##NAME##_front(cvector_##NAME *vec)                                                                          \
    {                                                                                                                                       \
        return *((TYPE *)__vector_front((__cvector_t *)vec));                                                                               \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 访问最后一个元素（返回值拷贝）                                                                                 \
     * @details 返回容器中最后一个元素（索引size-1位置）的副本。                                                       \
     *          容器不能为空，否则触发断言终止程序。时间复杂度O(1)。                                                \
     * @param vec 容器指针，不能为NULL且size必须> 0                                                                              \
     * @return 返回最后一个元素的值（拷贝）                                                                                   \
     * @note 在空容器上调用会触发断言。对于大对象建议使用back_ptr。                                                  \
     * @see cvector_##NAME##_back_ptr, cvector_##NAME##_front,                                                                              \
     * cvector_##NAME##_at                                                                                                                  \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 100);                                                                                                     \
     * cvector_int_push_back(vec, 200);                                                                                                     \
     * int last = cvector_int_back(vec);  // last = 200                                                                                     \
     * printf("last = %d\n", last);  // 输出: last = 200                                                                                  \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE cvector_##NAME##_back(cvector_##NAME *vec)                                                                           \
    {                                                                                                                                       \
        return *((TYPE *)__vector_back((__cvector_t *)vec, sizeof(TYPE)));                                                                  \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 访问第一个元素（返回指针）                                                                                       \
     * @details 返回指向第一个元素（索引0位置）的指针，可用于直接修改元素。                                    \
     *          容器不能为空，否则触发断言终止程序。时间复杂度O(1)。                                                \
     * @param vec 容器指针，不能为NULL且size必须> 0                                                                              \
     * @return 返回指向第一个元素的指针                                                                                         \
     * @note 在空容器上调用会触发断言。容器扩容时返回的指针可能失效。                                           \
     * @see cvector_##NAME##_front, cvector_##NAME##_back_ptr,                                                                              \
     * cvector_##NAME##_at_ptr                                                                                                              \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 100);                                                                                                     \
     * int* first_ptr = cvector_int_front_ptr(vec);                                                                                         \
     * *first_ptr = 999;  // 修改第一个元素                                                                                          \
     * printf("first = %d\n", cvector_int_front(vec));  // 输出: first = 999                                                              \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_front_ptr(cvector_##NAME *vec)                                                                     \
    {                                                                                                                                       \
        return ((TYPE *)__vector_front((__cvector_t *)vec));                                                                                \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 访问最后一个元素（返回指针）                                                                                    \
     * @details 返回指向最后一个元素（索引size-1位置）的指针，可用于直接修改元素。                            \
     *          容器不能为空，否则触发断言终止程序。时间复杂度O(1)。                                                \
     * @param vec 容器指针，不能为NULL且size必须> 0                                                                              \
     * @return 返回指向最后一个元素的指针                                                                                      \
     * @note 在空容器上调用会触发断言。容器扩容时返回的指针可能失效。                                           \
     * @see cvector_##NAME##_back, cvector_##NAME##_front_ptr,                                                                              \
     * cvector_##NAME##_at_ptr                                                                                                              \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 100);                                                                                                     \
     * cvector_int_push_back(vec, 200);                                                                                                     \
     * int* last_ptr = cvector_int_back_ptr(vec);                                                                                           \
     * *last_ptr = 999;  // 修改最后一个元素                                                                                        \
     * printf("last = %d\n", cvector_int_back(vec));  // 输出: last = 999                                                                 \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_back_ptr(cvector_##NAME *vec)                                                                      \
    {                                                                                                                                       \
        return ((TYPE *)__vector_back((__cvector_t *)vec, sizeof(TYPE)));                                                                   \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 获取指向底层数据数组的指针                                                                                       \
     * @details 返回指向容器内部连续数组的首地址。                                                                         \
     *          可以用于与普通数组接口交互，或进行批量操作。时间复杂度O(1)。                                    \
     * @param vec 容器指针，允许为NULL                                                                                              \
     * @return 返回指向内部数组的指针，如果vec为NULL或容器未分配内存则返回NULL                                    \
     * @note                                                                                                                                \
     * 容器扩容或重新分配内存时，返回的指针会失效。返回指针有效访问范围为[0,                               \
     * size)。                                                                                                                             \
     * @see cvector_##NAME##_begin, cvector_##NAME##_end                                                                                    \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * for (int i = 0; i < 5; i++) {                                                                                                        \
     *     cvector_int_push_back(vec, i * 10);                                                                                              \
     * }                                                                                                                                    \
     * int* data = cvector_int_data(vec);                                                                                                   \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", data[i]);  // 输出: 0 10 20 30 40                                                                                \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_data(cvector_##NAME *vec)                                                                          \
    {                                                                                                                                       \
        return (TYPE *)__vector_data((__cvector_t *)vec);                                                                                   \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 获取指向第一个元素的迭代器指针                                                                                 \
     * @details 返回指向第一个元素（索引0位置）的指针。                                                                  \
     *          可以配合end()用于遍历容器。时间复杂度O(1)。                                                                \
     * @param vec 容器指针，允许为NULL                                                                                              \
     * @return 返回指向第一个元素的指针，如果vec为NULL则返回NULL                                                         \
     * @note 容器扩容时返回的指针会失效。                                                                                     \
     * @see cvector_##NAME##_end, cvector_##NAME##_data                                                                                     \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * // 使用begin和end遍历                                                                                                           \
     * for (int* it = cvector_int_begin(vec); it != cvector_int_end(vec); ++it) {                                                           \
     *     printf("%d ", *it);  // 输出: 10 20 30                                                                                         \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_begin(cvector_##NAME *vec)                                                                         \
    {                                                                                                                                       \
        return (TYPE *)__vector_begin((__cvector_t *)vec);                                                                                  \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 获取指向最后一个元素之后位置的迭代器指针                                                                  \
     * @details 返回指向最后一个元素后一位的指针（不指向有效元素）。                                              \
     *          配合begin()可以用于遍历：for(TYPE* it = begin(vec); it !=                                                          \
     * end(vec); ++it) 注意：不能解引用该指针。时间复杂度O(1)。                                                           \
     * @param vec 容器指针，允许为NULL                                                                                              \
     * @return 返回指向尾后位置的指针，如果vec为NULL则返回NULL                                                            \
     * @note 容器扩容时返回的指针会失效。该指针不指向有效元素，不能解引用。                                  \
     * @see cvector_##NAME##_begin, cvector_##NAME##_data                                                                                   \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 100);                                                                                                     \
     * cvector_int_push_back(vec, 200);                                                                                                     \
     * // end()指向最后一个元素之后的位置                                                                                      \
     * int* end_ptr = cvector_int_end(vec);                                                                                                 \
     * int* last_ptr = end_ptr - 1;  // 指向最后一个元素                                                                            \
     * printf("last = %d\n", *last_ptr);  // 输出: last = 200                                                                             \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_end(cvector_##NAME *vec)                                                                           \
    {                                                                                                                                       \
        return (TYPE *)__vector_end((__cvector_t *)vec, sizeof(TYPE));                                                                      \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 清空容器，移除所有元素                                                                                             \
     * @details 移除所有元素，将size设为0，但不改变容器的容量。                                                        \
     *          不释放已分配的内存，capacity保持不变。时间复杂度O(1)。                                                 \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @note 调用后size为0，但capacity保持不变。                                                                                 \
     * @see cvector_##NAME##_shrink_to_fit, cvector_##NAME##_empty                                                                          \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * printf("size = %zu, capacity = %zu\n",                                                                                               \
     *        cvector_int_size(vec), cvector_int_capacity(vec));  // size=2, capacity=4                                                     \
     * cvector_int_clear(vec);                                                                                                              \
     * printf("size = %zu, capacity = %zu\n",                                                                                               \
     *        cvector_int_size(vec), cvector_int_capacity(vec));  // size=0, capacity=4                                                     \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_clear(cvector_##NAME *vec)                                                                          \
    {                                                                                                                                       \
        __vector_clear((__cvector_t *)vec);                                                                                                 \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 在指定位置插入元素                                                                                                   \
     * @details 在pos位置前插入元素value。pos及之后的所有元素会后移一位。                                            \
     *          可能触发容器自动扩容（容量翻倍）。                                                                         \
     *          平均时间复杂度O(n)，最坏情况需要移动所有元素。                                                         \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param pos 插入位置索引，必须在[0,                                                                                         \
     * size]范围内（允许等于size，即尾部插入）                                                                               \
     * @param value 要插入的元素值                                                                                                   \
     * @note 越界会触发断言。插入操作可能使所有指向容器元素的指针和迭代器失效。                            \
     * @see cvector_##NAME##_push_back, cvector_##NAME##_erase                                                                              \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * cvector_int_insert(vec, 1, 20);  // 在位置1插入20                                                                               \
     * // 容器内容现在是: 10, 20, 30                                                                                                 \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 30                                                                      \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_insert(cvector_##NAME *vec, size_t pos,                                                             \
                                               TYPE value)                                                                                  \
    {                                                                                                                                       \
        __vector_insert((__cvector_t *)vec, pos, (const char *)&value,                                                                      \
                        sizeof(TYPE));                                                                                                      \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 删除指定位置的元素                                                                                                   \
     * @details 删除pos位置的元素，pos之后的所有元素会前移一位填补空缺。                                           \
     *          时间复杂度O(n)。                                                                                                      \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param pos 要删除的元素位置索引，必须在[0, size)范围内                                                              \
     * @note 越界会触发断言。删除操作会使指向pos及之后位置的指针和迭代器失效。                               \
     * @see cvector_##NAME##_remove, cvector_##NAME##_pop_back,                                                                             \
     * cvector_##NAME##_clear                                                                                                               \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * cvector_int_erase(vec, 1);  // 删除位置1的元素(20)                                                                            \
     * // 容器内容现在是: 10, 30                                                                                                     \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 30                                                                         \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_erase(cvector_##NAME *vec, size_t pos)                                                              \
    {                                                                                                                                       \
        __vector_erase((__cvector_t *)vec, pos, sizeof(TYPE));                                                                              \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 删除所有值等于指定值的元素                                                                                       \
     * @details 删除容器中所有与value值相等的元素。                                                                          \
     *          使用memcmp进行字节级比较，从后向前遍历删除。时间复杂度O(n)。                                       \
     * @param vec 容器指针，如果为NULL则不做任何操作                                                                         \
     * @param value 要删除的元素值                                                                                                   \
     * @note 会删除所有匹配的元素（不只是第一个）。删除操作会使相关指针失效。                               \
     * @see cvector_##NAME##_erase, cvector_##NAME##_find                                                                                   \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * cvector_int_remove(vec, 10);  // 删除所有值为10的元素                                                                       \
     * // 容器内容现在是: 20, 30                                                                                                     \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 20 30                                                                         \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_remove(cvector_##NAME *vec,                                                                         \
                                               TYPE value)                                                                                  \
    {                                                                                                                                       \
        __vector_remove((__cvector_t *)vec, (const char *)&value, sizeof(TYPE));                                                            \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 在容器末尾添加元素                                                                                                   \
     * @details 在容器尾部追加一个元素。这是最常用的添加元素方式。                                                 \
     *          如果容量不足会自动扩容（容量翻倍）。均摊时间复杂度O(1)。                                          \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param value 要添加的元素值                                                                                                   \
     * @note 扩容操作可能使所有指向容器元素的指针和迭代器失效。                                                    \
     * @see cvector_##NAME##_pop_back, cvector_##NAME##_insert,                                                                             \
     * cvector_##NAME##_append                                                                                                              \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 3                                                                  \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 30                                                                      \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_push_back(cvector_##NAME *vec,                                                                      \
                                                  TYPE value)                                                                               \
    {                                                                                                                                       \
        __vector_push_back((__cvector_t *)vec, (const char *)&value,                                                                        \
                           sizeof(TYPE));                                                                                                   \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 删除容器末尾的元素                                                                                                   \
     * @details 删除最后一个元素，size减1。                                                                                      \
     *          不改变容量，不释放内存。时间复杂度O(1)。                                                                  \
     * @param vec 容器指针，不能为NULL且size必须> 0                                                                              \
     * @note 在空容器上调用会触发断言。仅使指向最后一个元素的指针失效。                                        \
     * @see cvector_##NAME##_push_back, cvector_##NAME##_erase,                                                                             \
     * cvector_##NAME##_back                                                                                                                \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 3                                                                  \
     * cvector_int_pop_back(vec);  // 删除最后一个元素                                                                              \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 2                                                                  \
     * printf("last = %d\n", cvector_int_back(vec));  // 输出: last = 20                                                                  \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_pop_back(cvector_##NAME *vec)                                                                       \
    {                                                                                                                                       \
        __vector_pop_back((__cvector_t *)vec);                                                                                              \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 调整容器大小（新元素填充0）                                                                                      \
     * @details 改变容器的size为new_size。                                                                                           \
     *          如果new_size > size，新增元素会被初始化为0（memset(0)）。                                                   \
     *          如果new_size <                                                                                                            \
     * size，多余元素会被丢弃。可能触发扩容或保持原容量。                                                            \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param new_size 新的大小                                                                                                         \
     * @note 扩大时可能使所有指针失效，缩小时使指向被删除元素的指针失效。                                     \
     * @see cvector_##NAME##_resize_with_val, cvector_##NAME##_reserve                                                                      \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_resize(vec, 5);  // 扩大到5个元素，新元素为0                                                                  \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 0 0 0                                                                   \
     * }                                                                                                                                    \
     * cvector_int_resize(vec, 2);  // 缩小到2个元素                                                                                  \
     * printf("\nsize = %zu\n", cvector_int_size(vec));  // 输出: size = 2                                                                \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_resize(cvector_##NAME *vec,                                                                         \
                                               size_t new_size)                                                                             \
    {                                                                                                                                       \
        __vector_resize((__cvector_t *)vec, new_size, 0, sizeof(TYPE));                                                                     \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 调整容器大小（新元素填充指定值）                                                                              \
     * @details 改变容器的size为new_size。                                                                                           \
     *          如果new_size >                                                                                                            \
     * size，新增元素会被初始化为value的副本。 如果new_size <                                                              \
     * size，多余元素会被丢弃。可能触发扩容或保持原容量。                                                            \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param new_size 新的大小                                                                                                         \
     * @param value 新增元素的初始值                                                                                                \
     * @note 扩大时可能使所有指针失效，缩小时使指向被删除元素的指针失效。                                     \
     * @see cvector_##NAME##_resize, cvector_##NAME##_assign                                                                                \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_resize_with_val(vec, 5, 99);  // 扩大到5个元素，新元素为99                                                    \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 99 99 99                                                                \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_resize_with_val(                                                                                    \
        cvector_##NAME *vec, size_t new_size, TYPE value)                                                                                   \
    {                                                                                                                                       \
        __vector_resize((__cvector_t *)vec, new_size, (const char *)&value,                                                                 \
                        sizeof(TYPE));                                                                                                      \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 默认比较函数（用于排序）                                                                                          \
     * @details 使用memcmp进行字节级比较，适用于基本类型和简单结构体。                                              \
     *          对于指针、浮点数等特殊类型，建议提供自定义比较函数。                                              \
     * @param lhs 左操作数指针                                                                                                        \
     * @param rhs 右操作数指针                                                                                                        \
     * @return 返回值< 0表示lhs < rhs，返回0表示相等，返回值> 0表示lhs > rhs                                              \
     * @note 该函数通常不需要直接调用，主要用于sort函数的默认行为。                                                \
     * @see cvector_##NAME##_sort, cvector_##NAME##_sort_with_cmp                                                                           \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * // 通常不直接调用，而是通过sort函数间接使用                                                                        \
     * int a = 10, b = 20;                                                                                                                  \
     * int result = cvector_int_cmp(&a, &b);  // result < 0, 因为10 < 20                                                                  \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE int cvector_##NAME##_cmp(const TYPE *lhs, const TYPE *rhs)                                                                \
    {                                                                                                                                       \
        return memcmp(lhs, rhs, sizeof(TYPE));                                                                                              \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 对容器中的元素进行排序（使用默认比较函数）                                                               \
     * @details 使用标准库qsort函数对所有元素进行排序。                                                                    \
     *          默认使用memcmp进行字节级比较，时间复杂度O(n                                                                \
     * log n)。                                                                                                                            \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @note 对于指针、浮点数等需要自定义比较逻辑的类型，应使用sort_with_cmp。                                    \
     * @see cvector_##NAME##_sort_with_cmp, cvector_##NAME##_cmp                                                                            \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_sort(vec);  // 排序                                                                                                    \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 30                                                                      \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_sort(cvector_##NAME *vec)                                                                           \
    {                                                                                                                                       \
        __vector_sort((__cvector_t *)vec,                                                                                                   \
                      (__cvector_t_cmp_cb)cvector_##NAME##_cmp, 0,                                                                          \
                      __vector_size((__cvector_t *)vec), sizeof(TYPE));                                                                     \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 对容器中的元素进行排序（使用自定义比较函数）                                                            \
     * @details 使用标准库qsort函数和用户提供的比较函数对元素排序。                                                  \
     *          时间复杂度O(n log n)。                                                                                                \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param cb 自定义比较回调函数，返回负数表示lhs < rhs                                                                  \
     * @note 比较函数的返回值语义必须与标准的比较函数一致。                                                          \
     * @see cvector_##NAME##_sort, cvector_##NAME##_cmp_cb                                                                                  \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * // 自定义降序比较函数                                                                                                       \
     * int int_cmp_desc(const int* a, const int* b) {                                                                                       \
     *     return (*b - *a);  // 降序                                                                                                     \
     * }                                                                                                                                    \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_sort_with_cmp(vec, int_cmp_desc);  // 降序排序                                                                       \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 30 20 10                                                                      \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_sort_with_cmp(                                                                                      \
        cvector_##NAME *vec, cvector_##NAME##_cmp_cb cb)                                                                                    \
    {                                                                                                                                       \
        __vector_sort((__cvector_t *)vec, (__cvector_t_cmp_cb)cb, 0,                                                                        \
                      __vector_size((__cvector_t *)vec), sizeof(TYPE));                                                                     \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 拷贝容器内容                                                                                                            \
     * @details 将src容器的所有元素拷贝到dst容器。                                                                            \
     *          会清空dst原有内容，然后复制src的所有元素。可能触发dst扩容。时间复杂度O(n)。                  \
     * @param dst 目标容器指针，不能为NULL                                                                                        \
     * @param src 源容器指针，不能为NULL                                                                                           \
     * @note                                                                                                                                \
     * dst原有内容会被清空。拷贝后两个容器相互独立。可能使dst的所有指针失效。                               \
     * @see cvector_##NAME##_swap, cvector_##NAME##_assign                                                                                  \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* src = cvector_int_create();                                                                                             \
     * cvector_int_push_back(src, 10);                                                                                                      \
     * cvector_int_push_back(src, 20);                                                                                                      \
     * cvector_int* dst = cvector_int_create();                                                                                             \
     * cvector_int_copy(dst, src);  // 拷贝src到dst                                                                                      \
     * printf("dst size = %zu\n", cvector_int_size(dst));  // 输出: dst size = 2                                                          \
     * cvector_int_push_back(src, 30);  // 修改src不影响dst                                                                            \
     * printf("src size = %zu, dst size = %zu\n",                                                                                           \
     *        cvector_int_size(src), cvector_int_size(dst));  // 输出: src size = 3, dst size = 2                                         \
     * cvector_int_destroy(&src);                                                                                                           \
     * cvector_int_destroy(&dst);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_copy(cvector_##NAME *dst,                                                                           \
                                             const cvector_##NAME *src)                                                                     \
    {                                                                                                                                       \
        __vector_copy((__cvector_t *)dst, (const __cvector_t *)src, sizeof(TYPE));                                                          \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 交换两个容器的内容                                                                                                   \
     * @details 交换两个容器的所有内容，包括data指针、size和capacity。                                                   \
     *          这是一个高效操作，时间复杂度O(1)，不涉及元素拷贝。                                                   \
     * @param vec1 第一个容器指针，不能为NULL                                                                                    \
     * @param vec2 第二个容器指针，不能为NULL                                                                                    \
     * @note                                                                                                                                \
     * 这是一个高效操作，仅交换容器的内部指针，不移动元素。所有指针仍然有效但指向被交换的容器。 \
     * @see cvector_##NAME##_copy                                                                                                           \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec1 = cvector_int_create();                                                                                            \
     * cvector_int_push_back(vec1, 10);                                                                                                     \
     * cvector_int* vec2 = cvector_int_create();                                                                                            \
     * cvector_int_push_back(vec2, 20);                                                                                                     \
     * cvector_int_push_back(vec2, 30);                                                                                                     \
     * cvector_int_swap(vec1, vec2);  // 交换两个容器                                                                                 \
     * printf("vec1 size = %zu\n", cvector_int_size(vec1));  // 输出: vec1 size = 2                                                       \
     * printf("vec2 size = %zu\n", cvector_int_size(vec2));  // 输出: vec2 size = 1                                                       \
     * printf("vec1[0] = %d\n", cvector_int_at(vec1, 0));  // 输出: vec1[0] = 20                                                          \
     * printf("vec2[0] = %d\n", cvector_int_at(vec2, 0));  // 输出: vec2[0] = 10                                                          \
     * cvector_int_destroy(&vec1);                                                                                                          \
     * cvector_int_destroy(&vec2);                                                                                                          \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_swap(cvector_##NAME *vec1,                                                                          \
                                             cvector_##NAME *vec2)                                                                          \
    {                                                                                                                                       \
        __vector_swap((__cvector_t *)vec1, (__cvector_t *)vec2);                                                                            \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 替换容器内容为count个相同的value                                                                                   \
     * @details 清空容器并填充count个value副本。                                                                                 \
     *          会清空原有内容，然后填充新元素。可能触发扩容。时间复杂度O(count)。                             \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param count 要填充的元素数量                                                                                                \
     * @param value 要填充的元素值                                                                                                   \
     * @note 会清空原有内容。可能使所有指针失效。                                                                         \
     * @see cvector_##NAME##_resize_with_val, cvector_##NAME##_append,                                                                      \
     * cvector_##NAME##_clear                                                                                                               \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_assign(vec, 5, 99);  // 替换为5个99                                                                                  \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 5                                                                  \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 99 99 99 99 99                                                                \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_assign(cvector_##NAME *vec,                                                                         \
                                               size_t count, TYPE value)                                                                    \
    {                                                                                                                                       \
        __vector_assign((__cvector_t *)vec, count, (const char *)&value,                                                                    \
                        sizeof(TYPE));                                                                                                      \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 在容器末尾追加多个元素                                                                                             \
     * @details 在尾部批量追加多个元素。                                                                                        \
     *          比多次调用push_back更高效。可能触发扩容（按需）。均摊时间复杂度O(count)。                       \
     * @param vec 容器指针，不能为NULL                                                                                              \
     * @param items 指向要追加的元素数组的指针，不能为NULL                                                                 \
     * @param count 要追加的元素数量                                                                                                \
     * @note count为0时不做任何操作。扩容可能使所有指针失效。                                                          \
     * @see cvector_##NAME##_push_back, cvector_##NAME##_assign                                                                             \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * int arr[] = {20, 30, 40, 50};                                                                                                        \
     * cvector_int_append(vec, arr, 4);  // 追加4个元素                                                                                \
     * printf("size = %zu\n", cvector_int_size(vec));  // 输出: size = 5                                                                  \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 30 40 50                                                                \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_append(                                                                                             \
        cvector_##NAME *vec, const TYPE *items, size_t count)                                                                               \
    {                                                                                                                                       \
        __vector_append((__cvector_t *)vec, (const char *)items, count,                                                                     \
                        sizeof(TYPE));                                                                                                      \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 判断两个容器是否相等                                                                                                \
     * @details 比较两个容器的size和所有元素内容。                                                                           \
     *          使用memcmp进行字节级比较。时间复杂度O(n)。                                                                  \
     * @param vec1 第一个容器指针                                                                                                    \
     * @param vec2 第二个容器指针                                                                                                    \
     * @return 如果两个容器相等返回非0值（真），否则返回0（假）                                                      \
     * @note 两个NULL指针被认为相等。size不同则立即返回不相等。                                                        \
     * @see cvector_##NAME##_copy                                                                                                           \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec1 = cvector_int_create();                                                                                            \
     * cvector_int* vec2 = cvector_int_create();                                                                                            \
     * cvector_int_push_back(vec1, 10);                                                                                                     \
     * cvector_int_push_back(vec1, 20);                                                                                                     \
     * cvector_int_push_back(vec2, 10);                                                                                                     \
     * cvector_int_push_back(vec2, 20);                                                                                                     \
     * if (cvector_int_equals(vec1, vec2)) {                                                                                                \
     *     printf("两个容器相等\n");                                                                                                  \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec1);                                                                                                          \
     * cvector_int_destroy(&vec2);                                                                                                          \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE int cvector_##NAME##_equals(cvector_##NAME *vec1,                                                                         \
                                              cvector_##NAME *vec2)                                                                         \
    {                                                                                                                                       \
        return __vector_equals((__cvector_t *)vec1, (__cvector_t *)vec2,                                                                    \
                               sizeof(TYPE));                                                                                               \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 查找元素首次出现的位置                                                                                             \
     * @details 线性查找第一个与value匹配的元素。                                                                             \
     *          使用memcmp进行字节级比较。时间复杂度O(n)。                                                                  \
     * @param vec 容器指针                                                                                                              \
     * @param value 要查找的元素值                                                                                                   \
     * @return 返回找到的元素索引（>=0），如果未找到或vec为NULL返回-1                                                  \
     * @note 对于已排序容器，建议使用二分查找以提高效率。                                                             \
     * @see cvector_##NAME##_contains, cvector_##NAME##_remove                                                                              \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * int index = cvector_int_find(vec, 20);                                                                                               \
     * if (index >= 0) {                                                                                                                    \
     *     printf("找到元素20，位置为 %d\n", index);  // 输出: 找到元素20，位置为 1                                       \
     * } else {                                                                                                                             \
     *     printf("未找到元素\n");                                                                                                     \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE int cvector_##NAME##_find(cvector_##NAME *vec, TYPE value)                                                                \
    {                                                                                                                                       \
        return __vector_find((__cvector_t *)vec, (const char *)&value,                                                                      \
                             sizeof(TYPE));                                                                                                 \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 判断容器是否包含指定元素                                                                                          \
     * @details 检查容器中是否存在与value匹配的元素。                                                                       \
     *          使用memcmp进行字节级比较。时间复杂度O(n)。                                                                  \
     * @param vec 容器指针                                                                                                              \
     * @param value 要查找的元素值                                                                                                   \
     * @return 如果找到返回非0值（真），未找到或vec为NULL返回0（假）                                                  \
     * @note 只关心是否存在，不关心位置时使用此函数比find更清晰。                                                   \
     * @see cvector_##NAME##_find, cvector_##NAME##_remove                                                                                  \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * if (cvector_int_contains(vec, 20)) {                                                                                                 \
     *     printf("容器包含元素20\n");                                                                                                \
     * }                                                                                                                                    \
     * if (!cvector_int_contains(vec, 30)) {                                                                                                \
     *     printf("容器不包含元素30\n");                                                                                             \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE int cvector_##NAME##_contains(cvector_##NAME *vec,                                                                        \
                                                TYPE value)                                                                                 \
    {                                                                                                                                       \
        return __vector_contains((__cvector_t *)vec, (const char *)&value,                                                                  \
                                 sizeof(TYPE));                                                                                             \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 反转容器中所有元素的顺序                                                                                          \
     * @details 原地反转所有元素的顺序，第一个和最后一个互换，第二个和倒数第二个互换，依此类推。    \
     *          时间复杂度O(n/2)。                                                                                                    \
     * @param vec 容器指针                                                                                                              \
     * @note 空容器或单元素容器调用此函数不做任何操作。                                                                \
     * @see cvector_##NAME##_sort                                                                                                           \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * DEFINE_CVECTOR(int)                                                                                                                  \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     * cvector_int_push_back(vec, 30);                                                                                                      \
     * printf("反转前: ");                                                                                                               \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 10 20 30                                                                      \
     * }                                                                                                                                    \
     * cvector_int_reverse(vec);                                                                                                            \
     * printf("\n反转后: ");                                                                                                             \
     * for (size_t i = 0; i < cvector_int_size(vec); i++) {                                                                                 \
     *     printf("%d ", cvector_int_at(vec, i));  // 输出: 30 20 10                                                                      \
     * }                                                                                                                                    \
     * cvector_int_destroy(&vec);                                                                                                           \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_reverse(cvector_##NAME *vec)                                                                        \
    {                                                                                                                                       \
        __vector_reverse((__cvector_t *)vec, sizeof(TYPE));                                                                                 \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 移动向量（零拷贝）                                                                                                   \
     * @details 将源向量的数据所有权转移到目标向量，避免拷贝数据                                                    \
     * @param dst 目标向量                                                                                                              \
     * @param src 源向量                                                                                                                 \
     * @note 操作后源向量变为空，目标向量获得源向量的所有数据                                                       \
     * @warning 目标向量原有数据会被释放                                                                                        \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * cvector_int* vec1 = cvector_int_create();                                                                                            \
     * cvector_int_push_back(vec1, 10);                                                                                                     \
     * cvector_int_push_back(vec1, 20);                                                                                                     \
     *                                                                                                                                      \
     * cvector_int* vec2 = cvector_int_create();                                                                                            \
     * cvector_int_move(vec2, vec1);  // 零拷贝移动                                                                                    \
     *                                                                                                                                      \
     * // vec1 现在为空，vec2 拥有所有数据                                                                                       \
     * printf("vec1 size: %zu\n", cvector_int_size(vec1));  // 输出: 0                                                                    \
     * printf("vec2 size: %zu\n", cvector_int_size(vec2));  // 输出: 2                                                                    \
     * cvector_int_destroy(&vec1);                                                                                                          \
     * cvector_int_destroy(&vec2);                                                                                                          \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_move(cvector_##NAME *dst, cvector_##NAME *src)                                                      \
    {                                                                                                                                       \
        __vector_move((__cvector_t *)dst, (__cvector_t *)src);                                                                              \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 释放向量数据的所有权                                                                                                \
     * @details 将向量内部数据指针返回给调用者，调用者负责释放内存                                                 \
     * @param vec 向量指针                                                                                                              \
     * @param out_size 输出参数：返回元素数量                                                                                    \
     * @param out_capacity 输出参数：返回容量                                                                                      \
     * @return 返回数据指针，调用者需要负责释放                                                                             \
     * @warning 返回的指针必须使用当前分配器的 free 函数释放                                                             \
     * @note 调用后向量变为空                                                                                                       \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_push_back(vec, 10);                                                                                                      \
     * cvector_int_push_back(vec, 20);                                                                                                      \
     *                                                                                                                                      \
     * size_t size, capacity;                                                                                                               \
     * int* data = cvector_int_release(vec, &size, &capacity);                                                                              \
     *                                                                                                                                      \
     * printf("Released %zu elements\n", size);                                                                                             \
     * // 使用 data...                                                                                                                    \
     * free(data);  // 手动释放                                                                                                         \
     * cvector_int_destroy(&vec);  // vec 已经为空                                                                                      \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE TYPE *cvector_##NAME##_release(cvector_##NAME *vec, size_t *out_size, size_t *out_capacity)                               \
    {                                                                                                                                       \
        return (TYPE *)__vector_release((__cvector_t *)vec, out_size, out_capacity);                                                        \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 从外部缓冲区获取所有权                                                                                             \
     * @details 让向量接管外部已分配的内存缓冲区                                                                            \
     * @param vec 向量指针                                                                                                              \
     * @param external_data 外部数据缓冲区                                                                                           \
     * @param size 当前元素数量                                                                                                       \
     * @param capacity 缓冲区容量                                                                                                      \
     * @param take_ownership 是否获取所有权（1=获取，0=仅引用）                                                              \
     * @warning 如果 take_ownership=1，external_data 必须由当前分配器分配                                                      \
     * @note 向量销毁时会根据 take_ownership 决定是否释放内存                                                               \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * // 示例1：获取所有权                                                                                                         \
     * int* data = (int*)malloc(100 * sizeof(int));                                                                                         \
     * for (int i = 0; i < 100; i++) data[i] = i;                                                                                           \
     *                                                                                                                                      \
     * cvector_int* vec = cvector_int_create();                                                                                             \
     * cvector_int_take_ownership(vec, data, 100, 100, 1);  // 获取所有权                                                              \
     * // vec 销毁时会自动释放 data                                                                                                 \
     * cvector_int_destroy(&vec);                                                                                                           \
     *                                                                                                                                      \
     * // 示例2：仅引用（不获取所有权）                                                                                       \
     * int static_buffer[50];                                                                                                               \
     * cvector_int* vec2 = cvector_int_create();                                                                                            \
     * cvector_int_take_ownership(vec2, static_buffer, 0, 50, 0);  // 仅引用                                                             \
     * // vec2 销毁时不会释放 static_buffer                                                                                          \
     * cvector_int_destroy(&vec2);                                                                                                          \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_take_ownership(cvector_##NAME *vec, TYPE *external_data,                                            \
                                                       size_t size, size_t capacity, int take_ownership)                                    \
    {                                                                                                                                       \
        __vector_take_ownership((__cvector_t *)vec, (char *)external_data, size, capacity, sizeof(TYPE), take_ownership);                   \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 使用固定缓冲区初始化向量（无动态分配）                                                                     \
     * @details 在栈上或预分配的内存中创建向量，不进行动态分配                                                       \
     * @param vec 向量指针                                                                                                              \
     * @param fixed_buffer 固定缓冲区指针                                                                                            \
     * @param buffer_capacity 缓冲区容量（元素个数）                                                                             \
     * @note 固定模式下不能扩容，超出容量的 push_back 会失败                                                              \
     * @warning fixed_buffer 必须在向量整个生命周期内有效                                                                     \
     * @warning 向量销毁时不会释放 fixed_buffer                                                                                    \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * // 在栈上创建固定大小向量                                                                                                 \
     * int buffer[100];                                                                                                                     \
     * cvector_int vec_stack;                                                                                                               \
     * cvector_int_init_fixed(&vec_stack, buffer, 100);                                                                                     \
     *                                                                                                                                      \
     * cvector_int_push_back(&vec_stack, 10);                                                                                               \
     * cvector_int_push_back(&vec_stack, 20);                                                                                               \
     * printf("size: %zu\n", cvector_int_size(&vec_stack));  // 输出: 2                                                                   \
     *                                                                                                                                      \
     * // 超出容量会失败                                                                                                             \
     * for (int i = 0; i < 200; i++) {                                                                                                      \
     *     cvector_int_push_back(&vec_stack, i);                                                                                            \
     *     if (cvector_get_last_error() != CVECTOR_OK) {                                                                                    \
     *         printf("超出容量限制\n");                                                                                              \
     *         break;                                                                                                                       \
     *     }                                                                                                                                \
     * }                                                                                                                                    \
     *                                                                                                                                      \
     * cvector_int_uninit(&vec_stack);  // 不会释放 buffer                                                                              \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE void cvector_##NAME##_init_fixed(cvector_##NAME *vec, TYPE *fixed_buffer, size_t buffer_capacity)                         \
    {                                                                                                                                       \
        __vector_init_fixed((__cvector_t *)vec, (char *)fixed_buffer, buffer_capacity, sizeof(TYPE));                                       \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 检查向量是否为固定模式                                                                                             \
     * @param vec 向量指针                                                                                                              \
     * @return 1=固定模式，0=动态模式                                                                                              \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * if (cvector_int_is_fixed(vec)) {                                                                                                     \
     *     printf("固定模式，容量有限\n");                                                                                         \
     * } else {                                                                                                                             \
     *     printf("动态模式，可自动扩容\n");                                                                                      \
     * }                                                                                                                                    \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE int cvector_##NAME##_is_fixed(const cvector_##NAME *vec)                                                                  \
    {                                                                                                                                       \
        return __vector_is_fixed((const __cvector_t *)vec);                                                                                 \
    }                                                                                                                                       \
                                                                                                                                            \
    /**                                                                                                                                     \
     * @brief 获取向量的剩余容量                                                                                                   \
     * @param vec 向量指针                                                                                                              \
     * @return 剩余可用容量                                                                                                           \
     * @note 对于固定模式向量特别有用                                                                                           \
     *                                                                                                                                      \
     * 使用示例：                                                                                                                      \
     * @code                                                                                                                                \
     * printf("剩余空间: %zu\n", cvector_int_available_capacity(vec));                                                                  \
     * if (cvector_int_available_capacity(vec) > 0) {                                                                                       \
     *     cvector_int_push_back(vec, 42);                                                                                                  \
     * }                                                                                                                                    \
     * @endcode                                                                                                                             \
     */                                                                                                                                     \
    static INLINE size_t cvector_##NAME##_available_capacity(const cvector_##NAME *vec)                                                     \
    {                                                                                                                                       \
        return __vector_available_capacity((const __cvector_t *)vec);                                                                       \
    }

/**
 * @def cvector_foreach(vec_ptr, item_type, item_name)
 * @brief 正向遍历cvector容器的所有元素
 * @details 这是一个便捷宏，用于以类似for-each的方式遍历容器中的每个元素。
 *          会自动处理索引，提供值拷贝的迭代变量。
 *          遍历顺序为从第一个元素到最后一个元素（索引0到size-1）。
 *          在循环体内，item_name是元素的值拷贝，修改它不会影响容器中的原始元素。
 * @param vec_ptr 指向cvector容器的指针，类型为 cvector_TYPE*
 * @param item_type 元素类型，必须与容器定义时的TYPE一致
 * @param item_name 循环变量名，在循环体中可以使用此变量访问当前元素的值
 * @note 循环体内的 item_name 是值拷贝，修改它不影响容器。如需修改容器元素，应使用索引或指针访问。
 * @warning 遍历过程中不要修改容器结构（如push_back, erase等），可能导致未定义行为。
 * @see cvector_foreach_reverse
 *
 * 使用示例：
 * @code
 * DEFINE_CVECTOR(int)
 *
 * cvector_int* vec = cvector_int_create();
 * cvector_int_push_back(vec, 10);
 * cvector_int_push_back(vec, 20);
 * cvector_int_push_back(vec, 30);
 *
 * // 遍历并打印所有元素
 * cvector_foreach(vec, int, value) {
 *     printf("value = %d\n", value);  // 输出: 10, 20, 30
 * }
 *
 * // 注意：修改value不会影响容器
 * cvector_foreach(vec, int, value) {
 *     value = 100;  // 不会改变容器中的元素
 * }
 *
 * // 如需修改容器元素，应该使用索引
 * for (size_t i = 0; i < cvector_int_size(vec); ++i) {
 *     int* ptr = cvector_int_at_ptr(vec, i);
 *     *ptr = 100;  // 这样可以修改容器中的元素
 * }
 *
 * cvector_int_destroy(&vec);
 * @endcode
 */
#define cvector_foreach(vec_ptr, item_type, item_name)                                               \
    for (int item_name##_scope_ = 1; item_name##_scope_; item_name##_scope_ = 0)                     \
        for (size_t item_name##_index = 0; item_name##_index < (vec_ptr)->size; ++item_name##_index) \
            for (item_type item_name = (vec_ptr)->data[item_name##_index],                           \
                           *item_name##_once = &item_name;                                           \
                 item_name##_once; item_name##_once = NULL)

/**
 * @def cvector_foreach_reverse(vec_ptr, item_type, item_name)
 * @brief 反向遍历cvector容器的所有元素
 * @details 这是一个便捷宏，用于以类似for-each的方式反向遍历容器中的每个元素。
 *          会自动处理索引，提供值拷贝的迭代变量。
 *          遍历顺序为从最后一个元素到第一个元素（索引size-1到0）。
 *          在循环体内，item_name是元素的值拷贝，修改它不会影响容器中的原始元素。
 * @param vec_ptr 指向cvector容器的指针，类型为 cvector_TYPE*
 * @param item_type 元素类型，必须与容器定义时的TYPE一致
 * @param item_name 循环变量名，在循环体中可以使用此变量访问当前元素的值
 * @note 循环体内的 item_name 是值拷贝，修改它不影响容器。如需修改容器元素，应使用索引或指针访问。
 * @warning 遍历过程中不要修改容器结构（如push_back, erase等），可能导致未定义行为。
 * @see cvector_foreach
 *
 * 使用示例：
 * @code
 * DEFINE_CVECTOR(int)
 *
 * cvector_int* vec = cvector_int_create();
 * cvector_int_push_back(vec, 10);
 * cvector_int_push_back(vec, 20);
 * cvector_int_push_back(vec, 30);
 *
 * // 反向遍历并打印所有元素
 * cvector_foreach_reverse(vec, int, value) {
 *     printf("value = %d\n", value);  // 输出: 30, 20, 10
 * }
 *
 * // 反向查找第一个满足条件的元素（从后往前）
 * int found = 0;
 * cvector_foreach_reverse(vec, int, value) {
 *     if (value == 20) {
 *         printf("找到元素: %d\n", value);
 *         found = 1;
 *         break;  // 可以使用break退出循环
 *     }
 * }
 *
 * // 反向遍历删除元素（从后往前删除是安全的）
 * for (size_t i = cvector_int_size(vec); i-- > 0;) {
 *     if (cvector_int_at(vec, i) < 25) {
 *         cvector_int_erase(vec, i);  // 从后往前删除避免索引问题
 *     }
 * }
 *
 * cvector_int_destroy(&vec);
 * @endcode
 */
#define cvector_foreach_reverse(vec_ptr, item_type, item_name)                     \
    for (int item_name##_scope_ = 1; item_name##_scope_; item_name##_scope_ = 0)   \
        for (size_t item_name##_index = (vec_ptr)->size; item_name##_index-- > 0;) \
            for (item_type item_name = (vec_ptr)->data[item_name##_index],         \
                           *item_name##_once = &item_name;                         \
                 item_name##_once; item_name##_once = NULL)

    // 注意：以下部分是库内部函数，用户不要调用-------------------

    typedef struct __cvector_t __cvector_t;
    typedef int (*__cvector_t_cmp_cb)(const void *lhs, const void *rhs);

    /* 构造函数/析构函数 */
    CC_API __cvector_t *CC_CALL __vector_create(size_t item_size);
    CC_API __cvector_t *CC_CALL __vector_create_with_capacity(size_t capacity,
                                                              size_t item_size);
    CC_API void CC_CALL __vector_init(__cvector_t *vec, size_t item_size);
    CC_API void CC_CALL __vector_init_with_capacity(__cvector_t *vec,
                                                    size_t capacity,
                                                    size_t item_size);
    CC_API void CC_CALL __vector_uninit(__cvector_t *vec);
    CC_API void CC_CALL __vector_destroy(__cvector_t *vec);

    /* 容量相关 */
    CC_API size_t CC_CALL __vector_size(const __cvector_t *vec);
    CC_API size_t CC_CALL __vector_capacity(const __cvector_t *vec);
    CC_API int CC_CALL __vector_empty(const __cvector_t *vec);
    CC_API void CC_CALL __vector_reserve(__cvector_t *vec, size_t new_cap,
                                         size_t item_size);
    CC_API void CC_CALL __vector_shrink_to_fit(__cvector_t *vec, size_t item_size);

    /* 元素访问 */
    CC_API char *CC_CALL __vector_at(__cvector_t *vec, size_t pos,
                                     size_t item_size);
    CC_API char *CC_CALL __vector_front(__cvector_t *vec);
    CC_API char *CC_CALL __vector_back(__cvector_t *vec, size_t item_size);
    CC_API char *CC_CALL __vector_data(__cvector_t *vec);

    /* 迭代器 */
    CC_API char *CC_CALL __vector_begin(__cvector_t *vec);
    CC_API char *CC_CALL __vector_end(__cvector_t *vec, size_t item_size);

    /* 修改器 */
    CC_API void CC_CALL __vector_clear(__cvector_t *vec);
    CC_API void CC_CALL __vector_insert(__cvector_t *vec, size_t pos,
                                        const char *item_ptr, size_t item_size);
    CC_API void CC_CALL __vector_erase(__cvector_t *vec, size_t pos,
                                       size_t item_size);
    CC_API void CC_CALL __vector_remove(__cvector_t *vec, const char *item_ptr,
                                        size_t item_size);
    CC_API void CC_CALL __vector_push_back(__cvector_t *vec, const char *item_ptr,
                                           size_t item_size);
    CC_API void CC_CALL __vector_pop_back(__cvector_t *vec);
    CC_API void CC_CALL __vector_resize(__cvector_t *vec, size_t new_size,
                                        const char *item_ptr, size_t item_size);
    CC_API void CC_CALL __vector_sort(__cvector_t *vec, __cvector_t_cmp_cb cmp,
                                      size_t start, size_t size, size_t item_size);

    /* 新增功能函数 */
    CC_API void CC_CALL __vector_copy(__cvector_t *dst, const __cvector_t *src,
                                      size_t item_size);
    CC_API void CC_CALL __vector_swap(__cvector_t *vec1, __cvector_t *vec2);
    CC_API void CC_CALL __vector_assign(__cvector_t *vec, size_t count,
                                        const char *item_ptr, size_t item_size);
    CC_API void CC_CALL __vector_append(__cvector_t *vec, const char *items,
                                        size_t count, size_t item_size);
    CC_API int CC_CALL __vector_equals(__cvector_t *vec1, __cvector_t *vec2,
                                       size_t item_size);
    CC_API int CC_CALL __vector_find(__cvector_t *vec, const char *item_ptr,
                                     size_t item_size);
    CC_API int CC_CALL __vector_contains(__cvector_t *vec, const char *item_ptr,
                                         size_t item_size);
    CC_API void CC_CALL __vector_reverse(__cvector_t *vec, size_t item_size);

    /* 零拷贝操作（Move Semantics） */
    CC_API void CC_CALL __vector_move(__cvector_t *dst, __cvector_t *src);
    CC_API char *CC_CALL __vector_release(__cvector_t *vec, size_t *out_size, size_t *out_capacity);
    CC_API void CC_CALL __vector_take_ownership(__cvector_t *vec, char *external_data,
                                                size_t size, size_t capacity,
                                                size_t item_size, int take_ownership);

    /* 固定大小无分配模式 */
    CC_API void CC_CALL __vector_init_fixed(__cvector_t *vec, char *fixed_buffer,
                                            size_t buffer_capacity, size_t item_size);
    CC_API int CC_CALL __vector_is_fixed(const __cvector_t *vec);
    CC_API size_t CC_CALL __vector_available_capacity(const __cvector_t *vec);

/* ==================== 内存分配器接口 ==================== */

/* 包含分配器接口定义 */
#include "cvector_allocator.h"

    /**
     * @brief 获取最后一次操作的错误码（全局，多线程环境可能不准确）
     * @return cvector_error_t 错误码
     * @see cvector_error_t
     * @note 在多线程/RTOS环境下，建议使用 __vector_get_error() 获取向量特定的错误码
     */
    CC_API cvector_error_t CC_CALL cvector_get_last_error(void);

    /**
     * @brief 清除错误码
     */
    CC_API void CC_CALL cvector_clear_error(void);

    /**
     * @brief 获取向量特定的错误码（线程安全）
     * @param vec 向量指针
     * @return cvector_error_t 错误码，如果 vec 为 NULL 则返回 CVECTOR_ERROR_INVALID
     * @note 这是线程安全的，每个向量维护自己的错误码
     * @note 推荐在多线程/RTOS环境下使用此函数代替 cvector_get_last_error()
     * @see cvector_get_last_error
     */
    CC_API cvector_error_t CC_CALL __vector_get_error(const struct __cvector_t *vec);

#ifdef CVECTOR_ENABLE_MEMORY_TRACKING
    /**
     * @brief 获取当前分配的总内存大小
     * @return size_t 字节数
     * @note 仅在启用 CVECTOR_ENABLE_MEMORY_TRACKING 时可用
     */
    CC_API size_t CC_CALL cvector_get_memory_usage(void);

    /**
     * @brief 获取峰值内存使用量
     * @return size_t 字节数
     * @note 仅在启用 CVECTOR_ENABLE_MEMORY_TRACKING 时可用
     */
    CC_API size_t CC_CALL cvector_get_peak_memory_usage(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

#ifndef C_CORE_MAP_H_
#define C_CORE_MAP_H_

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
    /*
     * 注意：千万注意，本模块中的key_ptr指针指向的内容不可直接修改
     */

    /**
     * @brief 定义一个键类型为KT、值类型为VT的map类型
     * 
     * 该宏会生成一个红黑树实现的map容器，类型名称为 cmap_KT_VT。
     * 生成的map支持插入、删除、查找等操作，时间复杂度为O(log n)。
     * 
     * @param KT 键的类型（key type）
     * @param VT 值的类型（value type）
     * 
     * @note 生成的类型名为 cmap_KT_VT，例如 DEFINE_CMAP(int, double) 生成 cmap_int_double
     * @note 键类型需要支持 memcmp 比较，或者使用 create_with_cmp 指定比较函数
     * 
     * 使用示例:
     * @code
     * DEFINE_CMAP(int, double);  // 定义一个int到double的map
     * cmap_int_double *map = cmap_int_double_create();
     * cmap_int_double_insert(map, 1, 3.14);
     * cmap_int_double_destroy(&map);
     * @endcode
     */
#define DEFINE_CMAP(KT, VT) DEFINE_CMAP_AS(KT, VT, KT##_##VT)

    /**
     * @brief 定义一个键类型为KT指针、值类型为VT的map类型
     * 
     * @param KT 键指针指向的类型
     * @param VT 值的类型
     * 
     * @note 生成的类型名为 cmap_KT_ptr_VT
     */
#define DEFINE_CMAP_KEY_PTR_TO_VALUE(KT, VT) DEFINE_CMAP_AS(KT *, VT, KT##_ptr_##VT)

    /**
     * @brief 定义一个键类型为KT、值类型为VT指针的map类型
     * 
     * @param KT 键的类型
     * @param VT 值指针指向的类型
     * 
     * @note 生成的类型名为 cmap_KT_VT_ptr
     */
#define DEFINE_CMAP_KEY_TO_VALUE_PTR(KT, VT) DEFINE_CMAP_AS(KT, VT *, KT##_##VT##_ptr)

    /**
     * @brief 定义一个键类型为KT指针、值类型为VT指针的map类型
     * 
     * @param KT 键指针指向的类型
     * @param VT 值指针指向的类型
     * 
     * @note 生成的类型名为 cmap_KT_ptr_VT_ptr
     */
#define DEFINE_CMAP_KEY_PTR_TO_VALUE_PTR(KT, VT) DEFINE_CMAP_AS(KT *, VT *, KT##_ptr_##VT##_ptr)

    /**
     * @brief 定义一个键类型为KT指针、值类型为VT的map类型，并指定自定义名称
     * 
     * @param KT 键指针指向的类型
     * @param VT 值的类型
     * @param NAME 自定义的map类型名称
     * 
     * @note 生成的类型名为 cmap_NAME
     */
#define DEFINE_CMAP_KEY_PTR_TO_VALUE_AS(KT, VT, NAME) DEFINE_CMAP_AS(KT *, VT, NAME)

    /**
     * @brief 定义一个键类型为KT、值类型为VT指针的map类型，并指定自定义名称
     * 
     * @param KT 键的类型
     * @param VT 值指针指向的类型
     * @param NAME 自定义的map类型名称
     * 
     * @note 生成的类型名为 cmap_NAME
     */
#define DEFINE_CMAP_KEY_TO_VALUE_PTR_AS(KT, VT, NAME) DEFINE_CMAP_AS(KT, VT *, NAME)

    /**
     * @brief 定义一个键类型为KT指针、值类型为VT指针的map类型，并指定自定义名称
     * 
     * @param KT 键指针指向的类型
     * @param VT 值指针指向的类型
     * @param NAME 自定义的map类型名称
     * 
     * @note 生成的类型名为 cmap_NAME
     */
#define DEFINE_CMAP_KEY_PTR_TO_VALUE_PTR_AS(KT, VT, NAME) DEFINE_CMAP_AS(KT *, VT *, NAME)

    /**
     * @brief 定义一个键类型为KT、值类型为VT的map类型，并指定自定义名称
     * 
     * 该宏会生成完整的红黑树map容器类型定义，包括节点结构、map结构以及所有操作函数。
     * 生成的map基于红黑树实现，保证插入、删除、查找操作的时间复杂度为O(log n)。
     * 
     * @param KT 键的类型
     * @param VT 值的类型
     * @param NAME 自定义的map类型名称
     * 
     * @note 生成的类型包括：
     *       - cmap_NAME: map容器类型
     *       - cmap_NAME_node: 节点类型
     *       - cmap_NAME_color: 节点颜色枚举（红黑）
     *       - cmap_NAME_cmp_cb: 比较函数回调类型
     * @note 生成的函数包括create, destroy, insert, find, remove等完整操作
     * 
     * 使用示例:
     * @code
     * DEFINE_CMAP_AS(int, char*, mymap);
     * cmap_mymap *m = cmap_mymap_create();
     * cmap_mymap_insert(m, 1, "hello");
     * cmap_mymap_destroy(&m);
     * @endcode
     */
#define DEFINE_CMAP_AS(KT, VT, NAME)                                                                                                           \
    typedef enum cmap_##NAME##_color{                                                                                                          \
        cmap_##NAME##_RED,                                                                                                                     \
        cmap_##NAME##_BLACK} cmap_##NAME##_color;                                                                                              \
                                                                                                                                               \
    typedef struct cmap_##NAME##_node                                                                                                          \
    {                                                                                                                                          \
        const KT * const key_ptr; /*注意：千万注意，本模块中的key_ptr指针指向的值和内容都不可直接修改，只能使用map的insert,remove,erase,clear修改*/   \
        VT * const value_ptr; /*注意：千万注意，本模块中的value_ptr指针不可直接修改，只能使用map的insert,remove,erase,clear修改*/                   \
        cmap_##NAME##_color color;                                                                                                             \
        struct cmap_##NAME##_node *left;                                                                                                       \
        struct cmap_##NAME##_node *right;                                                                                                      \
        struct cmap_##NAME##_node *parent;                                                                                                     \
    } cmap_##NAME##_node;                                                                                                                      \
                                                                                                                                               \
    /**
     * @brief 键比较函数回调类型
     * 
     * @param lhs 左操作数键的指针
     * @param rhs 右操作数键的指针
     * @return 返回值<0表示lhs<rhs, =0表示lhs==rhs, >0表示lhs>rhs
     */                                                                       \
    typedef int (*cmap_##NAME##_cmp_cb)(const KT *lhs, const KT *rhs);                                                                         \
                                                                                                                                               \
    typedef struct cmap_##NAME                                                                                                                 \
    {                                                                                                                                          \
        size_t size;                                                                                                                           \
        cmap_##NAME##_node *root;                                                                                                              \
        cmap_##NAME##_node *nil;                                                                                                               \
        cmap_##NAME##_cmp_cb cmp_cb;                                                                                                           \
    } cmap_##NAME;                                                                                                                             \
                                                                                                                                               \
    /**
     * @brief 默认的键比较函数，使用memcmp比较
     * 
     * @param lhs 左操作数键的指针
     * @param rhs 右操作数键的指针
     * @return 返回值<0表示lhs<rhs, =0表示lhs==rhs, >0表示lhs>rhs
     */                                                                          \
    static INLINE int cmap_##NAME##_cmp(const KT *lhs, const KT *rhs)                                                                          \
    {                                                                                                                                          \
        return memcmp(lhs, rhs, sizeof(KT));                                                                                                   \
    }                                                                                                                                          \
    /**
     * @brief 创建一个map对象
     * 
     * 使用默认的memcmp比较函数创建map。
     * 
     * @return 成功返回map指针，失败返回NULL
     * 
     * @note 使用完毕后需要调用destroy函数释放内存
     * @note 默认比较函数适用于POD类型，对于复杂类型请使用create_with_cmp
     * 
     * 使用示例:
     * @code
     * cmap_int_double *map = cmap_int_double_create();
     * if (map) {
     *     // 使用map...
     *     cmap_int_double_destroy(&map);
     * }
     * @endcode
     */                                                                         \
    static INLINE cmap_##NAME *cmap_##NAME##_create()                                                                                          \
    {                                                                                                                                          \
        return (cmap_##NAME *)__cmap_create((__cmap_t_cmp_cb)cmap_##NAME##_cmp, sizeof(cmap_##NAME), sizeof(cmap_##NAME##_node));              \
    }                                                                                                                                          \
    /**
     * @brief 使用自定义比较函数创建一个map对象
     * 
     * @param cb 自定义的键比较函数
     * @return 成功返回map指针，失败返回NULL
     * 
     * @note 使用完毕后需要调用destroy函数释放内存
     * @note 比较函数需在整个map生命周期内保持有效
     * 
     * 使用示例:
     * @code
     * int my_cmp(const int *a, const int *b) { return *b - *a; } // 降序
     * cmap_int_double *map = cmap_int_double_create_with_cmp(my_cmp);
     * @endcode
     */                                         \
    static INLINE cmap_##NAME *cmap_##NAME##_create_with_cmp(cmap_##NAME##_cmp_cb cb)                                                          \
    {                                                                                                                                          \
        return (cmap_##NAME *)__cmap_create((__cmap_t_cmp_cb)cb, sizeof(cmap_##NAME), sizeof(cmap_##NAME##_node));                             \
    }                                                                                                                                          \
    /**
     * @brief 销毁map对象并释放所有内存
     * 
     * @param pmap 指向map指针的指针
     * 
     * @note 函数会自动将*pmap设置为NULL
     * @note 如果pmap或*pmap为NULL，函数不执行任何操作
     * @note 销毁后map中的所有键值对都会被释放
     * 
     * 使用示例:
     * @code
     * cmap_int_double *map = cmap_int_double_create();
     * // 使用map...
     * cmap_int_double_destroy(&map);  // map将被设置为NULL
     * @endcode
     */                                                              \
    static INLINE void cmap_##NAME##_destroy(cmap_##NAME **pmap)                                                                               \
    {                                                                                                                                          \
        if (pmap && *pmap)                                                                                                                     \
        {                                                                                                                                      \
            __cmap_destroy((__cmap_t *)(*pmap));                                                                                               \
            *pmap = 0;                                                                                                                         \
        }                                                                                                                                      \
    }                                                                                                                                          \
    /**
     * @brief 初始化一个已分配的map对象
     * 
     * 用于栈上分配或结构体成员的map初始化。
     * 
     * @param map 指向待初始化的map对象的指针
     * @return 成功返回0，失败返回非0值
     * 
     * @note 使用完毕后需要调用uninit函数清理
     * @note 使用默认的memcmp比较函数
     * 
     * 使用示例:
     * @code
     * cmap_int_double map;
     * if (cmap_int_double_init(&map) == 0) {
     *     // 使用map...
     *     cmap_int_double_uninit(&map);
     * }
     * @endcode
     */                                    \
    static INLINE int cmap_##NAME##_init(cmap_##NAME *map)                                                                                     \
    {                                                                                                                                          \
        return __cmap_init((__cmap_t *)(map), (__cmap_t_cmp_cb)cmap_##NAME##_cmp, sizeof(cmap_##NAME), sizeof(cmap_##NAME##_node));            \
    }                                                                                                                                          \
    /**
     * @brief 使用自定义比较函数初始化一个已分配的map对象
     * 
     * @param map 指向待初始化的map对象的指针
     * @param cb 自定义的键比较函数
     * @return 成功返回0，失败返回非0值
     * 
     * @note 使用完毕后需要调用uninit函数清理
     * 
     * 使用示例:
     * @code
     * int my_cmp(const int *a, const int *b) { return *b - *a; }
     * cmap_int_double map;
     * cmap_int_double_init_with_cmp(&map, my_cmp);
     * @endcode
     */                                  \
    static INLINE int cmap_##NAME##_init_with_cmp(cmap_##NAME *map, cmap_##NAME##_cmp_cb cb)                                                   \
    {                                                                                                                                          \
        return __cmap_init((__cmap_t *)(map), (__cmap_t_cmp_cb)cb, sizeof(cmap_##NAME), sizeof(cmap_##NAME##_node));                           \
    }                                                                                                                                          \
    /**
     * @brief 清理map对象并释放其内部资源
     * 
     * @param map 指向map对象的指针
     * 
     * @note 该函数不释放map对象本身的内存，仅释放map内部资源
     * @note 适用于栈上分配的map对象
     * @note 调用后map对象不可再使用，除非重新init
     * 
     * 使用示例:
     * @code
     * cmap_int_double map;
     * cmap_int_double_init(&map);
     * // 使用map...
     * cmap_int_double_uninit(&map);
     * @endcode
     */                                                                 \
    static INLINE void cmap_##NAME##_uninit(cmap_##NAME *map)                                                                                  \
    {                                                                                                                                          \
        __cmap_uninit((__cmap_t *)(map));                                                                                                      \
    }                                                                                                                                          \
    /**
     * @brief 向map中插入一个键值对
     * 
     * 如果键已存在，则插入失败。
     * 
     * @param map 指向map对象的指针
     * @param key 要插入的键
     * @param value 要插入的值
     * @return 成功返回0，如果键已存在或内存不足返回非0值
     * 
     * @note 时间复杂度为O(log n)
     * @note 如果需要覆盖已存在的值，请使用insert_or_assign
     * 
     * 使用示例:
     * @code
     * cmap_int_double *map = cmap_int_double_create();
     * if (cmap_int_double_insert(map, 1, 3.14) == 0) {
     *     printf("插入成功\n");
     * }
     * @endcode
     */                                \
    static INLINE int cmap_##NAME##_insert(cmap_##NAME *map, KT key, VT value)                                                                 \
    {                                                                                                                                          \
        return __cmap_insert((__cmap_t *)(map), (const char *)&key, (const char *)&value, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node)); \
    }                                                                                                                                          \
    /**
     * @brief 查找指定键对应的节点
     * 
     * @param map 指向map对象的指针
     * @param key 要查找的键
     * @return 找到返回节点指针，未找到返回end迭代器
     * 
     * @note 时间复杂度为O(log n)
     * @note 返回的节点可用于获取key_ptr和value_ptr
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     double v = *node->value_ptr;
     * }
     * @endcode
     */                            \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_find(cmap_##NAME *map, KT key)                                                             \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_find((__cmap_t *)(map), (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));   \
    }                                                                                                                                          \
    /**
     * @brief 获取指定键对应的值
     * 
     * @param map 指向map对象的指针
     * @param key 要查找的键
     * @return 返回对应的值
     * 
     * @note 时间复杂度为O(log n)
     * @note 如果键不存在，行为未定义，请先用find或contains检查
     * 
     * 使用示例:
     * @code
     * if (cmap_int_double_contains(map, 1)) {
     *     double v = cmap_int_double_at(map, 1);
     *     printf("value = %f\n", v);
     * }
     * @endcode
     */                               \
    static INLINE VT cmap_##NAME##_at(cmap_##NAME *map, KT key)                                                                                \
    {                                                                                                                                          \
        VT *vp = (VT *)__cmap_at((__cmap_t *)(map), (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                   \
        return *vp;                                                                                                                            \
    }                                                                                                                                          \
    /**
     * @brief 获取指定键对应的值的指针
     * 
     * @param map 指向map对象的指针
     * @param key 要查找的键
     * @return 返回指向值的指针
     * 
     * @note 时间复杂度为O(log n)
     * @note 如果键不存在，行为未定义
     * @note 返回的指针在map被修改后可能失效
     * 
     * 使用示例:
     * @code
     * double *vp = cmap_int_double_at_ptr(map, 1);
     * if (vp) *vp = 2.71;  // 直接修改值
     * @endcode
     */                          \
    static INLINE VT *cmap_##NAME##_at_ptr(cmap_##NAME *map, KT key)                                                                           \
    {                                                                                                                                          \
        VT *vp = (VT *)__cmap_at((__cmap_t *)(map), (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                   \
        return vp;                                                                                                                             \
    }                                                                                                                                          \
    /**
     * @brief 根据键删除map中的元素
     * 
     * @param map 指向map对象的指针
     * @param key 要删除的键
     * 
     * @note 时间复杂度为O(log n)
     * @note 如果键不存在，函数不执行任何操作
     * 
     * 使用示例:
     * @code
     * cmap_int_double_insert(map, 1, 3.14);
     * cmap_int_double_remove(map, 1);  // 删除键为1的元素
     * @endcode
     */                                                         \
    static INLINE void cmap_##NAME##_remove(cmap_##NAME *map, KT key)                                                                          \
    {                                                                                                                                          \
        __cmap_remove((__cmap_t *)(map), (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                              \
    }                                                                                                                                          \
    /**
     * @brief 根据节点迭代器删除map中的元素
     * 
     * @param map 指向map对象的指针
     * @param node 要删除的节点迭代器
     * 
     * @note 时间复杂度为O(log n)
     * @note node不能为end迭代器
     * @note 删除后node指针失效，不可再使用
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     cmap_int_double_erase(map, node);
     * }
     * @endcode
     */                                        \
    static INLINE void cmap_##NAME##_erase(cmap_##NAME *map, cmap_##NAME##_node *node)                                                         \
    {                                                                                                                                          \
        __cmap_erase((__cmap_t *)(map), (__cmap_node_t *)node);                                                                                \
    }                                                                                                                                          \
    /**
     * @brief 清空map中的所有元素
     * 
     * @param map 指向map对象的指针
     * 
     * @note 时间复杂度为O(n)
     * @note 清空后map的size变为0
     * @note 所有迭代器失效
     * 
     * 使用示例:
     * @code
     * cmap_int_double_clear(map);
     * // 现在map为空
     * @endcode
     */                                                                  \
    static INLINE void cmap_##NAME##_clear(cmap_##NAME *map)                                                                                   \
    {                                                                                                                                          \
        __cmap_clear((__cmap_t *)(map));                                                                                                       \
    }                                                                                                                                          \
    /**
     * @brief 获取map中元素的数量
     * 
     * @param map 指向map对象的指针
     * @return 返回map中元素的数量
     * 
     * @note 时间复杂度为O(1)
     * 
     * 使用示例:
     * @code
     * size_t count = cmap_int_double_size(map);
     * printf("map有%zu个元素\n", count);
     * @endcode
     */                                                                 \
    static INLINE size_t cmap_##NAME##_size(cmap_##NAME *map)                                                                                  \
    {                                                                                                                                          \
        return __cmap_size((__cmap_t *)(map));                                                                                                 \
    }                                                                                                                                          \
    /**
     * @brief 检查map是否为空
     * 
     * @param map 指向map对象的指针
     * @return map为空返回非0值，不为空返回0
     * 
     * @note 时间复杂度为O(1)
     * 
     * 使用示例:
     * @code
     * if (cmap_int_double_empty(map)) {
     *     printf("map是空的\n");
     * }
     * @endcode
     */                                                                   \
    static INLINE int cmap_##NAME##_empty(cmap_##NAME *map)                                                                                    \
    {                                                                                                                                          \
        return __cmap_empty((__cmap_t *)(map));                                                                                                \
    }                                                                                                                                          \
    /**
     * @brief 获取指向第一个元素的迭代器
     * 
     * @param map 指向map对象的指针
     * @return 返回指向第一个元素的迭代器，如果map为空则返回end
     * 
     * @note 时间复杂度为O(log n)
     * @note 元素按键的升序排列
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *it = cmap_int_double_begin(map);
     * if (it != cmap_int_double_end(map)) {
     *     int k = *it->key_ptr;
     *     double v = *it->value_ptr;
     * }
     * @endcode
     */                                   \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_begin(cmap_##NAME *map)                                                                    \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_begin((__cmap_t *)(map));                                                                          \
    }                                                                                                                                          \
    /**
     * @brief 获取表示末尾的迭代器
     * 
     * @param map 指向map对象的指针
     * @return 返回表示末尾的哨兵迭代器
     * 
     * @note 时间复杂度为O(1)
     * @note end迭代器不指向有效元素，不能解引用
     * 
     * 使用示例:
     * @code
     * for (cmap_int_double_node *it = cmap_int_double_begin(map);
     *      it != cmap_int_double_end(map);
     *      it = cmap_int_double_next(map, it)) {
     *     // 处理元素
     * }
     * @endcode
     */                                     \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_end(cmap_##NAME *map)                                                                      \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_end((__cmap_t *)(map));                                                                            \
    }                                                                                                                                          \
    /**
     * @brief 获取当前节点的下一个节点
     * 
     * @param map 指向map对象的指针
     * @param node 当前节点
     * @return 返回下一个节点，如果已是最后一个则返回end
     * 
     * @note 时间复杂度为O(log n)，均摊O(1)
     * @note node不能为end迭代器
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *next = cmap_int_double_next(map, current);
     * @endcode
     */                          \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_next(cmap_##NAME *map, cmap_##NAME##_node *node)                                           \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_next((__cmap_t *)(map), (__cmap_node_t *)node);                                                    \
    }                                                                                                                                          \
    /**
     * @brief 获取当前节点的前一个节点
     * 
     * @param map 指向map对象的指针
     * @param node 当前节点
     * @return 返回前一个节点，如果已是第一个则返回end
     * 
     * @note 时间复杂度为O(log n)，均摊O(1)
     * @note 可对end迭代器调用以获取最后一个元素
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *last = cmap_int_double_prev(map, cmap_int_double_end(map));
     * @endcode
     */                          \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_prev(cmap_##NAME *map, cmap_##NAME##_node *node)                                           \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_prev((__cmap_t *)(map), (__cmap_node_t *)node);                                                    \
    }                                                                                                                                          \
    /**
     * @brief 从节点获取键的值
     * 
     * @param node 节点迭代器
     * @return 返回键的副本
     * 
     * @note 时间复杂度为O(1)
     * @note node不能为end迭代器
     * @note 也可以直接使用node->key_ptr访问键
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     int key = cmap_int_double_get_key(node);
     * }
     * @endcode
     */                          \
    static INLINE KT cmap_##NAME##_get_key(cmap_##NAME##_node *node)                                                                           \
    {                                                                                                                                          \
        KT *re = (KT *)__cmap_get_key((__cmap_node_t *)node, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                              \
        return *re;                                                                                                                            \
    }                                                                                                                                          \
    /**
     * @brief 从节点获取值的副本
     * 
     * @param node 节点迭代器
     * @return 返回值的副本
     * 
     * @note 时间复杂度为O(1)
     * @note node不能为end迭代器
     * @note 也可以直接使用node->value_ptr访问值
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     double val = cmap_int_double_get_value(node);
     * }
     * @endcode
     */                        \
    static INLINE VT cmap_##NAME##_get_value(cmap_##NAME##_node *node)                                                                         \
    {                                                                                                                                          \
        VT *re = (VT *)__cmap_get_value((__cmap_node_t *)node, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                            \
        return *re;                                                                                                                            \
    }                                                                                                                                          \
    /**
     * @brief 设置节点的值
     * 
     * @param node 节点迭代器
     * @param value 新的值
     * 
     * @note 时间复杂度为O(1)
     * @note node不能为end迭代器
     * @note 也可以直接通过node->value_ptr修改值
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     cmap_int_double_set_value(node, 2.71);
     * }
     * @endcode
     */                      \
    static INLINE void cmap_##NAME##_set_value(cmap_##NAME##_node *node, const VT value)                                                       \
    {                                                                                                                                          \
        __cmap_set_value((__cmap_node_t *)node, (const char *)&value, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                     \
    }                                                                                                                                          \
    /**
     * @brief 检查map中是否包含指定的键
     * 
     * @param map 指向map对象的指针
     * @param key 要检查的键
     * @return 包含返回非0值，不包含返回0
     * 
     * @note 时间复杂度为O(log n)
     * 
     * 使用示例:
     * @code
     * if (cmap_int_double_contains(map, 1)) {
     *     printf("map包含键1\n");
     * }
     * @endcode
     */                                                  \
    static INLINE int cmap_##NAME##_contains(cmap_##NAME *map, const KT key)                                                                   \
    {                                                                                                                                          \
        return __cmap_contains((__cmap_t *)map, (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                       \
    }                                                                                                                                          \
    /**
     * @brief 插入或更新键值对
     * 
     * 如果键不存在则插入，如果键已存在则更新其对应的值。
     * 
     * @param map 指向map对象的指针
     * @param key 要插入或更新的键
     * @param value 要插入或更新的值
     * @return 成功返回0，失败返回非0值
     * 
     * @note 时间复杂度为O(log n)
     * @note 与insert的区别在于，本函数会覆盖已存在的值
     * 
     * 使用示例:
     * @code
     * cmap_int_double_insert_or_assign(map, 1, 3.14);  // 插入
     * cmap_int_double_insert_or_assign(map, 1, 2.71);  // 更新
     * @endcode
     */                     \
    static INLINE int cmap_##NAME##_insert_or_assign(cmap_##NAME *map, KT key, VT value)                                                      \
    {                                                                                                                                          \
        return __cmap_insert_or_assign((__cmap_t *)(map), (const char *)&key, (const char *)&value, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node)); \
    }                                                                                                                                          \
    /**
     * @brief 统计指定键的元素数量
     * 
     * @param map 指向map对象的指针
     * @param key 要统计的键
     * @return 返回键的数量（对于map只能是0或1）
     * 
     * @note 时间复杂度为O(log n)
     * @note 对于map，返回值只能是0或1，相当于contains的另一种表达
     * 
     * 使用示例:
     * @code
     * size_t cnt = cmap_int_double_count(map, 1);
     * if (cnt > 0) printf("找到了\n");
     * @endcode
     */                                 \
    static INLINE size_t cmap_##NAME##_count(cmap_##NAME *map, const KT key)                                                                  \
    {                                                                                                                                          \
        return __cmap_count((__cmap_t *)map, (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                         \
    }                                                                                                                                          \
    /**
     * @brief 查找第一个不小于给定键的元素
     * 
     * @param map 指向map对象的指针
     * @param key 要查找的键
     * @return 返回第一个键不小于key的节点，如果不存在则返回end
     * 
     * @note 时间复杂度为O(log n)
     * @note 如果key存在，返回该键对应的节点
     * @note 如果key不存在，返回第一个大于key的节点
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *lb = cmap_int_double_lower_bound(map, 5);
     * // lb指向第一个键>=5的元素
     * @endcode
     */                    \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_lower_bound(cmap_##NAME *map, KT key)                                                     \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_lower_bound((__cmap_t *)(map), (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node)); \
    }                                                                                                                                          \
    /**
     * @brief 查找第一个大于给定键的元素
     * 
     * @param map 指向map对象的指针
     * @param key 要查找的键
     * @return 返回第一个键大于key的节点，如果不存在则返回end
     * 
     * @note 时间复杂度为O(log n)
     * @note 无论key是否存在，都返回第一个大于key的节点
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *ub = cmap_int_double_upper_bound(map, 5);
     * // ub指向第一个键>5的元素
     * @endcode
     */                    \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_upper_bound(cmap_##NAME *map, KT key)                                                     \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_upper_bound((__cmap_t *)(map), (const char *)&key, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node)); \
    }                                                                                                                                          \
    /**
     * @brief 交换两个map的内容
     * 
     * @param map1 第一个map
     * @param map2 第二个map
     * 
     * @note 时间复杂度为O(1)
     * @note 交换后所有迭代器保持有效，但指向的map对象改变
     * 
     * 使用示例:
     * @code
     * cmap_int_double *map1 = cmap_int_double_create();
     * cmap_int_double *map2 = cmap_int_double_create();
     * cmap_int_double_swap(map1, map2);  // 交换内容
     * @endcode
     */                              \
    static INLINE void cmap_##NAME##_swap(cmap_##NAME *map1, cmap_##NAME *map2)                                                               \
    {                                                                                                                                          \
        __cmap_swap((__cmap_t *)map1, (__cmap_t *)map2);                                                                                       \
    }                                                                                                                                          \
    /**
     * @brief 获取map的最大容量
     * 
     * @param map 指向map对象的指针
     * @return 返回理论上的最大容量
     * 
     * @note 时间复杂度为O(1)
     * @note 实际可用容量取决于系统内存
     * 
     * 使用示例:
     * @code
     * size_t max = cmap_int_double_max_size(map);
     * @endcode
     */                                                            \
    static INLINE size_t cmap_##NAME##_max_size(cmap_##NAME *map)                                                                             \
    {                                                                                                                                          \
        return __cmap_max_size((__cmap_t *)map);                                                                                               \
    }                                                                                                                                          \
    /**
     * @brief 获取反向迭代器的起始位置（最后一个元素）
     * 
     * @param map 指向map对象的指针
     * @return 返回指向最后一个元素的迭代器，如果map为空则返回rend
     * 
     * @note 时间复杂度为O(log n)
     * @note 用于反向遍历map
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *rit = cmap_int_double_rbegin(map);
     * if (rit != cmap_int_double_rend(map)) {
     *     // 处理最后一个元素
     * }
     * @endcode
     */                 \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_rbegin(cmap_##NAME *map)                                                                  \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_rbegin((__cmap_t *)(map));                                                                         \
    }                                                                                                                                          \
    /**
     * @brief 获取反向迭代器的结束位置
     * 
     * @param map 指向map对象的指针
     * @return 返回反向迭代器的哨兵位置
     * 
     * @note 时间复杂度为O(1)
     * @note rend迭代器不指向有效元素，不能解引用
     * 
     * 使用示例:
     * @code
     * for (cmap_int_double_node *rit = cmap_int_double_rbegin(map);
     *      rit != cmap_int_double_rend(map);
     *      rit = cmap_int_double_prev(map, rit)) {
     *     // 反向遍历
     * }
     * @endcode
     */                   \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_rend(cmap_##NAME *map)                                                                    \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_rend((__cmap_t *)(map));                                                                           \
    }                                                                                                                                          \
    /**
     * @brief 合并两个map
     * 
     * 将src中的所有元素移动到dest中。如果dest中已存在相同的键，则src中的元素保留在src中。
     * 
     * @param dest 目标map
     * @param src 源map（操作后可能为空或包含冲突的元素）
     * 
     * @note 时间复杂度为O(n*log(m+n))，其中n是src的大小，m是dest的大小
     * @note src中未冲突的元素会被移动到dest，src可能变为空或只包含冲突元素
     * 
     * 使用示例:
     * @code
     * cmap_int_double *dest = cmap_int_double_create();
     * cmap_int_double *src = cmap_int_double_create();
     * // ... 填充dest和src ...
     * cmap_int_double_merge(dest, src);  // 合并
     * @endcode
     */              \
    static INLINE void cmap_##NAME##_merge(cmap_##NAME *dest, cmap_##NAME *src)                                                               \
    {                                                                                                                                          \
        __cmap_merge((__cmap_t *)dest, (__cmap_t *)src);                                                                                       \
    }                                                                                                                                          \
    /**
     * @brief 从节点获取键的指针
     * 
     * @param node 节点迭代器
     * @return 返回指向键的const指针
     * 
     * @note 时间复杂度为O(1)
     * @note node不能为end迭代器
     * @note 返回的指针在map被修改后可能失效
     * @note 也可以直接使用node->key_ptr
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     const int *key_p = cmap_int_double_get_key_ptr(node);
     * }
     * @endcode
     */                    \
    static INLINE const KT *cmap_##NAME##_get_key_ptr(cmap_##NAME##_node *node)                                                                     \
    {                                                                                                                                          \
        return (const KT *)__cmap_get_key((__cmap_node_t *)node, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                               \
    }                                                                                                                                          \
    /**
     * @brief 从节点获取值的指针
     * 
     * @param node 节点迭代器
     * @return 返回指向值的指针
     * 
     * @note 时间复杂度为O(1)
     * @note node不能为end迭代器
     * @note 返回的指针在map被修改后可能失效
     * @note 也可以直接使用node->value_ptr
     * 
     * 使用示例:
     * @code
     * cmap_int_double_node *node = cmap_int_double_find(map, 1);
     * if (node != cmap_int_double_end(map)) {
     *     double *val_p = cmap_int_double_get_value_ptr(node);
     *     *val_p = 2.71;  // 修改值
     * }
     * @endcode
     */                  \
    static INLINE VT *cmap_##NAME##_get_value_ptr(cmap_##NAME##_node *node)                                                                   \
    {                                                                                                                                          \
        return (VT *)__cmap_get_value((__cmap_node_t *)node, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node));                             \
    }                                                                                                                                          \
    /**
     * @brief 尝试就地构造插入元素（如果键不存在）
     * 
     * 如果键不存在则插入，如果键已存在则不做任何操作。
     * 
     * @param map 指向map对象的指针
     * @param key 要插入的键
     * @param value 要插入的值
     * @return 成功插入返回0，键已存在或失败返回非0值
     * 
     * @note 时间复杂度为O(log n)
     * @note 与insert相同，但语义上表示"尝试就地构造"
     * 
     * 使用示例:
     * @code
     * if (cmap_int_double_try_emplace(map, 1, 3.14) == 0) {
     *     printf("插入成功\n");
     * } else {
     *     printf("键已存在\n");
     * }
     * @endcode
     */                          \
    static INLINE int cmap_##NAME##_try_emplace(cmap_##NAME *map, KT key, VT value)                                                           \
    {                                                                                                                                          \
        return __cmap_try_emplace((__cmap_t *)(map), (const char *)&key, (const char *)&value, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node)); \
    }                                                                                                                                          \
    /**
     * @brief 就地构造插入元素，并返回节点和插入状态
     * 
     * @param map 指向map对象的指针
     * @param key 要插入的键
     * @param value 要插入的值
     * @param inserted 输出参数，非NULL时返回是否插入成功（1表示插入，0表示已存在）
     * @return 返回对应键的节点（新插入或已存在的节点）
     * 
     * @note 时间复杂度为O(log n)
     * @note 无论键是否已存在，都返回对应的节点
     * @note 通过inserted参数可以判断是否真正插入了新元素
     * 
     * 使用示例:
     * @code
     * int ins = 0;
     * cmap_int_double_node *node = cmap_int_double_emplace(map, 1, 3.14, &ins);
     * if (ins) {
     *     printf("插入了新元素\n");
     * } else {
     *     printf("键已存在，返回已有节点\n");
     * }
     * @endcode
     */              \
    static INLINE cmap_##NAME##_node *cmap_##NAME##_emplace(cmap_##NAME *map, KT key, VT value, int *inserted)                               \
    {                                                                                                                                          \
        return (cmap_##NAME##_node *)__cmap_emplace((__cmap_t *)(map), (const char *)&key, (const char *)&value, sizeof(KT), sizeof(VT), sizeof(cmap_##NAME##_node), inserted); \
    }

/**
 * @brief 正向遍历map的所有元素（支持在遍历中删除当前元素）
 * 
 * 按键的升序遍历map中的所有元素。支持在循环体内安全地调用erase删除当前迭代器。
 * 
 * @param map_ptr 指向map对象的指针
 * @param KT 键的类型
 * @param VT 值的类型
 * @param iter_name 迭代器变量名
 * 
 * @note 不可在foreach循环中修改键的值
 * @note 迭代器类型为 cmap_KT_VT_node*
 * @note 可以通过iter_name->key_ptr和iter_name->value_ptr访问键和值
 * 
 * 使用示例:
 * @code
 * DEFINE_CMAP(int, double);
 * cmap_int_double *map = cmap_int_double_create();
 * // ... 添加元素 ...
 * cmap_foreach(map, int, double, it) {
 *     printf("key=%d, value=%f\n", *it->key_ptr, *it->value_ptr);
 *     if (*it->key_ptr == 5) {
 *         cmap_int_double_erase(map, it);  // 安全删除
 *     }
 * }
 * @endcode
 */
#define cmap_foreach(map_ptr, KT, VT, iter_name) cmap_as_foreach(map_ptr, KT, VT, iter_name, KT##_##VT)

/**
 * @brief 使用自定义类型别名正向遍历map的所有元素
 * 
 * 与cmap_foreach相同，但可以指定自定义的类型别名。
 * 
 * @param map_ptr 指向map对象的指针
 * @param KT 键的类型
 * @param VT 值的类型
 * @param iter_name 迭代器变量名
 * @param alias 自定义的类型别名（对应DEFINE_CMAP_AS的NAME参数）
 * 
 * @note 不可在foreach循环中修改键的值
 * @note 支持在循环体内删除当前元素
 * 
 * 使用示例:
 * @code
 * DEFINE_CMAP_AS(int, double, mymap);
 * cmap_mymap *map = cmap_mymap_create();
 * cmap_as_foreach(map, int, double, it, mymap) {
 *     printf("key=%d, value=%f\n", *it->key_ptr, *it->value_ptr);
 * }
 * @endcode
 */
#define cmap_as_foreach(map_ptr, KT, VT, iter_name, alias)                                        \
    for (int iter_name##_scope_ = 1; iter_name##_scope_; iter_name##_scope_ = 0)              \
        for (cmap_##alias##_node *iter_name = 0, *__iter_name_##_next = 0;                    \
             iter_name##_scope_;                                                               \
             iter_name##_scope_ = 0)                                                           \
            if ((iter_name = cmap_##alias##_begin((map_ptr)),                                 \
                 __iter_name_##_next = cmap_##alias##_next((map_ptr), iter_name), 1))         \
                for (; iter_name != cmap_##alias##_end((map_ptr));                            \
                     iter_name = __iter_name_##_next,                                         \
                     __iter_name_##_next = cmap_##alias##_next((map_ptr), __iter_name_##_next))

/**
 * @brief 反向遍历map的所有元素（支持在遍历中删除当前元素）
 * 
 * 按键的降序遍历map中的所有元素。支持在循环体内安全地调用erase删除当前迭代器。
 * 
 * @param map_ptr 指向map对象的指针
 * @param KT 键的类型
 * @param VT 值的类型
 * @param iter_name 迭代器变量名
 * 
 * @note 不可在foreach循环中修改键的值
 * @note 从最大键向最小键遍历
 * 
 * 使用示例:
 * @code
 * cmap_foreach_reverse(map, int, double, it) {
 *     printf("key=%d, value=%f\n", *it->key_ptr, *it->value_ptr);
 *     // 按降序遍历
 * }
 * @endcode
 */
#define cmap_foreach_reverse(map_ptr, KT, VT, iter_name) cmap_as_foreach_reverse(map_ptr, KT, VT, iter_name, KT##_##VT)

/**
 * @brief 使用自定义类型别名反向遍历map的所有元素
 * 
 * 与cmap_foreach_reverse相同，但可以指定自定义的类型别名。
 * 
 * @param map_ptr 指向map对象的指针
 * @param KT 键的类型
 * @param VT 值的类型
 * @param iter_name 迭代器变量名
 * @param alias 自定义的类型别名
 * 
 * @note 不可在foreach循环中修改键的值
 * @note 支持在循环体内删除当前元素
 * 
 * 使用示例:
 * @code
 * DEFINE_CMAP_AS(int, double, mymap);
 * cmap_mymap *map = cmap_mymap_create();
 * cmap_as_foreach_reverse(map, int, double, it, mymap) {
 *     printf("key=%d\n", *it->key_ptr);
 * }
 * @endcode
 */
#define cmap_as_foreach_reverse(map_ptr, KT, VT, iter_name, alias)                                \
    for (int iter_name##_scope_ = 1; iter_name##_scope_; iter_name##_scope_ = 0)              \
        for (cmap_##alias##_node *iter_name = 0, *__iter_name_##_next = 0;                    \
             iter_name##_scope_;                                                               \
             iter_name##_scope_ = 0)                                                           \
            if ((iter_name = cmap_##alias##_prev((map_ptr), cmap_##alias##_end((map_ptr))),   \
                 __iter_name_##_next = cmap_##alias##_prev((map_ptr), iter_name), 1))         \
                for (; iter_name != cmap_##alias##_end((map_ptr));                            \
                     iter_name = __iter_name_##_next,                                         \
                     __iter_name_##_next = cmap_##alias##_prev((map_ptr), __iter_name_##_next))

/**
 * @brief 使用反向迭代器遍历map的所有元素（支持在遍历中删除当前元素）
 * 
 * 使用rbegin/rend反向迭代器从最大键向最小键遍历。支持在循环体内安全地调用erase删除当前迭代器。
 * 
 * @param map_ptr 指向map对象的指针
 * @param KT 键的类型
 * @param VT 值的类型
 * @param iter_name 迭代器变量名
 * 
 * @note 不可在foreach循环中修改键的值
 * @note 与cmap_foreach_reverse功能相同，但使用rbegin/rend语义
 * 
 * 使用示例:
 * @code
 * cmap_rforeach(map, int, double, rit) {
 *     printf("key=%d, value=%f\n", *rit->key_ptr, *rit->value_ptr);
 *     if (*rit->key_ptr == 5) {
 *         cmap_int_double_erase(map, rit);  // 安全删除
 *     }
 * }
 * @endcode
 */
#define cmap_rforeach(map_ptr, KT, VT, iter_name) cmap_as_rforeach(map_ptr, KT, VT, iter_name, KT##_##VT)

/**
 * @brief 使用自定义类型别名和反向迭代器遍历map的所有元素
 * 
 * 与cmap_rforeach相同，但可以指定自定义的类型别名。
 * 
 * @param map_ptr 指向map对象的指针
 * @param KT 键的类型
 * @param VT 值的类型
 * @param iter_name 迭代器变量名
 * @param alias 自定义的类型别名
 * 
 * @note 不可在foreach循环中修改键的值
 * @note 支持在循环体内删除当前元素
 * 
 * 使用示例:
 * @code
 * DEFINE_CMAP_AS(int, double, mymap);
 * cmap_mymap *map = cmap_mymap_create();
 * cmap_as_rforeach(map, int, double, rit, mymap) {
 *     printf("key=%d\n", *rit->key_ptr);
 * }
 * @endcode
 */
#define cmap_as_rforeach(map_ptr, KT, VT, iter_name, alias)                                      \
    for (int iter_name##_scope_ = 1; iter_name##_scope_; iter_name##_scope_ = 0)            \
        for (cmap_##alias##_node *iter_name = 0, *__iter_name_##_rnext = 0;                 \
             iter_name##_scope_;                                                             \
             iter_name##_scope_ = 0)                                                         \
            if ((iter_name = cmap_##alias##_rbegin((map_ptr)),                              \
                 __iter_name_##_rnext = cmap_##alias##_prev((map_ptr), iter_name), 1))      \
                for (; iter_name != cmap_##alias##_rend((map_ptr));                         \
                     iter_name = __iter_name_##_rnext,                                      \
                     __iter_name_##_rnext = cmap_##alias##_prev((map_ptr), __iter_name_##_rnext))


    // 注意：以下部分是库内部函数，用户不要调用-------------------

    typedef struct __cmap_t __cmap_t;
    typedef struct __cmap_node_t __cmap_node_t;
    typedef int (*__cmap_t_cmp_cb)(const void *lhs, const void *rhs);

    CC_API __cmap_t *CC_CALL __cmap_create(__cmap_t_cmp_cb cmp_cb, size_t map_size, size_t node_size);
    CC_API void CC_CALL __cmap_destroy(__cmap_t *tree);
    CC_API int CC_CALL __cmap_init(__cmap_t *tree, __cmap_t_cmp_cb cmp_cb, size_t map_size, size_t node_size);
    CC_API void CC_CALL __cmap_uninit(__cmap_t *tree);
    CC_API int CC_CALL __cmap_insert(__cmap_t *tree, const char *key_ptr, const char *value_ptr, size_t key_size,
                                     size_t value_size, size_t node_size);
    CC_API __cmap_node_t *CC_CALL __cmap_find(__cmap_t *tree, const char *key_ptr, size_t key_size, size_t value_size, size_t node_size);
    CC_API int CC_CALL __cmap_contains(__cmap_t *m, const char *key, size_t key_size, size_t value_size, size_t node_size);
    CC_API char *CC_CALL __cmap_at(__cmap_t *tree, const char *key_ptr, size_t key_size, size_t value_size, size_t node_size);
    CC_API void CC_CALL __cmap_erase(__cmap_t *tree, __cmap_node_t *node);
    CC_API void CC_CALL __cmap_remove(__cmap_t *tree, const char *key_ptr, size_t key_size, size_t value_size, size_t node_size);
    CC_API void CC_CALL __cmap_clear(__cmap_t *tree);
    CC_API size_t CC_CALL __cmap_size(__cmap_t *tree);
    CC_API int CC_CALL __cmap_empty(__cmap_t *tree);
    CC_API __cmap_node_t *CC_CALL __cmap_begin(__cmap_t *tree);
    CC_API __cmap_node_t *CC_CALL __cmap_next(__cmap_t *tree, __cmap_node_t *current);
    CC_API __cmap_node_t *CC_CALL __cmap_prev(__cmap_t *tree, __cmap_node_t *current);
    CC_API __cmap_node_t *CC_CALL __cmap_end(__cmap_t *tree);
    CC_API char *CC_CALL __cmap_get_key(__cmap_node_t *iter, size_t key_size, size_t value_size, size_t node_size);
    CC_API char *CC_CALL __cmap_get_value(__cmap_node_t *iter, size_t key_size, size_t value_size, size_t node_size);
    CC_API void CC_CALL __cmap_set_value(__cmap_node_t *iter, const char *value, size_t key_size, size_t value_size, size_t node_size);
    CC_API int CC_CALL __cmap_insert_or_assign(__cmap_t *tree, const char *key_ptr, const char *value_ptr, size_t key_size,
                                               size_t value_size, size_t node_size);
    CC_API size_t CC_CALL __cmap_count(__cmap_t *tree, const char *key_ptr, size_t key_size, size_t value_size, size_t node_size);
    CC_API __cmap_node_t *CC_CALL __cmap_lower_bound(__cmap_t *tree, const char *key_ptr, size_t key_size, size_t value_size, size_t node_size);
    CC_API __cmap_node_t *CC_CALL __cmap_upper_bound(__cmap_t *tree, const char *key_ptr, size_t key_size, size_t value_size, size_t node_size);
    CC_API void CC_CALL __cmap_swap(__cmap_t *tree1, __cmap_t *tree2);
    CC_API size_t CC_CALL __cmap_max_size(__cmap_t *tree);
    CC_API __cmap_node_t *CC_CALL __cmap_rbegin(__cmap_t *tree);
    CC_API __cmap_node_t *CC_CALL __cmap_rend(__cmap_t *tree);
    CC_API void CC_CALL __cmap_merge(__cmap_t *dest, __cmap_t *src);
    CC_API int CC_CALL __cmap_try_emplace(__cmap_t *tree, const char *key_ptr, const char *value_ptr, size_t key_size,
                                          size_t value_size, size_t node_size);
    CC_API __cmap_node_t *CC_CALL __cmap_emplace(__cmap_t *tree, const char *key_ptr, const char *value_ptr, size_t key_size,
                                                  size_t value_size, size_t node_size, int *inserted);

#ifdef __cplusplus
}
#endif

#endif

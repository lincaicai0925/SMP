#ifndef C_CORE_PTRC_H
#define C_CORE_PTRC_H

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
#else
#define CC_CALL
#define CC_API
#endif

    /** \addtogroup cptr_container 指针容器
     * 用于库的作者，在纯C库中实现防空指针和防野指针的功能
     * @{
     */

    /**
     * @brief 递归互斥锁类型（基于cthread.h的c_recursive_mutex封装）
     */
    typedef struct cthread_recursive_mutex
    {
        unsigned char opaque[64]; /*!< 不透明存储，用于存放c_recursive_mutex */
    } cthread_recursive_mutex;

    /**
     * \struct cptr_container 指针容器 
     * 
     */
    typedef struct cptr_container
    {
        long flag;   /*!< 标记，用户不应访问此成员 */
        void *impl; /*!< 标记，用户不应访问此成员 */
    } cptr_container;

    /**
     * 指针遍历回调
     * @param self 总线容器
     * @param index 当前节点名称
     * @param ptr 指针
     * @param userdata 用户数据
     * @return 返回是否退出，非0表示结束遍历，0表示继续
     */
    typedef int(CC_CALL *cptrc_foreach_cb)(
        cptr_container *self,
        int index,
        void *ptr,
        void *userdata);

    /**
     * @brief 创建指针
     * 
     * @param self 容器
     * @param size 长度
     * @param mutex 互斥对象
     * @return 指针
     */
    CC_API void *CC_CALL cptrc_create_ptr(cptr_container *self, int size, cthread_recursive_mutex *mutex);

    /**
     * @brief 托管指针
     * 
     * @param self 容器
     * @param ptr 指针
     * @param mutex 互斥对象
     * @return 无 
     */
    CC_API void CC_CALL cptrc_add(cptr_container *self, void *ptr, cthread_recursive_mutex *mutex);

    /**
     * @brief 锁定指针
     * 
     * @param self 容器
     * @param ptr 指针
     * @return bool
     */
    CC_API int CC_CALL cptrc_lock_ptr(cptr_container *self, void *ptr);

    /**
     * @brief 解锁指针
     * 
     * @param self 容器
     * @param ptr 指针
     * @return bool
     */
    CC_API int CC_CALL cptrc_unlock_ptr(cptr_container *self, void *ptr);

    /**
     * @brief 获取内部托管的指针数量
     * 
     * @param self 容器
     * @return 数量
     */
    CC_API int CC_CALL cptrc_get_size(cptr_container *self);

    /**
     * @brief 销毁指针
     * 
     * @param self 容器
     * @param ptr 指针
     * @return 无
     */
    CC_API void CC_CALL cptrc_destroy(cptr_container *self, void *ptr);

    /**
     * @brief 重置容器
     * 
     * @param self 容器
     * @return 无
     */
    CC_API void CC_CALL cptrc_reset(cptr_container *self);

    /**
     * @brief 遍历所有指针
     * 
     * @param self 容器
     * @param cb 回调
     * @param cb_userdata 回调自定义参数
     * @return 无
     */
    CC_API void CC_CALL cptrc_foreach_ptr(cptr_container *self, cptrc_foreach_cb cb, void *cb_userdata);
    /** @}*/

//----------------------------------------------------------------------------------------------------------------------------------------------------------

    /** \addtogroup cptr_container 指针容器
     * 用于库的作者，在纯C库中实现防空指针和防野指针的功能
     * @{
     */

    /**
     * @brief 读写互斥锁类型（基于cthread.h的c_shared_mutex封装）
     */
    typedef struct cthread_rw_mutex
    {
        unsigned char opaque[128]; /*!< 不透明存储，用于存放c_shared_mutex */
    } cthread_rw_mutex;

    /**
     * \struct cptr_container 指针容器 
     * 
     */
    typedef struct cptr_rw
    {
        long flag;   /*!< 标记，用户不应访问此成员 */
        void *impl; /*!< 标记，用户不应访问此成员 */
    } cptr_rw;

    /**
     * 指针遍历回调
     * @param self 总线容器
     * @param index 当前节点名称
     * @param ptr 指针
     * @param userdata 用户数据
     * @return 返回是否退出，非0表示结束遍历，0表示继续
     */
    typedef int(CC_CALL *cptr_rw_foreach_cb)(
        cptr_rw *self,
        int index,
        void *ptr,
        void *userdata);

    /**
     * @brief 托管指针
     * 
     * @param self 容器
     * @param ptr 指针
     * @param mutex 互斥对象
     * @return 返回托管后的指针 
     */
    CC_API int CC_CALL cptr_rw_add(cptr_rw *self, void *ptr, cthread_rw_mutex *mutex);

    /**
     * @brief 读锁定指针
     * 
     * @param self 容器
     * @param ptr_handle 托管的指针钉句柄
     * @return 锁定成功的实际指针，如果返回空，表示锁定失败
     */
    CC_API void * CC_CALL cptr_rw_read_lock_ptr(cptr_rw *self, int ptr_handle);

    /**
     * @brief 锁定指针
     * 
     * @param self 容器
     * @param ptr_handle 托管的指针钉句柄
     * @return 锁定成功的实际指针，如果返回空，表示锁定失败
     */
    CC_API void * CC_CALL cptr_rw_write_lock_ptr(cptr_rw *self, int ptr_handle);


    /**
     * @brief 获取内部托管的指针数量
     * 
     * @param self 容器
     * @return 数量
     */
    CC_API int CC_CALL cptr_rw_get_size(cptr_rw *self);


    /**
     * @brief 删除指针
     * 
     * @param self 容器
     * @param ptr_handle 指针句柄
     * @param unlock 指针句柄
     * @return bool
     */
    CC_API void * CC_CALL cptr_rw_write_lock_ptr_and_erase(cptr_rw *self, int ptr_handle);

    /**
     * @brief 重置容器
     * 
     * @param self 容器
     * @return 无
     */
    CC_API void CC_CALL cptr_rw_reset(cptr_rw *self);

    /**
     * @brief 遍历所有指针
     * 
     * @param self 容器
     * @param cb 回调
     * @param cb_userdata 回调自定义参数
     * @return 无
     */
    CC_API void CC_CALL cptr_rw_foreach_ptr(cptr_rw *self, cptr_rw_foreach_cb cb, void *cb_userdata);
    /** @}*/

#ifdef __cplusplus
}
#endif
#endif
#ifndef C_CORE_TLS_H_
#define C_CORE_TLS_H_

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
#endif
    /** \addtogroup CTLS 线程本地存储
     * @{
     */

#define CTLS_DEF_ID(var)                     \
    static volatile long s_tls_lock_id_##var = 0; \
    static volatile long var = -1;

#define CTLS_MAKE_ID(var) ctls_auto_make_res_id(&s_tls_lock_id_##var, &var)

    typedef void(CC_CALL *ctls_reset_cb)(void *ptr, void *user_data);
    /**
     * @brief 用于从线程本地存储返回大对象
     *      使用此函数返回变量的性能开销与返回600-1000字节相当,(在atlas(600)和x86(1000)上测试)
     * @param id
     * @param size
     * @param freefun
     * @param user_data
     * @return void*
     */
    CC_API void *CC_CALL ctls_alloc_or_reset(long id, unsigned int size, ctls_reset_cb freefun, void *user_data);

    CC_API void *CC_CALL ctls_re_alloc(long id, unsigned int size, ctls_reset_cb freefun, void *user_data);
    CC_API void CC_CALL ctls_set(long id, void *ptr, ctls_reset_cb freefun, void *user_data);
    CC_API void *CC_CALL ctls_get(long id);
    CC_API void CC_CALL ctls_free(long id);

    CC_API void CC_CALL ctls_auto_make_res_id(volatile long *static_self_lock_id, volatile long *tls_id);

    CC_API void *CC_CALL ctls_get_tmp_ptr();

    /** @}*/

#ifdef __cplusplus
}
#endif

#endif

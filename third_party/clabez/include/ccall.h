#ifndef HL_CALL_H_
#define HL_CALL_H_

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

#if defined(_WIN32) || defined(_WIN64)
    typedef long fast_call_st;
#else
typedef int fast_call_st;
#endif

#define DEFINE_CCALL_ONCE_FLAG(name) static ccall_once_flag name = {0};
    typedef struct ccall_once_flag ccall_once_flag;
    typedef int(CC_CALL *ccall_once_cb)(void *user_data);
    CC_API void CC_CALL ccall_once(ccall_once_flag *flag, ccall_once_cb cb, void *user_data);
    CC_API int CC_CALL ccall_once_result(ccall_once_flag *flag, ccall_once_cb cb, void *user_data);
    CC_API int CC_CALL ccall_once_result_timeout(ccall_once_flag *flag, ccall_once_cb cb, void *user_data, unsigned timeout_ms);

    typedef struct cfast_call_once_flag
    {
        volatile fast_call_st state;
    } cfast_call_once_flag;

    CC_API void CC_CALL cfast_call_once(cfast_call_once_flag *flag, ccall_once_cb cb, void *user_data);
    CC_API int CC_CALL cfast_call_once_result(cfast_call_once_flag *flag, ccall_once_cb cb, void *user_data);

    typedef struct ccall_once_flag
    {
        volatile int _;  // 私有成员，用户不要访问
        volatile int __; // 私有成员，用户不要访问
        char ___[120];   // 私有成员，用户不要访问
    } ccall_once_flag;

#ifdef __cplusplus
}
#endif
#endif

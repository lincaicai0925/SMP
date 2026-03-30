/**
 * @file cwsa_shared.h
 * @brief 共享的 Windows Winsock 初始化/清理管理
 */

#ifndef CWSA_SHARED_H
#define CWSA_SHARED_H


#ifdef __cplusplus
extern "C" {
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


/**
 * @brief 增加 WSA 引用计数并在需要时初始化
 * @return 0 成功，非0失败
 * @note 线程安全，使用原子操作
 */
CC_API int CC_CALL cwsa_shared_startup(void);

/**
 * @brief 减少 WSA 引用计数并在需要时清理
 * @note 线程安全，使用原子操作
 */
CC_API void CC_CALL cwsa_shared_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* CWSA_SHARED_H */


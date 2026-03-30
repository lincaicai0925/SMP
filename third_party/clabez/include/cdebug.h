#ifndef C_CORE_DEBUG_H_
#define C_CORE_DEBUG_H_

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
    /** \addtogroup clog_debug 调试
     * @{
     */

    /**
     * @brief 输出到标准控制台和DebugView
     *
     * @param to_debugview
     * @param to_stdout
     * @param to_stderr
     * @param format 内存地址
     * @return 无
     */
    CC_API void CC_CALL cdebug_printf(int to_debugview, int to_stdout, int to_stderr, char const *const format, ...);

    /**
     * @brief 内存转字符串
     *
     * @param buf 输入内存
     * @param buf_len 输入内存长度
     * @param out_str 输出字符串
     * @param out_str_len 输出字符串长度
     * @return 长度
     */
    CC_API int CC_CALL cdebug_buf2str(void *buf, int buf_len, char *out_str, int out_str_len);

    /**
     * @brief 打印内存
     *
     * @param buf 内存地址
     * @param buf_len 内存长度
     * @param head_br 首换行 如："\r\n","\n"
     * @param tail_br 尾换行 如："\r\n","\n"
     * @return 无
     */
    CC_API void CC_CALL cdebug_print_buf(void *buf, int buf_len, char *head_br, char *tail_br);

    #if defined(_WIN32) || defined(_WIN64)
    /**
     * @brief 等待Attach
     * 
     * 注意：如果vs窗口处于最小化状态时，不再自动附加。也就是说，窗口最小化时表示用户不想调试
     *
     * @param vs_wnd_caption 指定使用某个vs窗口附加到进程进行调试
     * @param pid 进程id,如果是0表示当前进程
     * @param timeout_ms 超时时间
     * @param input_speed 输入的速度等级0，表示最快，100表示最慢(最长的操作会导致10s等待)
     * @param force_attach_when_wnd_minimized 1表示即便窗口已经最小化，也附加
     * @return BOOL
     */
    CC_API int CC_CALL cdebug_auto_attach_vs(char *vs_wnd_caption,unsigned int pid, unsigned int timeout_ms, unsigned int input_speed,unsigned int force_attach_when_wnd_minimized);
    #endif

    /** @}*/

#ifdef __cplusplus
}
#endif

#endif
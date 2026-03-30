#ifndef C_CORE_PROC_H_
#define C_CORE_PROC_H_
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

    /** \addtogroup cproc 进程
     * 可用用于获取系统中的进程列表
     * @{
     */

    /**
     * @brief 处理进程输出的回调
     * 回调函数返回0时，表示继续。非0时，表示中断。并且在exec_*函数中返回此值
     *
     */
    typedef int (*cproc_line_cb)(char *line, unsigned int line_size, unsigned int line_index, void *userdata);

    /**
     * \struct cproc_lines 进程输出的行
     *
     */
    typedef struct cproc_lines
    {
        int line_count; /*!< 行数量 */
        char **lines;   /*!< 每行的指针 */
    } cproc_lines;

    /**
     * \struct cproc_detail 进程输出的明细
     *
     */
    typedef struct cproc_detail
    {
        cproc_lines *lines; /*!< 每行的指针 */
        char *content;              /*!< 整形文本 */
    } cproc_detail;

    /**
     * \struct cproc_mem_working_set 内存占用
     *
     */
    typedef struct cproc_mem_working_set
    {
        unsigned long peak_workingset_size;
        unsigned long workingset_size;
    } cproc_mem_working_set;

    /**
     * \struct cproc_handle 进程句柄
     * 
     */
    typedef struct cproc_handle
    {
#if defined(_WIN32) || defined(_WIN64)
        void *hProcess;
        void *hThread;
        unsigned long dwProcessId;
        unsigned long dwThreadId;
#else
        int pid;
#endif
        int is_running;
    } cproc_handle;

    /**
     * \struct cproc_pipe 管道句柄
     *
     */
    typedef struct cproc_pipe
    {
#if defined(_WIN32) || defined(_WIN64)
        void *hRead;
        void *hWrite;
#else
        int fd_read;
        int fd_write;
#endif
    } cproc_pipe;

    /**
     * \struct cproc_stream_cfg 流配置
     *
     */
    typedef struct cproc_stream_cfg
    {
        cproc_pipe *stdin_pipe;   /*!< 标准输入管道 */
        cproc_pipe *stdout_pipe;  /*!< 标准输出管道 */
        cproc_pipe *stderr_pipe;  /*!< 标准错误管道 */
        int inherit_handles;      /*!< 是否继承句柄 */
    } cproc_stream_cfg;

    /**
     * \struct cproc_start_info 进程启动信息
     *
     */
    typedef struct cproc_start_info
    {
        const char *cmd;           /*!< 命令行 */
        const char *work_dir;      /*!< 工作目录 */
        const char **envp;         /*!< 环境变量数组，以NULL结尾 */
        cproc_stream_cfg *streams; /*!< 流配置 */
        int show_window;           /*!< 是否显示窗口(Windows) */
        int create_new_console;    /*!< 是否创建新控制台(Windows) */
        int detached;              /*!< 是否后台运行 */
    } cproc_start_info;

   /**
     * @brief 异步启动进程
     *
     * @param cmd 命令行
     * @param work_dir 工作目录，0，表示当前目录
     * @return 启动结果
     */
    CC_API int CC_CALL cproc_async_exec(const char *cmd,const char * work_dir);

    /**
     * @brief 执行进程，输出到回调
     * 
     * @param cmd 命令行
     * @param work_dir 工作目录
     * @param cb 回调
     * @param userdata 用户数据
     * @return 进行结果代码
     */
    CC_API int CC_CALL cproc_exec_to_cb(const char *cmd, char * work_dir, cproc_line_cb cb, void *userdata);

    /**
     * @brief 执行进程，输出到字符串
     *
     * @param cmd 命令行
     * @param output_buf 输出缓冲区
     * @param output_buf_len 输出缓冲区长度
     * @return 进行结果代码
     */
    CC_API int CC_CALL cproc_exec_to_str(const char *cmd, char *output_buf, int output_buf_len,const char * work_dir);

    /**
     * @brief 创建字符缓冲并执行
     *
     * @param cmd 命令行
     * @return 执行结果
     */
    CC_API char *CC_CALL cproc_create_str_buf_and_exec(const char *cmd);

    /**
     * @brief 销毁cproc_create_str_buf_and_exec产生的结果
     *
     * @param str 字符指针的指针
     * @return 空
     */
    CC_API void CC_CALL cproc_delete_str_buf(char **str);

    /**
     * @brief 创建行并执行
     * 
     * @param cmd 命令行
     * @return 行数组 
     */
    CC_API cproc_lines *CC_CALL cproc_create_lines_and_exec(const char *cmd);

    /**
     * @brief 创建明细并执行
     * 
     * @param cmd 命令行
     * @return 结果指针 
     */
    CC_API cproc_detail *CC_CALL cproc_create_detail_and_exec(const char *cmd);

    /**
     * @brief 删除行数组
     * 
     * @param plines 行数组的指针
     * @return 无
     */
    CC_API void CC_CALL cproc_delete_lines(cproc_lines **plines);

    /**
     * @brief 删除明细
     * 
     * @param pdetail 明细指针的指针
     * @return 无 
     */
    CC_API void CC_CALL cproc_celete_detail(cproc_detail **pdetail);

    /**
     * @brief 通过进程名称获取进程ID
     * 
     * @param name 进程名称
     * @return id
     */
    CC_API unsigned int CC_CALL cproc_get_id_by_name(const char *name);

    /**
     * @brief 通过进程名杀死进程
     * 
     * @param name 进程名
     * @param exit_code 退出代码
     * @return bool
     */
    CC_API int CC_CALL cproc_kill_by_name(const char *name, int exit_code);

    /**
     * @brief 通过进程id杀死进程
     * 
     * @param id 进程id
     * @param exit_code 退出代码
     * @return 无
     */
    CC_API void CC_CALL cproc_kill_by_id(unsigned long id, int exit_code);

    /**
     * @brief 获取单次读取长度
     * 
     * @return 长度
     */
    CC_API int CC_CALL cproc_get_once_read_buf_size();

    typedef enum CPROC_DIR_TYPE
    {
        CPROC_MODULE_DIR,
        CPROC_WORK_DIR
    } CPROC_DIR_TYPE;
   /**
     * @brief 获取当前模块路径
     * @param type 获取文件夹类型
     * @param unix_like_backslash 返回的路径使用是否unix风格的反斜杠，否则使用本地系统风格windows为正斜杠
     * 
     * @return 长度
     */
    CC_API char *CC_CALL cproc_get_dir(CPROC_DIR_TYPE type,int unix_like_backslash);

    /**
     * @brief 创建管道
     * 
     * @param pipe 管道指针
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_pipe_create(cproc_pipe *pipe);

    /**
     * @brief 关闭管道
     * 
     * @param pipe 管道指针
     * @return 无
     */
    CC_API void CC_CALL cproc_pipe_close(cproc_pipe *pipe);

    /**
     * @brief 从管道读取
     * 
     * @param pipe 管道指针
     * @param buf 缓冲区
     * @param size 读取大小
     * @return 实际读取的字节数，-1表示错误
     */
    CC_API int CC_CALL cproc_pipe_read(cproc_pipe *pipe, void *buf, unsigned int size);

    /**
     * @brief 向管道写入
     * 
     * @param pipe 管道指针
     * @param buf 缓冲区
     * @param size 写入大小
     * @return 实际写入的字节数，-1表示错误
     */
    CC_API int CC_CALL cproc_pipe_write(cproc_pipe *pipe, const void *buf, unsigned int size);

    /**
     * @brief 创建进程
     * 
     * @param handle 进程句柄指针
     * @param start_info 启动信息
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_create(cproc_handle *handle, const cproc_start_info *start_info);

    /**
     * @brief 等待进程结束
     * 
     * @param handle 进程句柄指针
     * @param timeout_ms 超时时间(毫秒)，-1表示无限等待
     * @return 0成功，1超时，-1错误
     */
    CC_API int CC_CALL cproc_wait(cproc_handle *handle, int timeout_ms);

    /**
     * @brief 获取进程退出码
     * 
     * @param handle 进程句柄指针
     * @param exit_code 退出码指针
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_get_exit_code(cproc_handle *handle, int *exit_code);

    /**
     * @brief 终止进程
     * 
     * @param handle 进程句柄指针
     * @param exit_code 退出码
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_terminate(cproc_handle *handle, int exit_code);

    /**
     * @brief 检查进程是否在运行
     * 
     * @param handle 进程句柄指针
     * @return 1运行中，0已结束
     */
    CC_API int CC_CALL cproc_is_running(cproc_handle *handle);

    /**
     * @brief 关闭进程句柄
     * 
     * @param handle 进程句柄指针
     * @return 无
     */
    CC_API void CC_CALL cproc_close(cproc_handle *handle);

    /**
     * @brief 在PATH中搜索可执行文件
     * 
     * @param name 程序名
     * @param out_path 输出路径缓冲区
     * @param out_path_len 缓冲区长度
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_search_path(const char *name, char *out_path, int out_path_len);

    /**
     * @brief 创建环境变量数组
     * 
     * @param count 变量数量
     * @return 环境变量数组指针，使用后需调用cproc_env_free释放
     */
    CC_API const char **CC_CALL cproc_env_create(int count);

    /**
     * @brief 设置环境变量
     * 
     * @param envp 环境变量数组
     * @param index 索引
     * @param name 变量名
     * @param value 变量值
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_env_set(const char **envp, int index, const char *name, const char *value);

    /**
     * @brief 复制当前进程环境变量
     * 
     * @return 环境变量数组指针，使用后需调用cproc_env_free释放
     */
    CC_API const char **CC_CALL cproc_env_copy_current();

    /**
     * @brief 释放环境变量数组
     * 
     * @param envp 环境变量数组指针
     * @return 无
     */
    CC_API void CC_CALL cproc_env_free(const char **envp);

    /**
     * @brief 快速启动进程并等待结束
     * 
     * @param cmd 命令行
     * @param work_dir 工作目录
     * @param exit_code 退出码指针（可为NULL）
     * @param timeout_ms 超时时间（毫秒），-1表示无限等待
     * @return 0成功，1超时，-1错误
     */
    CC_API int CC_CALL cproc_run(const char *cmd, const char *work_dir, int *exit_code, int timeout_ms);

    /**
     * @brief 快速启动进程并获取输出
     * 
     * @param cmd 命令行
     * @param output_buf 输出缓冲区
     * @param output_buf_len 输出缓冲区长度
     * @param exit_code 退出码指针（可为NULL）
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_run_get_output(const char *cmd, char *output_buf, int output_buf_len, int *exit_code);

    /**
     * @brief 获取进程ID
     * 
     * @param handle 进程句柄
     * @return 进程ID
     */
    CC_API unsigned long CC_CALL cproc_get_pid(cproc_handle *handle);

    /**
     * @brief 发送信号到进程（Linux）或终止进程（Windows）
     * 
     * @param handle 进程句柄
     * @param signal 信号编号（Linux）或退出码（Windows）
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_signal(cproc_handle *handle, int signal);

    /**
     * @brief 获取进程工作集内存使用情况
     * 
     * @param handle 进程句柄（可为NULL表示当前进程）
     * @param mem_info 内存信息结构体指针
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_get_memory_info(cproc_handle *handle, cproc_mem_working_set *mem_info);

    /**
     * @brief 设置管道为非阻塞模式
     * 
     * @param pipe 管道指针
     * @param non_blocking 是否非阻塞
     * @return 0成功，非0失败
     */
    CC_API int CC_CALL cproc_pipe_set_non_blocking(cproc_pipe *pipe, int non_blocking);

    /**
     * @brief 检查管道是否有数据可读
     * 
     * @param pipe 管道指针
     * @param timeout_ms 超时时间（毫秒），0表示立即返回，-1表示阻塞
     * @return 1有数据，0无数据/超时，-1错误
     */
    CC_API int CC_CALL cproc_pipe_peek(cproc_pipe *pipe, int timeout_ms);

    /** @}*/

#ifdef __cplusplus
}
#endif
#endif

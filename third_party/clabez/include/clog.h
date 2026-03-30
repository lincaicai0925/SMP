#ifndef C_CORE_LOG_H_
#define C_CORE_LOG_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

//=========================================================================
// 版本信息
//=========================================================================

#define CLOG_VERSION_MAJOR 2
#define CLOG_VERSION_MINOR 0
#define CLOG_VERSION_PATCH 0
#define CLOG_VERSION_STRING "2.0.0"

//=========================================================================
// 构建模式配置
//=========================================================================

/**
 * CLOG_BUILD_SHARED: 构建模式
 *   0 = 静态库模式（所有符号内部链接）
 *   1 = 构建共享DLL（导出符号）
 *   2 = 使用共享DLL（导入符号）
 */
#ifndef CLOG_BUILD_SHARED
  #define CLOG_BUILD_SHARED 1  // 默认构建为共享DLL
#endif

//=========================================================================
// API导出宏
//=========================================================================

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

    /** \addtogroup clog 日志库
     * @{
     */
    
    /**
     * @brief 日志级别
     * 
     * 日志级别从低到高依次为：
     * - TRACE: 最低级别，用于打印函数级的输入输出参数，用于详细跟踪
     * - DEBUG: 用于开发过程中打印调试信息
     * - INFO:  用于输出程序运行的重要信息，可用于生产环境
     * - WARN:  用于输出警告信息，表明可能出现潜在错误
     * - ERROR: 用于输出错误信息，但不影响系统继续运行
     * - FATAL: 最高级别，用于输出严重错误，可能导致应用程序退出
     * - OFF:   关闭日志输出
     */
    typedef enum clog_Level
    {
        CLOG_LEVEL_TRACE = 0, /**< 最低级别一般用来打印函数级的输入输出参数 */
        CLOG_LEVEL_DEBUG = 1, /**< 用来输出调试信息 */
        CLOG_LEVEL_INFO = 2,  /**< 用来输出提示信息 */
        CLOG_LEVEL_WARN = 3,  /**< 用来输出警告信息 */
        CLOG_LEVEL_ERROR = 4, /**< 用来输出错误信息 */
        CLOG_LEVEL_FATAL = 5, /**< 用来输出严重的警告信息 */
        CLOG_LEVEL_OFF = 6,   /**< 关闭 */
    } clog_Level;

    /**
     * @brief 日志文本编码格式
     * 
     * 支持三种编码格式：
     * - ANSI: 系统默认的多字节编码（Windows上通常是本地代码页，如GBK）
     * - UTF8: UTF-8多字节编码，跨平台通用
     * - Wide: Unicode宽字符编码（Windows上是UTF-16，Linux上是UTF-32）
     */
    typedef enum clog_Codec
    {
        clog_Codec_ANSI = 0, /**< ANSI多字节编码（系统默认代码页） */
        clog_Codec_UTF8 = 1, /**< UTF-8多字节编码 */
        clog_Codec_Wide = 2  /**< Unicode宽字符编码 */
    } clog_Codec;

    /**
     * @brief 文件滚动策略
     */
    typedef enum clog_RotatePolicy
    {
        CLOG_ROTATE_NONE = 0,     /**< 不滚动，单一文件 */
        CLOG_ROTATE_SIZE = 1,     /**< 按文件大小滚动 */
        CLOG_ROTATE_DAILY = 2,    /**< 按天滚动 */
        CLOG_ROTATE_HOURLY = 3,   /**< 按小时滚动 */
    } clog_RotatePolicy;

    /**
     * @brief 文件覆写策略
     */
    typedef enum clog_OverwritePolicy
    {
        CLOG_OVERWRITE_OLDEST = 0,              /**< 覆盖最旧的文件（按文件数量限制） */
        CLOG_OVERWRITE_DISABLE = 1,             /**< 禁止覆写，达到限制后停止写入 */
        CLOG_OVERWRITE_BY_FOLDER_SIZE = 2,      /**< 按文件夹总大小覆盖 */
        CLOG_OVERWRITE_BY_FOLDER_PERCENT = 3,   /**< 按文件夹占磁盘百分比覆盖 */
        CLOG_OVERWRITE_BY_DISK_FREE_SIZE = 4,   /**< 按磁盘剩余空间覆盖 */
        CLOG_OVERWRITE_BY_DISK_FREE_PERCENT = 5,/**< 按磁盘剩余百分比覆盖 */
    } clog_OverwritePolicy;

    /**
     * @brief 控制台输出目标
     */
    typedef enum clog_ConsoleTarget
    {
        CLOG_CONSOLE_STDOUT = 0, /**< 标准输出 */
        CLOG_CONSOLE_STDERR = 1, /**< 标准错误输出 */
    } clog_ConsoleTarget;

    /**
     * @brief 日志槽类型
     */
    typedef enum clog_SinkType
    {
        CLOG_SINK_CONSOLE = 0,       /**< 控制台槽 */
        CLOG_SINK_FILE = 1,          /**< 文件槽 */
        CLOG_SINK_DEBUGSTRING = 2,   /**< OutputDebugString槽(仅Windows) */
        CLOG_SINK_SYSLOG = 3,        /**< syslog槽(仅Linux/Unix) */
        CLOG_SINK_EVENTLOG = 4,      /**< Windows事件日志槽(仅Windows) */
        CLOG_SINK_CUSTOM = 5,        /**< 自定义槽 */
    } clog_SinkType;

    /**
     * @brief 日志槽句柄
     */
    typedef void* clog_SinkHandle;

    /**
     * @brief 自定义日志输出回调函数
     * 
     * @param logger logger名称（UTF-8编码）
     * @param level 日志级别
     * @param thread_id 线程ID
     * @param codec 文本编码格式
     * @param text 日志文本内容
     * @param text_len 文本长度（字节数）
     * @param userdata 用户自定义数据
     */
    typedef void(CC_CALL *clog_sink_fun)(const char *logger, clog_Level level,uint64_t thread_id, clog_Codec codec, const void *text, int text_len, void *userdata);

    /**
     * @brief 日志输出函数原型
     * 
     * 类似printf的可变参数函数
     * @param fmt 格式化字符串
     * @param ... 可变参数
     * @return 返回输出的字符数，失败返回负数
     */
    typedef int (CC_CALL *clog_Fun)(const void *fmt, ...);

    //=========================================================================
    // 版本检查API
    //=========================================================================

    /**
     * @brief 获取日志库版本号
     * 
     * @return 版本号，格式：major * 10000 + minor * 100 + patch
     *         例如：v2.0.0 返回 20000
     */
    CC_API int CC_CALL clog_get_version(void);

    /**
     * @brief 获取版本字符串
     * 
     * @return 版本字符串，例如 "2.0.0"
     */
    CC_API const char* CC_CALL clog_get_version_string(void);

    /**
     * @brief 检查版本兼容性
     * 
     * 检查运行时版本是否与编译时版本兼容。
     * 主版本号必须相同，次版本号必须大于等于要求的版本。
     * 
     * @param required_major 要求的主版本号
     * @param required_minor 要求的次版本号
     * @return 1表示兼容，0表示不兼容
     */
    CC_API int CC_CALL clog_check_version(int required_major, int required_minor);

    //=========================================================================
    // 初始化和清理API
    //=========================================================================

    /**
     * @brief 从配置字符串初始化日志系统
     * 
     * 配置字符串使用JSON格式，支持配置全局级别、logger级别、槽配置等
     * 
     * @param str_utf8 UTF-8编码的配置字符串
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_init_from_config_string(const char *str_utf8);

    /**
     * @brief 从配置文件初始化日志系统
     * 
     * 配置文件使用JSON格式
     * 
     * @param filepath_utf8 UTF-8编码的配置文件路径
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_init_from_cfg_file(const char *filepath_utf8);

    /**
     * @brief 初始化一个logger
     * 
     * 如果logger已存在则返回其ID，否则创建新的logger
     * 
     * @param logger logger名称（UTF-8编码）
     * @return 返回logger的ID，失败返回-1
     */
    CC_API int CC_CALL clog_init_logger(const char *logger);

    /**
     * @brief 清理日志系统，释放所有资源
     * 
     * 关闭所有文件，释放所有内存，应在程序退出前调用
     */
    CC_API void CC_CALL clog_clean_up();

    //=========================================================================
    // 日志级别设置API
    //=========================================================================

    /**
     * @brief 设置全局日志输出级别
     * 
     * 全局级别会影响所有未单独设置级别的logger
     * 
     * @param level 日志级别
     * @return 返回之前的日志级别
     */
    CC_API int CC_CALL clog_set_level(clog_Level level);

    /**
     * @brief 获取全局日志输出级别
     * 
     * @return 返回当前全局日志级别
     */
    CC_API clog_Level CC_CALL clog_get_level();

    /**
     * @brief 设置指定logger的日志输出级别
     * 
     * @param logger logger名称（UTF-8编码）
     * @param level 日志级别
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_set_logger_level(const char *logger, clog_Level level);

    /**
     * @brief 让指定logger使用全局日志级别
     * 
     * @param logger logger名称（UTF-8编码）
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_set_logger_use_global_level(const char *logger);

    /**
     * @brief 获取指定logger的日志输出级别
     * 
     * @param logger logger名称（UTF-8编码）
     * @return 返回logger的日志级别
     */
    CC_API clog_Level CC_CALL clog_get_logger_level(const char *logger);


    /**
     * @brief 获取所有已注册的logger数量
     * 
     * @return logger数量（不包括索引0的占位符）
     */
    CC_API int CC_CALL clog_get_logger_count(void);

    /**
     * @brief 获取指定索引的logger信息
     * 
     * @param index logger索引（从1开始）
     * @param name_buffer 用于接收logger名称的缓冲区
     * @param buffer_size 缓冲区大小
     * @param out_level 输出logger的当前级别
     * @param out_use_global 输出是否使用全局级别（1表示使用全局级别）
     * @return 0表示成功，-1表示失败（索引无效）
     */
    CC_API int CC_CALL clog_get_logger_info(int index, char *name_buffer, int buffer_size, 
                                              clog_Level *out_level, int *out_use_global);

    /**
     * @brief 判断某个logger的某个级别的日志是否需要输出
     * 
     * @param level 日志级别
     * @param logger logger ID
     * @return 1表示需要输出，0表示不需要输出
     */
    CC_API int CC_CALL clog_is_out(clog_Level level, int logger);

    /**
     * @brief 检查指定级别的日志是否需要输出（带初始化，线程安全）
     * 
     * 此函数会自动完成以下操作：
     * 1. 使用原子操作读取 logger 变量
     * 2. 如果未初始化（< 0），则进行初始化
     * 3. 判断指定级别的日志是否需要输出
     * 
     * 线程安全特性：
     * - 使用原子操作读取 logger 指针指向的值
     * - 多线程并发调用时，确保只初始化一次
     * - 初始化完成后，后续调用性能开销很小
     * 
     * @param level 日志级别
     * @param logger_ptr logger 变量的指针（volatile int*）
     * @param logger_name logger 名称（用于初始化）
     * @return 1表示需要输出，0表示不需要输出
     */
    CC_API int CC_CALL clog_is_out_init(clog_Level level, volatile int* logger_ptr, const char* logger_name);

    /**
     * @brief 获取日志函数（带初始化，线程安全）
     * 
     * 此函数整合了初始化检查、级别判断和日志函数获取的逻辑：
     * 1. 使用原子操作读取 logger 变量
     * 2. 如果未初始化，则进行初始化
     * 3. 判断指定级别的日志是否需要输出
     * 4. 如果需要输出，返回日志函数；否则返回空操作函数
     * 
     * @param level 日志级别
     * @param logger_ptr logger 变量的指针（volatile int*）
     * @param logger_name logger 名称（用于初始化）
     * @param codec 编码格式
     * @return 返回日志函数指针
     */
    CC_API clog_Fun CC_CALL clog_get_log_fun_init(clog_Level level, volatile int* logger_ptr, const char* logger_name, clog_Codec codec);

    /**
     * @brief 检查是否需要输出日志并获取logger ID（带初始化，线程安全）
     * 
     * 此函数用于性能优化场景，避免不必要的参数求值：
     * 1. 使用原子操作读取 logger 变量
     * 2. 如果未初始化，则进行初始化
     * 3. 判断指定级别的日志是否需要输出
     * 4. 如果需要输出，返回logger ID；否则返回-1
     * 
     * 配合宏使用，可以实现：只有需要输出时，才求值日志参数
     * 
     * @param level 日志级别
     * @param logger_ptr logger 变量的指针（volatile int*）
     * @param logger_name logger 名称（用于初始化）
     * @return 如果需要输出返回logger ID（>=0），否则返回-1
     */
    CC_API int CC_CALL clog_should_log_init(clog_Level level, volatile int* logger_ptr, const char* logger_name);

    //=========================================================================
    // 日志槽管理API
    //=========================================================================

    /**
     * @brief 添加控制台日志槽
     * 
     * @param target 输出目标（stdout或stderr）
     * @param codec 输出编码格式
     * @param enable_color 是否启用彩色输出（1启用，0禁用）
     * @return 返回槽句柄，失败返回NULL
     */
    CC_API clog_SinkHandle CC_CALL clog_add_console_sink(clog_ConsoleTarget target, clog_Codec codec, int enable_color);

    /**
     * @brief 添加文件日志槽
     * 
     * @param filepath_utf8 日志文件路径（UTF-8编码）
     * @param codec 文件编码格式
     * @param rotate_policy 滚动策略
     * @param max_file_size 单个文件最大大小（字节），仅当rotate_policy为CLOG_ROTATE_SIZE时有效
     * @param max_files 最大文件数量，0表示不限制
     * @param overwrite_policy 覆写策略
     * @return 返回槽句柄，失败返回NULL
     */
    CC_API clog_SinkHandle CC_CALL clog_add_file_sink(
        const char *filepath_utf8,
        clog_Codec codec,
        clog_RotatePolicy rotate_policy,
        size_t max_file_size,
        int max_files,
        clog_OverwritePolicy overwrite_policy);

    /**
     * @brief 设置文件槽的空间管理参数
     * 
     * 根据不同的覆写策略，设置相应的阈值参数：
     * - CLOG_OVERWRITE_BY_FOLDER_SIZE: max_folder_size_bytes 生效
     * - CLOG_OVERWRITE_BY_FOLDER_PERCENT: max_folder_percent 生效（0-100）
     * - CLOG_OVERWRITE_BY_DISK_FREE_SIZE: min_disk_free_bytes 生效
     * - CLOG_OVERWRITE_BY_DISK_FREE_PERCENT: min_disk_free_percent 生效（0-100）
     * 
     * @param sink_handle 文件槽句柄
     * @param max_folder_size_bytes 文件夹最大占用空间（字节），0表示不限制
     * @param max_folder_percent 文件夹最大占磁盘百分比（0-100），0表示不限制
     * @param min_disk_free_bytes 磁盘最小剩余空间（字节），0表示不限制
     * @param min_disk_free_percent 磁盘最小剩余百分比（0-100），0表示不限制
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_set_file_sink_space_policy(
        clog_SinkHandle sink_handle,
        size_t max_folder_size_bytes,
        int max_folder_percent,
        size_t min_disk_free_bytes,
        int min_disk_free_percent);

    /**
     * @brief 设置文件槽空间检查间隔
     * 
     * 为了性能优化，不是每次写入都检查磁盘空间。
     * 可以设置按时间间隔或写入次数间隔检查。
     * 
     * @param sink_handle 文件槽句柄
     * @param check_interval_seconds 检查时间间隔（秒），0表示不按时间检查
     * @param check_interval_writes 检查写入次数间隔，0表示不按写入次数检查
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_set_file_sink_space_check_interval(
        clog_SinkHandle sink_handle,
        int check_interval_seconds,
        int check_interval_writes);

#if defined(_WIN32) || defined(_WIN64)
    /**
     * @brief 添加OutputDebugString日志槽（仅Windows）
     * 
     * 输出到Visual Studio调试器的输出窗口
     * 
     * @param codec 输出编码格式
     * @return 返回槽句柄，失败返回NULL
     */
    CC_API clog_SinkHandle CC_CALL clog_add_debugstring_sink(clog_Codec codec);

    /**
     * @brief 添加Windows事件日志槽（仅Windows）
     * 
     * 输出到Windows事件查看器
     * 
     * @param source_name_utf8 事件源名称（UTF-8编码），如应用程序名
     * @return 返回槽句柄，失败返回NULL
     */
    CC_API clog_SinkHandle CC_CALL clog_add_eventlog_sink(const char *source_name_utf8);
#endif

#if defined(__unix) || defined(__linux)
    /**
     * @brief 添加syslog日志槽（仅Linux/Unix）
     * 
     * @param ident 标识字符串，通常是程序名（UTF-8编码）
     * @param facility syslog facility，如LOG_USER、LOG_LOCAL0等
     * @return 返回槽句柄，失败返回NULL
     */
    CC_API clog_SinkHandle CC_CALL clog_add_syslog_sink(const char *ident, int facility);
#endif

    /**
     * @brief 添加自定义日志槽
     * 
     * 用户可以通过回调函数实现自定义的日志输出
     * 
     * @param sink 日志输出回调函数
     * @param userdata 用户自定义数据，会传递给回调函数
     * @param codec 期望的编码格式（日志系统会自动转换）
     * @return 返回槽句柄，失败返回NULL
     */
    CC_API clog_SinkHandle CC_CALL clog_add_custom_sink(clog_sink_fun sink, void *userdata, clog_Codec codec);

    /**
     * @brief 删除日志槽
     * 
     * @param sink_handle 槽句柄
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_remove_sink(clog_SinkHandle sink_handle);

    /**
     * @brief 刷新所有日志槽
     * 
     * 强制将缓冲区中的日志写入目标（文件、控制台等）
     */
    CC_API void CC_CALL clog_flush_all();

    /**
     * @brief 刷新指定日志槽
     * 
     * @param sink_handle 槽句柄
     */
    CC_API void CC_CALL clog_flush_sink(clog_SinkHandle sink_handle);

    //=========================================================================
    // 日志槽过滤器API
    //=========================================================================

    /**
     * @brief 设置日志槽的最小级别过滤器
     * 
     * 只有级别大于等于min_level的日志才会输出到该槽
     * 
     * @param sink_handle 槽句柄
     * @param min_level 最小日志级别
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_set_sink_level_filter(clog_SinkHandle sink_handle, clog_Level min_level);

    /**
     * @brief 设置日志槽的logger过滤器
     * 
     * 只有在白名单中的logger才会输出到该槽，NULL表示接受所有logger
     * 
     * @param sink_handle 槽句柄
     * @param logger_names UTF-8编码的logger名称数组
     * @param count 数组长度
     * @return 0表示成功，非0表示失败
     */
    CC_API int CC_CALL clog_set_sink_logger_filter(clog_SinkHandle sink_handle, const char **logger_names, int count);

    //=========================================================================
    // 辅助函数API
    //=========================================================================

    /**
     * @brief 获取日志函数指针
     * 
     * 内部使用，由宏调用
     * 
     * @param level 日志级别
     * @param logger logger ID
     * @param codec 编码格式
     * @return 返回日志输出函数指针
     */
    CC_API clog_Fun CC_CALL clog_get_log_fun(clog_Level level, int logger, clog_Codec codec);

    /**
     * @brief 获取空操作日志函数（用于宏优化）
     * 
     * 当日志不需要输出时，返回此函数以避免实际的日志处理。
     * 此函数用于解决宏定义中的 dangling else 问题。
     * 
     * @return 返回空操作日志函数指针
     */
    CC_API clog_Fun CC_CALL clog_get_noop_fun(void);

    /**
     * @brief 判断当前是进入函数还是退出函数
     * 
     * 用于函数追踪，内部使用
     * 
     * @param src_file 源文件名（__FILE__）
     * @param func 函数名（__func__）
     * @param line 行号（__LINE__）
     * @return 0表示进入函数，1表示退出函数
     */
    CC_API int CC_CALL clog_check_func_step(const char *src_file, const char *func, int line);



    //=========================================================================
    // 日志宏
    //=========================================================================

    /**
     * @brief 内部宏，用于获取日志函数并判断是否需要输出（线程安全、零开销版本）
     * 
     * 使用 for 循环技巧实现，既解决 dangling else 问题，又避免不必要的参数求值。
     * 
     * 性能优化特性（零开销抽象）：
     * - 当日志不需要输出时，后续的格式化参数**完全不会被求值**
     * - 避免了复杂参数计算的性能开销（如函数调用、表达式计算等）
     * - 例如：clog_info(LOGGER)("result: %d", expensive_func()) 
     *   当日志级别不满足时，expensive_func() 不会被调用
     * 
     * 线程安全特性：
     * - clog_should_log_init 内部使用原子操作读取和初始化 logger
     * - 初始化逻辑集中在该函数中，确保线程安全
     * - 多线程并发时，每个线程都能获得正确且一致的 logger ID
     * 
     * 实现原理：
     * - 使用 for 循环的初始化和条件判断部分
     * - 第一次迭代：检查是否需要输出（调用 clog_should_log_init）
     * - 如果需要输出（返回值>=0），执行循环体（调用格式化函数）
     * - 如果不需要输出（返回值<0），跳过循环体（参数不会被求值）
     * - 迭代表达式将计数器置为-1，确保只执行一次
     * 
     * 可以安全地在 if-else 语句中使用：
     * @code
     * if (condition)
     *     clog_info(LOGGER)("message 1");
     * else
     *     clog_info(LOGGER)("message 2");
     * @endcode
     */
#define clog(level, logger, codec) \
    for (int _clog_id = clog_should_log_init(level, &clog_##logger, #logger); \
         _clog_id >= 0; \
         _clog_id = -1) \
        clog_get_log_fun(level, _clog_id, codec)

    //=========================================================================
    // ANSI编码日志宏
    //=========================================================================

    /**
     * @brief 输出TRACE级别日志（ANSI编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_trace(logger) clog(CLOG_LEVEL_TRACE, logger, clog_Codec_ANSI)

    /**
     * @brief 输出DEBUG级别日志（ANSI编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_debug(logger) clog(CLOG_LEVEL_DEBUG, logger, clog_Codec_ANSI)

    /**
     * @brief 输出INFO级别日志（ANSI编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_info(logger) clog(CLOG_LEVEL_INFO, logger, clog_Codec_ANSI)

    /**
     * @brief 输出WARN级别日志（ANSI编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_warn(logger) clog(CLOG_LEVEL_WARN, logger, clog_Codec_ANSI)

    /**
     * @brief 输出ERROR级别日志（ANSI编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_error(logger) clog(CLOG_LEVEL_ERROR, logger, clog_Codec_ANSI)

    /**
     * @brief 输出FATAL级别日志（ANSI编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_fatal(logger) clog(CLOG_LEVEL_FATAL, logger, clog_Codec_ANSI)

    //=========================================================================
    // Wide字符日志宏
    //=========================================================================

    /**
     * @brief 输出TRACE级别日志（Wide字符编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_trace_w(logger) clog(CLOG_LEVEL_TRACE, logger, clog_Codec_Wide)

    /**
     * @brief 输出DEBUG级别日志（Wide字符编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_debug_w(logger) clog(CLOG_LEVEL_DEBUG, logger, clog_Codec_Wide)

    /**
     * @brief 输出INFO级别日志（Wide字符编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_info_w(logger) clog(CLOG_LEVEL_INFO, logger, clog_Codec_Wide)

    /**
     * @brief 输出WARN级别日志（Wide字符编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_warn_w(logger) clog(CLOG_LEVEL_WARN, logger, clog_Codec_Wide)

    /**
     * @brief 输出ERROR级别日志（Wide字符编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_error_w(logger) clog(CLOG_LEVEL_ERROR, logger, clog_Codec_Wide)

    /**
     * @brief 输出FATAL级别日志（Wide字符编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_fatal_w(logger) clog(CLOG_LEVEL_FATAL, logger, clog_Codec_Wide)

    //=========================================================================
    // UTF-8编码日志宏
    //=========================================================================

    /**
     * @brief 输出TRACE级别日志（UTF-8编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_trace_u8(logger) clog(CLOG_LEVEL_TRACE, logger, clog_Codec_UTF8)

    /**
     * @brief 输出DEBUG级别日志（UTF-8编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_debug_u8(logger) clog(CLOG_LEVEL_DEBUG, logger, clog_Codec_UTF8)

    /**
     * @brief 输出INFO级别日志（UTF-8编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_info_u8(logger) clog(CLOG_LEVEL_INFO, logger, clog_Codec_UTF8)

    /**
     * @brief 输出WARN级别日志（UTF-8编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_warn_u8(logger) clog(CLOG_LEVEL_WARN, logger, clog_Codec_UTF8)

    /**
     * @brief 输出ERROR级别日志（UTF-8编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_error_u8(logger) clog(CLOG_LEVEL_ERROR, logger, clog_Codec_UTF8)

    /**
     * @brief 输出FATAL级别日志（UTF-8编码）
     * @param logger logger名称（由clog_def_logger定义）
     */
#define clog_fatal_u8(logger) clog(CLOG_LEVEL_FATAL, logger, clog_Codec_UTF8)

    //=========================================================================
    // Logger定义宏
    //=========================================================================

    /**
     * @brief 定义一个Logger（线程安全版本）
     * 
     * 该宏会生成：
     * 1. 一个静态的 volatile int 变量 clog_##logger，初始化为 -1
     * 
     * 使用原子操作实现完美的线程安全初始化：
     * - 多个线程并发首次调用时，每个线程获得相同的logger ID
     * - 初始化逻辑集中在 clog_is_out_init 函数中
     * - 使用原子比较交换避免竞态条件
     * 
     * 使用示例：
     * @code
     * clog_def_logger(MAIN);  // 定义名为MAIN的logger
     * 
     * int main() {
     *     clog_info(MAIN)("Application started");
     *     clog_debug(MAIN)("Debug message: %d", 42);
     *     return 0;
     * }
     * @endcode
     * 
     * 注意事项：
     * - logger 变量会在首次使用 clog 宏时自动初始化
     * - 初始化是线程安全的，多个线程可以同时调用
     * - 可以安全地在 if-else 语句中使用 clog 宏
     * 
     * @param logger logger名称（标识符）
     */
#define clog_def_logger(logger) \
    static volatile int clog_##logger = -1

    /**
     * @brief 预定义的ROOT logger
     * 
     * 可以直接使用，无需定义
     */
    clog_def_logger(ROOT);

    //=========================================================================
    // 函数追踪宏
    //=========================================================================

    /**
     * @brief 线程安全的函数入口出口日志工具
     * 
     * 在函数开始和结束时自动输出日志，用于追踪函数调用
     * 
     * @param logger logger名称
     * @param fun 函数名
     */
#define clog_func_trace(logger, fun) clog(CLOG_LEVEL_TRACE, logger, clog_Codec_ANSI)("%c%s,%s[%d]\n", clog_check_func_step(__FILE__, fun, __LINE__) == 0 ? '>' : '<', fun, __FILE__, __LINE__)

    /** @}*/

#ifdef __cplusplus
}
#endif

#endif /* C_CORE_LOG_H_ */

#ifndef CPOPT_H
#define CPOPT_H
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
    /** \addtogroup LabEZ_GetOpt 命令行参数
     * @{
     */

    /**
     * \struct cpopt 命令行参数
     *
     */
    typedef struct cpopt cpopt;

    /**
     * \enum cpopt_value_t 命令行参数的值类型
     *
     */
    typedef enum cpopt_value_t
    {
        CPARG_NONE,  /*!< 不需要填值 */
        CPARG_BOOL,  /*!< bool类型 */
        CPARG_INT,   /*!< 整型 */
        CPARG_FLOAT, /*!< 浮点 */
        CPARG_STR,   /*!< 字符串 */
    } cpopt_value_t;

    /**
     * @brief 创建cpopt
     *
     * @param caption 标题
     * @return 命令行参数指针
     */
    CC_API cpopt *CC_CALL cpopt_create(const char *caption,const char *tail);

    /**
     * @brief 添加命令行参数
     *
     * @param self 创建cpopt指针
     * @param alias 别名
     * @param full_name 全名
     * @param type 类型 当类型为CPARG_NONE时，忽略后面的default_value,required_option,required_arg
     * @param default_value 默认值
     * @param detail 描述
     * @param required_option 是否是必选项
     * @param required_arg 是否需要参数
     * @param allow_multiple 是否允许多项（数组）
     * @return bool 非0为成功，0为失败
     */
    CC_API int CC_CALL cpopt_add_option(
        cpopt *self,
        char alias,
        const char *full_name,
        cpopt_value_t type,
        const char *default_value,
        const char *detail,
        int required_option,
        int required_arg,
        int allow_multiple);

    /**
     * @brief 向控制台打印帮助
     *
     * @param self cpopt指针
     * @return 无
     */
    CC_API void CC_CALL cpopt_print_help_msg(cpopt *self);

    /**
     * @brief 打印到字符串
     *
     * @param self cpopt指针
     * @return 字符串
     */
    CC_API const char *CC_CALL cpopt_get_help_msg(cpopt *self);

    /**
     * @brief 解析命令行
     *
     * @param self cpopt指针
     * @param argc main函数的argc
     * @param argv main函数的argv
     * @return bool
     */
    CC_API int CC_CALL cpopt_parse_cmd(cpopt *self, int argc, char **argv);

    /**
     * @brief 解析命令行字符串
     * 
     * @param self cpopt指针
     * @param cmd 命令行字符串
     * @return 无
     */
    CC_API int CC_CALL cpopt_parse_cmd_str(cpopt *self,const char *cmd);

    /**
     * @brief 打印解析失败的消息
     * 
     * @param self cpopt指针
     * @return 无 
     */
    CC_API void CC_CALL cpopt_print_parse_failed(cpopt *self);

    /**
     * @brief 获取解析失败的消息
     * 
     * @param self cpopt指针
     * @return 字符串 
     */
    CC_API const char * CC_CALL cpopt_get_parse_failed_str(cpopt *self);

    /**
     * @brief 获取非标准项的数量
     *
     * @param self cpopt指针
     * @return 数量
     */
    CC_API int CC_CALL cpopt_get_unnormalized_option_count(cpopt *self);

    /**
     * @brief 获取非标准项
     *
     * @param self cpopt指针
     * @param index 索引
     * @return 字符串
     */
    CC_API const char * CC_CALL cpopt_get_unnormalized_option_as_str(cpopt *self, int index);

    /**
     * @brief 获取命令中某个选项的个数
     * 
     * @param self cpopt指针
     * @param option 选项，可以是别名，也可以是全名
     * @return CC_API 
     */
    CC_API int CC_CALL cpopt_get_option_count(cpopt *self, const char *option);

    /**
     * @brief 获取命令中某个选项的第几个参数，返回bool
     * 如果是单选项时index只能为0
     * @param self cpopt指针
     * @param option 选项名，可以是别名，也可以是全名
     * @param index 索引
     * @return bool 0或1 
     */
    CC_API int CC_CALL cpopt_get_option_as_bool(cpopt *self, const char *option, int index);

    /**
     * @brief 获取命令中某个选项的第几个参数，返回整数
     * 
     * @param self cpopt指针
     * @param option 选项名，可以是别名，也可以是全名
     * @param index 索引
     * @return 整数 
     */
    CC_API int CC_CALL cpopt_get_option_as_int(cpopt *self, const char *option, int index);

    /**
     * @brief 获取命令中某个选项的第几个参数，返回浮点数
     * 
     * @param self cpopt指针
     * @param option 选项名，可以是别名，也可以是全名
     * @param index 索引
     * @return 浮点数 
     */
    CC_API float CC_CALL cpopt_get_option_as_float(cpopt *self, const char *option, int index);

    /**
     * @brief 获取命令中某个选项的第几个参数，返回字符串
     * 
     * @param self cpopt指针
     * @param option 选项名，可以是别名，也可以是全名
     * @param index 索引
     * @return 字符串 
     */
    CC_API const char *CC_CALL cpopt_get_option_as_str(cpopt *self, const char *option, int index);

    /**
     * @brief 获取命令中某个选项的第几个参数，返回字符串
     * 
     * @param option 选项名，可以是别名，也可以是全名
     * @param begin 索引
     * @param size 索引
     * @return 字符串 
     */
    CC_API int CC_CALL cpopt_find_valid_option(const char *option, int *begin, int *size);

    /**
     * @brief 启用Unix风格组合短选项支持（如 -abc 等价于 -a -b -c）
     * 
     * @param self cpopt指针
     * @param enable 是否启用（非0启用，0禁用）
     * @return 无
     */
    CC_API void CC_CALL cpopt_enable_combined_short_options(cpopt *self, int enable);

    /**
     * @brief 销毁cpopt指针
     *
     * @param opts cpopt指针的指针
     * @return 无
     */
    CC_API void CC_CALL cpopt_destroy(cpopt **pself);

    /** @}*/



#ifdef __cplusplus
}
#endif /* end of __cplusplus */
#endif

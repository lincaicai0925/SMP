
#ifndef C_CORE_INI_H_
#define C_CORE_INI_H_

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
    /** \addtogroup cini ini文件读写模块
     * @{
     */

    typedef struct cini cini;

    /**
     * @brief 创建ini对象
     *
     * @return ini对象
     */
    CC_API cini *CC_CALL cini_create();

    /**
     * @brief 销毁ini对象，此接口幂等
     * @param pini ini对象指针的指针
     * @return 无
     */
    CC_API void CC_CALL cini_destroy(cini **pini);

    /**
     * @brief 加载ini文件
     * @param ini ini对象指针
     * @param filename ini文件路径
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_load(cini *ini, const char *filename);

    /**
     * @brief 保存ini文件
     * @param ini ini对象指针
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_save(cini *ini);

    /**
     * @brief 保存ini文件到指定文件
     * @param ini ini对象指针
     * @param filename ini文件路径
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_save_as(cini *ini, const char *filename);

    /**
     * @brief 获取字符串
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param def 默认值
     * @return 字符串
     */
    CC_API const char *CC_CALL cini_get_string(cini *ini, const char *section, const char *key, const char *def);

    /**
     * @brief 获取整数
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param def 默认值
     * @return 整数
     */
    CC_API int CC_CALL cini_get_int(cini *ini, const char *section, const char *key, int def);

    /**
     * @brief 获取浮点数
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param def 默认值
     * @return 浮点数
     */
    CC_API double CC_CALL cini_get_double(cini *ini, const char *section, const char *key, double def);

    /**
     * @brief 获取布尔值
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param def 默认值
     * @return 布尔值
     */
    CC_API int CC_CALL cini_get_bool(cini *ini, const char *section, const char *key, int def);

    /**
     * @brief 设置字符串
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param value 字符串
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_set_string(cini *ini, const char *section, const char *key, const char *value);

    /**
     * @brief 设置整数
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param value 整数
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_set_int(cini *ini, const char *section, const char *key, int value);

    /**
     * @brief 设置浮点数
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param value 浮点数
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_set_double(cini *ini, const char *section, const char *key, double value);

    /**
     * @brief 设置布尔值
     * @param ini ini对象指针
     * @param section 节名称
     * @param key 键名称
     * @param value 布尔值
     * @return 成功返回0，失败返回-1
     */
    CC_API int CC_CALL cini_set_bool(cini *ini, const char *section, const char *key, int value);

    /** @}*/
#ifdef __cplusplus
}
#endif

#endif

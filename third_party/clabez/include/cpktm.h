#ifndef C_CORE_PKTM_H
#define C_CORE_PKTM_H
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
    /** \addtogroup cpktm 私有协议解包器
     * 可以处理各种私有协议，能处理数据丢包，错位，包粘连等情况
     * @{
     */

    /**
     * \struct cpktm 包匹配器
     *
     */
    typedef struct cpktm cpktm;

    /**
     * @brief 包长度回调
     * 
     * @param self 包匹配器
     * @param header_buffer 包头数据缓冲
     * @param header_size 包头数据长度
     * @param userdata 用户自定义数据
     * @return 返回长度
     * 
     */
    typedef int(CC_CALL *cpktm_size_getter_cb)(cpktm *self, unsigned char *header_buffer, int header_size, void *userdata);
    
    /**
     * @brief 包校验回调
     * @param self 包匹配器
     * @param buffer 数据包地址
     * @param size 数据包长度
     * @param userdata 用户自定义数据
     * @return bool
     * 
     */
    typedef int(CC_CALL *cpktm_checker_cb)(cpktm *self, unsigned char *buffer, int size, void *userdata);
    
    /**
     * @brief 包处理器
     * @param buffer 数据包地址
     * @param size 数据包长度
     * @param userdata 用户自定义数据
     * @return bool
     */
    typedef int(CC_CALL *cpktm_output_cb)(cpktm *self, unsigned char *buffer, int size, void *userdata);

    /**
     * @brief 创建包匹配器
     * 
     * @return 返回包匹配器指针，失败返回NULL
     */
    CC_API cpktm *CC_CALL cpktm_create();

    /**
     * @brief 设置包头长度
     * 
     * @param self 包匹配器
     * @param size 长度
     * @return 无
     */
    CC_API void CC_CALL cpktm_set_header_size(cpktm *self, int size);

    /**
     * @brief 设置包长度提取回调
     * 
     * @param self 包匹配器
     * @param pktsizegetter 包长度获取回调
     * @param userdata 包长度获取回调参数
     * @return 无 
     */
    CC_API void CC_CALL cpktm_set_pkt_size_getter(cpktm *self, cpktm_size_getter_cb pktsizegetter, void *userdata);

    /**
     * @brief 设置包检查器
     * 
     * @param self 包匹配器
     * @param pktchecker 包检查器回调
     * @param userdata 自定义参数
     * @return 无
     */
    CC_API void CC_CALL cpktm_set_pkt_checker(cpktm *self, cpktm_checker_cb pktchecker, void *userdata);

    /**
     * @brief 设置包处理器
     * 
     * @param self 包匹配器
     * @param processor 处理器回调
     * @param userdata 自定义参数
     * @return 无
     */
    CC_API void CC_CALL cpktm_set_processor(cpktm *self, cpktm_output_cb processor, void *userdata);

    /**
     * @brief 设置最大包长度
     * 
     * @param self 包匹配器
     * @param max_size 长度
     * @return 无
     */
    CC_API void CC_CALL cpktm_set_pkt_max_size(cpktm *self, int max_size);

    /**
     * @brief 设置最大缓冲区大小
     * 
     * @param self 包匹配器
     * @param max_size 最大缓冲区长度（字节），防止内存无限增长
     * @return 无
     * @note 默认值为512KB，在嵌入式环境中建议设置较小的值
     */
    CC_API void CC_CALL cpktm_set_buffer_max_size(cpktm *self, int max_size);

    /**
     * @brief 设置用户数据
     * 
     * @param self 包匹配器
     * @param userdata 用户自定义数据
     * @return 无
     */
    CC_API void CC_CALL cpktm_set_user_data(cpktm *self, void *userdata);

    /**
     * @brief 获取用户数据
     * 
     * @param self 包匹配器
     * @return 用户数据
     */
    CC_API void *CC_CALL cpktm_get_user_data(cpktm *self);

    /**
     * @brief 向包匹配器中添加数据
     * 
     * @param self 包匹配器
     * @param buffer 数据包
     * @param size 长度
     * @return 成功处理的包数量，<0表示错误
     *         0: 成功但未匹配到完整包
     *         >0: 成功匹配的包数量
     *         -1: 参数错误
     *         -2: 内存不足
     *         -3: 缓冲区已满
     */
    CC_API int CC_CALL cpktm_push(cpktm *self, const unsigned char *buffer, int size);

    /**
     * @brief 清理包匹配器内部缓冲区
     * 
     * @param self 包匹配器
     * @return 无
     */
    CC_API void CC_CALL cpktm_clear(cpktm *self);

    /**
     * @brief 销毁包匹配器
     * 
     * @param pself 包匹配器指针的指针
     * @return 无
     */
    CC_API void CC_CALL cpktm_destroy(cpktm **pself);


    /** @}*/

    


#ifdef __cplusplus
}
#endif
#endif
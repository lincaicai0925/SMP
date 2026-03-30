#ifndef C_CORE_MSGBOX_H_
#define C_CORE_MSGBOX_H_

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
    /** \addtogroup cmbox 增强消息框
     * @{
     */

    /**
     * @brief \enum cmbox_type 消息框类型
     *
     */
    typedef enum cmbox_type
    {
        CMBOX_BTN_OK,          /*!< 有一个确定按钮 */
        CMBOX_BTN_OKCANCEL,    /*!< 有一个确定按钮和一个取消按钮 */
        CMBOX_BTN_RETRYCANCEL, /*!< 有一个重试和一个取消按钮 */
        CMBOX_BTN_YESNO,       /*!< 有一个确定和一个否定按钮 */
        CMBOX_BTN_YESNOCANCEL  /*!< 有一个确定，一个否定和一个取消按钮 */
    } cmbox_type;

    /**
     * @brief \enum cmbox_btn_id 按钮ID
     *
     */
    typedef enum cmbox_btn_id
    {
        CMBOX_ID_OK,     /*!< 确定 */
        CMBOX_ID_CANCEL, /*!< 取消 */
        CMBOX_ID_RETRY,  /*!< 重试 */
        CMBOX_ID_YES,    /*!< 确定 */
        CMBOX_ID_NO      /*!< 否定 */
    } cmbox_btn_id;


    /**
     * @brief \struct cmbox_popup_ctx 弹窗回调上下文
     *
     */
    typedef struct cmbox_popup_ctx
    {
        char *caption;
        char *content;
        int type;
        int default_btn;
        void *parent;
        int timeout;
    } cmbox_popup_ctx;

    /**
     * @brief 消息框弹出回调。在单元测试中使用，用于快速跳过
     *
     */
    typedef cmbox_btn_id(CC_CALL *cmbox_popup_cb)(const cmbox_popup_ctx *ctx, void *user_data);

    /**
     * @brief 弹出消息框
     *
     * @param caption_utf8 标题文本
     * @param content_utf8 内容文本
     * @param type 类型
     * @param default_btn 默认按钮
     * @param time_out 超时时间，单位（毫秒）
     * @param parent 父窗口句柄，可以为空
     * @param cb 回调函数，可以为空
     * @param cb_arg 回调函数的参数，可以为空
     * @return 用户选择的结果ID
     */
    CC_API cmbox_btn_id CC_CALL cmbox_popup(const char *caption_utf8, const char *content_utf8, cmbox_type type, cmbox_btn_id default_btn, int time_out, void *parent);

    /**
     * @brief 设置弹窗回调
     * 应该只在单元测试中使用，模拟用户点击，用于加速测试
     * @param popup_ms 在多少毫秒后回调
     * @param cb 窗口句柄
     * @param user_data 用户数据
     * @return 无
     */
    CC_API void CC_CALL cmbox_set_popup_cb(int popup_ms, cmbox_popup_cb cb, void *user_data);

    /**
     * @brief 重置消息框弹出回调
     *
     * @return 无
     */
    CC_API void CC_CALL cmbox_reset_popup_cb();

    /**
     * @brief 在控制台弹出提示
     *
     * @param caption_utf8 标题
     * @param content_utf8 内容
     * @param type 提示类型
     * @param default_btn 默认按钮
     * @param time_out 超时时间
     * @param parent 父窗口
     * @param cb 回调
     * @param cb_arg 回调参数
     * @return 用户选择的id
     */
    CC_API cmbox_btn_id CC_CALL console_pupup(const char *caption_utf8, const char *content_utf8,
                                              cmbox_type type, cmbox_btn_id default_btn, int time_out, void *parent, cmbox_popup_cb cb, void *cb_arg);

    /**
     * @brief 从控制台读取结果
     *
     * @param timeout 超时间
     * @param disable_echo 是否开启回示
     * @return 结果
     */
    CC_API char *CC_CALL console_read_line(int timeout, int disable_echo);
    /** @}*/
#ifdef __cplusplus
}
#endif

#endif

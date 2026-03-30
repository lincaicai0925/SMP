#ifndef C_CORE_DOCVIEW_H
#define C_CORE_DOCVIEW_H
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
    /** \addtogroup cdcv 文档与视图
     * @{
     */
#ifndef cpoint_
#define cpoint_
    typedef struct cpoint
    {
        int x;
        int y;
    } cpoint;
#endif

#ifndef cpointf_
#define cpointf_
    typedef struct cpointf
    {
        float x;
        float y;
    } cpointf;
#endif

#ifndef csize_
#define csize_
    typedef struct csize
    {
        int width;
        int height;
    } csize;
#endif

#ifndef csizef_
#define csizef_
    typedef struct csizef
    {
        float width;
        float height;
    } csizef;
#endif

#ifndef crect_
#define crect_
    typedef struct crect
    {
        int x;
        int y;
        int width;
        int height;
    } crect;
#endif

#ifndef crectf_
#define crectf_
    typedef struct crectf
    {
        float x;
        float y;
        float width;
        float height;
    } crectf;
#endif
    /**
     * @brief \struct cdcv 文档与视图
     * 
     */
    typedef struct cdcv cdcv;

    /**
     * @brief 创建文档视图
     * 
     * @return 返回文档视图指针
     */
    CC_API cdcv *CC_CALL cdcv_create();

    /**
     * @brief 销毁文档视图
     * 
     * @param pself 文档视图指针的指针
     * @return 无 
     */
    CC_API void CC_CALL cdcv_destroy(cdcv **pself);

    /**
     * @brief 获取文档的边界（二维矩形坐标）
     * 
     * @param self 文档视图指针
     * @return 矩形 
     */
    CC_API crectf CC_CALL cdcv_get_doc_bounds(cdcv *self);

    /**
     * @brief 获取文档的尺寸
     * 
     * @param self 文档视图指针
     * @return 尺寸 
     */
    CC_API csizef CC_CALL cdcv_get_doc_size(cdcv *self);

    /**
     * @brief 获取文档视图的左上角原点
     * 
     * @param self 文档视图指针
     * @return 二维坐标点 
     */
    CC_API cpointf CC_CALL cdcv_get_doc_orgin_point(cdcv *self);

    /**
     * @brief 获取视图区域
     * 
     * @param self 文档视图指针
     * @return 矩形 
     */
    CC_API crectf CC_CALL cdcv_get_view_bounds(cdcv *self);

    /**
     * @brief 获取视图尺寸
     * 
     * @param self 文档视图指针
     * @return 尺寸 
     */
    CC_API csizef CC_CALL cdcv_get_view_size(cdcv *self);

    /**
     * @brief 获取视图左上角原点
     * 
     * @param self 文档视图指针
     * @return 二维坐标点
     */
    CC_API cpointf CC_CALL cdcv_get_view_orgin_point(cdcv *self);

    /**
     * @brief 设置文档左上角原点坐标
     * 
     * @param self 文档视图指针
     * @param pt 坐标点
     * @return 无
     */
    CC_API void CC_CALL cdcv_set_doc_orgin_point(cdcv *self, cpointf pt);

    /**
     * @brief 设置文档尺寸
     * 
     * @param self 文档视图指针
     * @param s 尺寸
     * @return 无 
     */
    CC_API void CC_CALL cdcv_set_doc_size(cdcv *self, csizef s);

    /**
     * @brief 设置文档边界
     * 
     * @param self 文档视图指针
     * @param r 矩形
     * @return 无
     */
    CC_API void CC_CALL cdcv_set_doc_bounds(cdcv *self, crectf r);

    /**
     * @brief 设置视图左上角原点
     * 
     * @param self 文档视图指针
     * @param pt 二维坐标点
     * @return 无
     */
    CC_API void CC_CALL cdcv_set_view_orgin_point(cdcv *self, cpointf pt);

    /**
     * @brief 设置视图尺寸
     * 
     * @param self 文档视图指针
     * @param s 尺寸
     * @return 无 
     */
    CC_API void CC_CALL cdcv_set_view_size(cdcv *self, csizef s);

    /**
     * @brief 调整视图尺寸
     * 
     * @param self 文档视图指针
     * @param s 新的尺寸
     * @param keep_aspect_ratio 是否保持等比例缩放，非0表示保持，0表示不保持
     * @return 无
     */
    CC_API void CC_CALL cdcv_resize_view(cdcv *self, csizef s, int keep_aspect_ratio);

    /**
     * @brief 设置视图区域
     * 
     * @param self 文档视图指针
     * @param r 矩形
     * @return 无 
     */
    CC_API void CC_CALL cdcv_set_view_bounds(cdcv *self, crectf r);

    /**
     * @brief 拖拽视图
     * 
     * @param self 文档视图指针
     * @param offset_view_x 视图x轴位移量
     * @param offset_view_y 视图y轴位移量
     * @return 无
     */
    CC_API void CC_CALL cdcv_drag(cdcv *self, float offset_view_x, float offset_view_y);

    /**
     * @brief 拖拽到某个点
     * 
     * @param self 文档视图指针
     * @param pt 目标二维坐标点
     * @return 无
     */
    CC_API void CC_CALL cdcv_drag_to(cdcv *self, cpointf pt);

    /**
     * @brief 获取文档中心
     * 
     * @param self 文档视图指针
     * @return 二维坐标点 
     */
    CC_API cpointf CC_CALL cdcv_get_doc_center(cdcv *self);

    /**
     * @brief 获取视图中心
     * 
     * @param self 文档视图指针
     * @return 二维坐标点 
     */
    CC_API cpointf CC_CALL cdcv_get_view_center(cdcv *self);

    /**
     * @brief 缩放视图
     * 
     * @param self 文档视图指针
     * @param factor 缩放比例因子
     * @param pt 缩放中心点
     * @return 无
     */
    CC_API void CC_CALL cdcv_Scale(cdcv *self, float factor, cpointf pt);

    /**
     * @brief 文档坐标转视图坐标（点）
     * 
     * @param self 文档视图指针
     * @param pt 文档坐标点
     * @return 转换后的视图坐标点
     */
    CC_API cpointf CC_CALL cdcv_doc_to_view_point_f(cdcv *self, cpointf pt);

    /**
     * @brief 文档坐标转视图坐标（矩形）
     * 
     * @param self 文档视图指针
     * @param r 文档坐标矩形
     * @return 转换后的视图坐标矩形
     */
    CC_API crectf CC_CALL cdcv_doc_to_view_rect(cdcv *self, crectf r);

    /**
     * @brief 视图坐标转文档坐标（点）
     * 
     * @param self 文档视图指针
     * @param pt 视图坐标点
     * @return 转换后的文档坐标点
     */
    CC_API cpointf CC_CALL cdcv_view_to_doc_point_f(cdcv *self, cpointf pt);

    /**
     * @brief 视图坐标转文档坐标（矩形）
     * 
     * @param self 文档视图指针
     * @param r 视图坐标矩形
     * @return 转换后的文档坐标矩形
     */
    CC_API crectf CC_CALL cdcv_view_to_doc_rect_f(cdcv *self, crectf r);

    /**
     * @brief 获取缩放比例因子
     * 
     * @param self 文档视图指针
     * @return 缩放比例因子
     */
    CC_API float CC_CALL cdcv_get_scale_factor(cdcv *self);

    /**
     * @brief 获取X轴（宽度）缩放系数
     * 
     * @param self 文档视图指针
     * @return X轴缩放系数
     */
    CC_API float CC_CALL cdcv_get_scale_x(cdcv *self);

    /**
     * @brief 获取Y轴（高度）缩放系数
     * 
     * @param self 文档视图指针
     * @return Y轴缩放系数
     */
    CC_API float CC_CALL cdcv_get_scale_y(cdcv *self);

    /**
     * @brief 判断是否为空（即没有做任何操作）
     * 
     * @param self 文档视图指针
     * @return 非0表示为空，0表示非空
     */
    CC_API int CC_CALL cdcv_is_empty(cdcv *self);

    /**
     * @brief 计算缩略图区域
     * 
     * @param viewsize 视图尺寸
     * @param docsize 文档尺寸
     * @return 矩形区域
     */
    CC_API crectf CC_CALL cdcv_calc_thumbnail_rect(csizef viewsize, csizef docsize);

    /**
     * @brief 计算包围盒
     * 
     * @param viewsize 视图尺寸
     * @param docsize 文档尺寸
     * @return 矩形区域 
     */
    CC_API crectf CC_CALL cdcv_calc_outer_bounding_box(csizef viewsize, csizef docsize);

    /** @}*/


   

#ifdef __cplusplus
}
#endif /* end of __cplusplus */
#endif
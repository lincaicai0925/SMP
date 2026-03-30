/**
 * @file c2d.h
 * @brief C99实现的2D几何算法库，参考Boost.Geometry
 * @details 提供完整的2D几何计算功能，包括点、线、多边形等几何对象的
 *          距离、面积、周长、相交、包含等算法，适用于嵌入式和单片机环境
 * @note 所有算法均采用高效实现，内存占用最小化
 */

#ifndef C_C2D_H
#define C_C2D_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================
 * 基础数据类型定义
 * ======================================================================== */

#ifndef cpoint_
#define cpoint_
    /**
     * @struct cpoint
     * @brief 2D点(整型坐标)
     */
    typedef struct cpoint
    {
        int x; /*!< x坐标 */
        int y; /*!< y坐标 */
    } cpoint;
#endif

#ifndef cpointf_
#define cpointf_
    /**
     * @struct cpointf
     * @brief 2D点(浮点坐标)
     */
    typedef struct cpointf
    {
        float x; /*!< x坐标 */
        float y; /*!< y坐标 */
    } cpointf;
#endif

#ifndef cpointd_
#define cpointd_
    /**
     * @struct cpointd
     * @brief 2D点(双精度浮点坐标)
     */
    typedef struct cpointd
    {
        double x; /*!< x坐标 */
        double y; /*!< y坐标 */
    } cpointd;
#endif

#ifndef csize_
#define csize_
    /**
     * @struct csize
     * @brief 尺寸(整型)
     */
    typedef struct csize
    {
        int width;  /*!< 宽度 */
        int height; /*!< 高度 */
    } csize;
#endif

#ifndef csizef_
#define csizef_
    /**
     * @struct csizef
     * @brief 尺寸(浮点)
     */
    typedef struct csizef
    {
        float width;  /*!< 宽度 */
        float height; /*!< 高度 */
    } csizef;
#endif

#ifndef crect_
#define crect_
    /**
     * @struct crect
     * @brief 矩形区域(整型坐标)
     */
    typedef struct crect
    {
        int x;      /*!< 左上角x坐标 */
        int y;      /*!< 左上角y坐标 */
        int width;  /*!< 宽度 */
        int height; /*!< 高度 */
    } crect;
#endif

#ifndef crectf_
#define crectf_
    /**
     * @struct crectf
     * @brief 矩形区域(浮点坐标)
     */
    typedef struct crectf
    {
        float x;      /*!< 左上角x坐标 */
        float y;      /*!< 左上角y坐标 */
        float width;  /*!< 宽度 */
        float height; /*!< 高度 */
    } crectf;
#endif

#ifndef cline_
#define cline_
    /**
     * @struct cline
     * @brief 线段(浮点坐标)
     */
    typedef struct cline
    {
        cpointf p1; /*!< 起点 */
        cpointf p2; /*!< 终点 */
    } cline;
#endif

#ifndef clined_
#define clined_
    /**
     * @struct clined
     * @brief 线段(双精度浮点坐标)
     */
    typedef struct clined
    {
        cpointd p1; /*!< 起点 */
        cpointd p2; /*!< 终点 */
    } clined;
#endif

#ifndef ccircle_
#define ccircle_
    /**
     * @struct ccircle
     * @brief 圆(浮点坐标)
     */
    typedef struct ccircle
    {
        cpointf center; /*!< 圆心 */
        float radius;   /*!< 半径 */
    } ccircle;
#endif

#ifndef cellipse_
#define cellipse_
    /**
     * @struct cellipse
     * @brief 椭圆(浮点坐标)
     */
    typedef struct cellipse
    {
        cpointf center; /*!< 中心点 */
        float rx;       /*!< x轴半径 */
        float ry;       /*!< y轴半径 */
        float angle;    /*!< 旋转角度(弧度) */
    } cellipse;
#endif

#ifndef cpolygon_
#define cpolygon_
    /**
     * @struct cpolygon
     * @brief 多边形(浮点坐标)
     * @note 顶点按逆时针或顺时针顺序存储
     */
    typedef struct cpolygon
    {
        cpointf* points; /*!< 顶点数组 */
        size_t count;    /*!< 顶点数量 */
        size_t capacity; /*!< 数组容量 */
    } cpolygon;
#endif

#ifndef cbox_
#define cbox_
    /**
     * @struct cbox
     * @brief 包围盒(浮点坐标)
     */
    typedef struct cbox
    {
        cpointf min; /*!< 最小点(左下角) */
        cpointf max; /*!< 最大点(右上角) */
    } cbox;
#endif

#ifndef cboxd_
#define cboxd_
    /**
     * @struct cboxd
     * @brief 包围盒(双精度浮点坐标)
     */
    typedef struct cboxd
    {
        cpointd min; /*!< 最小点(左下角) */
        cpointd max; /*!< 最大点(右上角) */
    } cboxd;
#endif

/* ========================================================================
 * 常量定义
 * ======================================================================== */

#ifndef C2D_PI
#define C2D_PI 3.14159265358979323846
#endif

#ifndef C2D_EPSILON
#define C2D_EPSILON 1e-10
#endif

/* ========================================================================
 * 点操作函数
 * ======================================================================== */

/**
 * @brief 计算两点之间的欧几里得距离
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 两点之间的距离
 */
float c2d_distance_pointf(const cpointf* p1, const cpointf* p2);

/**
 * @brief 计算两点之间的欧几里得距离(双精度)
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 两点之间的距离
 */
double c2d_distance_pointd(const cpointd* p1, const cpointd* p2);

/**
 * @brief 计算两点之间的曼哈顿距离
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 曼哈顿距离
 */
float c2d_manhattan_distance(const cpointf* p1, const cpointf* p2);

/**
 * @brief 计算两点之间的切比雪夫距离
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 切比雪夫距离
 */
float c2d_chebyshev_distance(const cpointf* p1, const cpointf* p2);

/**
 * @brief 点加法
 * @param p1 第一个点
 * @param p2 第二个点
 * @param result 结果点
 */
void c2d_point_add(const cpointf* p1, const cpointf* p2, cpointf* result);

/**
 * @brief 点减法
 * @param p1 第一个点
 * @param p2 第二个点
 * @param result 结果点
 */
void c2d_point_sub(const cpointf* p1, const cpointf* p2, cpointf* result);

/**
 * @brief 点数乘
 * @param p 点
 * @param scalar 标量
 * @param result 结果点
 */
void c2d_point_scale(const cpointf* p, float scalar, cpointf* result);

/**
 * @brief 计算点的模长
 * @param p 点
 * @return 模长
 */
float c2d_point_length(const cpointf* p);

/**
 * @brief 点归一化
 * @param p 点
 * @param result 归一化后的点
 * @return 0成功，-1失败(零向量)
 */
int c2d_point_normalize(const cpointf* p, cpointf* result);

/**
 * @brief 计算两点的点积
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 点积
 */
float c2d_dot_product(const cpointf* p1, const cpointf* p2);

/**
 * @brief 计算两点的叉积(2D中返回标量)
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 叉积的z分量
 */
float c2d_cross_product(const cpointf* p1, const cpointf* p2);

/**
 * @brief 线性插值
 * @param p1 起点
 * @param p2 终点
 * @param t 插值参数[0,1]
 * @param result 结果点
 */
void c2d_lerp(const cpointf* p1, const cpointf* p2, float t, cpointf* result);

/* ========================================================================
 * 线段操作函数
 * ======================================================================== */

/**
 * @brief 计算线段长度
 * @param line 线段
 * @return 长度
 */
float c2d_line_length(const cline* line);

/**
 * @brief 计算点到线段的最短距离
 * @param p 点
 * @param line 线段
 * @return 最短距离
 */
float c2d_distance_point_to_line(const cpointf* p, const cline* line);

/**
 * @brief 计算点到线段的最近点
 * @param p 点
 * @param line 线段
 * @param result 最近点
 */
void c2d_closest_point_on_line(const cpointf* p, const cline* line, cpointf* result);

/**
 * @brief 判断两条线段是否相交
 * @param line1 第一条线段
 * @param line2 第二条线段
 * @return 1相交，0不相交
 */
int c2d_line_intersects(const cline* line1, const cline* line2);

/**
 * @brief 计算两条线段的交点
 * @param line1 第一条线段
 * @param line2 第二条线段
 * @param result 交点(如果存在)
 * @return 1有交点，0无交点
 */
int c2d_line_intersection(const cline* line1, const cline* line2, cpointf* result);

/* ========================================================================
 * 矩形操作函数
 * ======================================================================== */

/**
 * @brief 计算矩形面积
 * @param rect 矩形
 * @return 面积
 */
float c2d_rect_area(const crectf* rect);

/**
 * @brief 计算矩形周长
 * @param rect 矩形
 * @return 周长
 */
float c2d_rect_perimeter(const crectf* rect);

/**
 * @brief 判断点是否在矩形内
 * @param p 点
 * @param rect 矩形
 * @return 1在内部，0不在
 */
int c2d_point_in_rect(const cpointf* p, const crectf* rect);

/**
 * @brief 判断两个矩形是否相交
 * @param rect1 第一个矩形
 * @param rect2 第二个矩形
 * @return 1相交，0不相交
 */
int c2d_rect_intersects(const crectf* rect1, const crectf* rect2);

/**
 * @brief 计算两个矩形的交集
 * @param rect1 第一个矩形
 * @param rect2 第二个矩形
 * @param result 交集矩形
 * @return 1有交集，0无交集
 */
int c2d_rect_intersection(const crectf* rect1, const crectf* rect2, crectf* result);

/**
 * @brief 计算两个矩形的并集(包围盒)
 * @param rect1 第一个矩形
 * @param rect2 第二个矩形
 * @param result 并集矩形
 */
void c2d_rect_union(const crectf* rect1, const crectf* rect2, crectf* result);

/**
 * @brief 判断第一个矩形是否包含第二个矩形
 * @param rect1 第一个矩形
 * @param rect2 第二个矩形
 * @return 1包含，0不包含
 */
int c2d_rect_contains(const crectf* rect1, const crectf* rect2);

/* ========================================================================
 * 圆操作函数
 * ======================================================================== */

/**
 * @brief 计算圆的面积
 * @param circle 圆
 * @return 面积
 */
float c2d_circle_area(const ccircle* circle);

/**
 * @brief 计算圆的周长
 * @param circle 圆
 * @return 周长
 */
float c2d_circle_perimeter(const ccircle* circle);

/**
 * @brief 判断点是否在圆内
 * @param p 点
 * @param circle 圆
 * @return 1在内部，0不在
 */
int c2d_point_in_circle(const cpointf* p, const ccircle* circle);

/**
 * @brief 判断两个圆是否相交
 * @param c1 第一个圆
 * @param c2 第二个圆
 * @return 1相交，0不相交
 */
int c2d_circle_intersects(const ccircle* c1, const ccircle* c2);

/**
 * @brief 判断圆与矩形是否相交
 * @param circle 圆
 * @param rect 矩形
 * @return 1相交，0不相交
 */
int c2d_circle_rect_intersects(const ccircle* circle, const crectf* rect);

/**
 * @brief 判断圆与线段是否相交
 * @param circle 圆
 * @param line 线段
 * @return 1相交，0不相交
 */
int c2d_circle_line_intersects(const ccircle* circle, const cline* line);

/* ========================================================================
 * 多边形操作函数
 * ======================================================================== */

/**
 * @brief 创建多边形
 * @param capacity 初始容量
 * @return 多边形指针，失败返回NULL
 */
cpolygon* c2d_polygon_create(size_t capacity);

/**
 * @brief 销毁多边形
 * @param poly 多边形
 */
void c2d_polygon_destroy(cpolygon* poly);

/**
 * @brief 向多边形添加顶点
 * @param poly 多边形
 * @param p 顶点
 * @return 0成功，-1失败
 */
int c2d_polygon_add_point(cpolygon* poly, const cpointf* p);

/**
 * @brief 清空多边形顶点
 * @param poly 多边形
 */
void c2d_polygon_clear(cpolygon* poly);

/**
 * @brief 计算多边形面积(使用鞋带公式)
 * @param poly 多边形
 * @return 面积(有符号，逆时针为正)
 */
float c2d_polygon_area(const cpolygon* poly);

/**
 * @brief 计算多边形周长
 * @param poly 多边形
 * @return 周长
 */
float c2d_polygon_perimeter(const cpolygon* poly);

/**
 * @brief 计算多边形质心
 * @param poly 多边形
 * @param centroid 质心
 * @return 0成功，-1失败
 */
int c2d_polygon_centroid(const cpolygon* poly, cpointf* centroid);

/**
 * @brief 判断点是否在多边形内(射线法)
 * @param p 点
 * @param poly 多边形
 * @return 1在内部，0不在
 */
int c2d_point_in_polygon(const cpointf* p, const cpolygon* poly);

/**
 * @brief 判断点是否在多边形内(回转数法)
 * @param p 点
 * @param poly 多边形
 * @return 1在内部，0不在
 */
int c2d_point_in_polygon_winding(const cpointf* p, const cpolygon* poly);

/**
 * @brief 计算多边形包围盒
 * @param poly 多边形
 * @param box 包围盒
 * @return 0成功，-1失败
 */
int c2d_polygon_envelope(const cpolygon* poly, cbox* box);

/**
 * @brief 判断多边形是否为凸多边形
 * @param poly 多边形
 * @return 1是凸多边形，0不是
 */
int c2d_polygon_is_convex(const cpolygon* poly);

/**
 * @brief 多边形简化(Douglas-Peucker算法)
 * @param poly 原多边形
 * @param epsilon 容差
 * @param result 简化后的多边形
 * @return 0成功，-1失败
 */
int c2d_polygon_simplify(const cpolygon* poly, float epsilon, cpolygon* result);

/**
 * @brief 计算凸包(Graham扫描法)
 * @param points 点集
 * @param count 点数量
 * @param hull 凸包多边形
 * @return 0成功，-1失败
 */
int c2d_convex_hull(const cpointf* points, size_t count, cpolygon* hull);

/**
 * @brief 多边形缓冲区(扩展/收缩)
 * @param poly 原多边形
 * @param distance 缓冲距离(正数扩展，负数收缩)
 * @param result 结果多边形
 * @return 0成功，-1失败
 */
int c2d_polygon_buffer(const cpolygon* poly, float distance, cpolygon* result);

/* ========================================================================
 * 包围盒操作函数
 * ======================================================================== */

/**
 * @brief 从点集计算包围盒
 * @param points 点集
 * @param count 点数量
 * @param box 包围盒
 * @return 0成功，-1失败
 */
int c2d_envelope_points(const cpointf* points, size_t count, cbox* box);

/**
 * @brief 判断点是否在包围盒内
 * @param p 点
 * @param box 包围盒
 * @return 1在内部，0不在
 */
int c2d_point_in_box(const cpointf* p, const cbox* box);

/**
 * @brief 判断两个包围盒是否相交
 * @param box1 第一个包围盒
 * @param box2 第二个包围盒
 * @return 1相交，0不相交
 */
int c2d_box_intersects(const cbox* box1, const cbox* box2);

/**
 * @brief 计算两个包围盒的交集
 * @param box1 第一个包围盒
 * @param box2 第二个包围盒
 * @param result 交集包围盒
 * @return 1有交集，0无交集
 */
int c2d_box_intersection(const cbox* box1, const cbox* box2, cbox* result);

/**
 * @brief 计算两个包围盒的并集
 * @param box1 第一个包围盒
 * @param box2 第二个包围盒
 * @param result 并集包围盒
 */
void c2d_box_union(const cbox* box1, const cbox* box2, cbox* result);

/**
 * @brief 扩展包围盒
 * @param box 包围盒
 * @param distance 扩展距离
 * @param result 扩展后的包围盒
 */
void c2d_box_expand(const cbox* box, float distance, cbox* result);

/* ========================================================================
 * 几何变换函数
 * ======================================================================== */

/**
 * @brief 点平移
 * @param p 点
 * @param dx x方向偏移
 * @param dy y方向偏移
 * @param result 结果点
 */
void c2d_translate_point(const cpointf* p, float dx, float dy, cpointf* result);

/**
 * @brief 点旋转(绕原点)
 * @param p 点
 * @param angle 旋转角度(弧度)
 * @param result 结果点
 */
void c2d_rotate_point(const cpointf* p, float angle, cpointf* result);

/**
 * @brief 点旋转(绕指定点)
 * @param p 点
 * @param center 旋转中心
 * @param angle 旋转角度(弧度)
 * @param result 结果点
 */
void c2d_rotate_point_around(const cpointf* p, const cpointf* center, 
                              float angle, cpointf* result);

/**
 * @brief 点缩放(绕原点)
 * @param p 点
 * @param sx x方向缩放因子
 * @param sy y方向缩放因子
 * @param result 结果点
 */
void c2d_scale_point(const cpointf* p, float sx, float sy, cpointf* result);

/**
 * @brief 多边形平移
 * @param poly 多边形
 * @param dx x方向偏移
 * @param dy y方向偏移
 * @param result 结果多边形
 * @return 0成功，-1失败
 */
int c2d_translate_polygon(const cpolygon* poly, float dx, float dy, cpolygon* result);

/**
 * @brief 多边形旋转
 * @param poly 多边形
 * @param center 旋转中心
 * @param angle 旋转角度(弧度)
 * @param result 结果多边形
 * @return 0成功，-1失败
 */
int c2d_rotate_polygon(const cpolygon* poly, const cpointf* center, 
                       float angle, cpolygon* result);

/**
 * @brief 多边形缩放
 * @param poly 多边形
 * @param center 缩放中心
 * @param sx x方向缩放因子
 * @param sy y方向缩放因子
 * @param result 结果多边形
 * @return 0成功，-1失败
 */
int c2d_scale_polygon(const cpolygon* poly, const cpointf* center,
                      float sx, float sy, cpolygon* result);

/* ========================================================================
 * 高级几何算法
 * ======================================================================== */

/**
 * @brief 计算多边形的最小包围圆(Welzl算法)
 * @param poly 多边形
 * @param circle 最小包围圆
 * @return 0成功，-1失败
 */
int c2d_minimum_bounding_circle(const cpolygon* poly, ccircle* circle);

/**
 * @brief 计算多边形的最小包围矩形(旋转卡壳法)
 * @param poly 多边形
 * @param rect 最小包围矩形顶点
 * @return 0成功，-1失败
 */
int c2d_minimum_bounding_rect(const cpolygon* poly, cpolygon* rect);

/**
 * @brief 判断两个多边形是否相交(SAT分离轴定理)
 * @param poly1 第一个多边形
 * @param poly2 第二个多边形
 * @return 1相交，0不相交
 */
int c2d_polygon_intersects(const cpolygon* poly1, const cpolygon* poly2);

/**
 * @brief 计算两个凸多边形的交集(Sutherland-Hodgman算法)
 * @param poly1 第一个凸多边形
 * @param poly2 第二个凸多边形
 * @param result 交集多边形
 * @return 0成功，-1失败
 */
int c2d_convex_polygon_intersection(const cpolygon* poly1, const cpolygon* poly2,
                                     cpolygon* result);

/**
 * @brief 多边形三角剖分(耳切法)
 * @param poly 多边形
 * @param triangles 三角形数组(输出)
 * @param max_triangles 最大三角形数量
 * @param count 实际三角形数量(输出)
 * @return 0成功，-1失败
 */
int c2d_polygon_triangulate(const cpolygon* poly, cpolygon* triangles,
                             size_t max_triangles, size_t* count);

/**
 * @brief 计算点到多边形的最短距离
 * @param p 点
 * @param poly 多边形
 * @return 最短距离
 */
float c2d_distance_point_to_polygon(const cpointf* p, const cpolygon* poly);

/**
 * @brief 计算两条线段之间的最短距离
 * @param line1 第一条线段
 * @param line2 第二条线段
 * @return 最短距离
 */
float c2d_distance_line_to_line(const cline* line1, const cline* line2);

/* ========================================================================
 * 工具函数
 * ======================================================================== */

/**
 * @brief 判断浮点数是否近似相等
 * @param a 第一个数
 * @param b 第二个数
 * @param epsilon 误差范围
 * @return 1相等，0不相等
 */
int c2d_float_equal(float a, float b, float epsilon);

/**
 * @brief 角度转弧度
 * @param degrees 角度
 * @return 弧度
 */
float c2d_deg_to_rad(float degrees);

/**
 * @brief 弧度转角度
 * @param radians 弧度
 * @return 角度
 */
float c2d_rad_to_deg(float radians);

/**
 * @brief 限制值在指定范围内
 * @param value 值
 * @param min 最小值
 * @param max 最大值
 * @return 限制后的值
 */
float c2d_clamp(float value, float min, float max);

#ifdef __cplusplus
}
#endif /* end of __cplusplus */
#endif /* C_C2D_H */
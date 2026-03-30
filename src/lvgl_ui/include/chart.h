#ifndef _CHART_H_
#define _CHART_H_

#include "lvgl.h"
#include "clabez/include/cthread.h"
#include "clabez/include/catomic.h"
#include "data_distribution.h"
#include "cjson/cJSON.h"

// TODO 将对象私有结构重新命名
#define CHART_FPS_ENABLE                        1               // 开启曲线刷新帧率的显示
#define CHART_DEMO_DATA                         0               // 测试数据生成


#define CHART_SERIES_NUM                        4               // 能够被添加到图表中系列的最大数量
#define CHART_SERIES_NAME_MAX_LEN               128             // 系列名称的最大长度（bytes）

typedef int32_t (*series_bind_fun)(void* user_data);

typedef void (*x_bind_fun)(int32_t *x_buffer, uint32_t size);

/**
 * @brief 图表类型定义
 * 用于指定系列的绘制类型（折线图、柱状图、散点图）
 */
typedef uint8_t         chart_type_t;

/**
 * @brief 图表颜色类型定义（RGB565格式）
 * 16位颜色值：5位红色 + 6位绿色 + 5位蓝色
 */
typedef uint16_t        chart_color_t;

enum{
    CHART_TYPE_LINE,
    CHART_TYPE_BAR,
    CHART_TYPE_SCATTER
};

// 散点图marker（带不同标记形状）待实现
typedef enum {
    SCATTER_MARKER_CIRCLE,
    SCATTER_MARKER_SQUARE,
    SCATTER_MARKER_TRIANGLE,
    SCATTER_MARKER_DIAMOND,
} scatter_marker_type_t;

/**
 * @brief 刻度配置描述符
 *
 * 用于配置图表的 Y 轴刻度的样式和范围参数
 */
typedef struct
{
    char name[CHART_SERIES_NAME_MAX_LEN];                       // 轴名称名称
    uint32_t tick_color;                                        // 刻度线和文字的颜色
    int32_t tick_width;                                         // 刻度线的宽度（像素）
    int32_t tick_length;                                        // 大刻度线的长度（像素），小刻度线会自动减少

    int32_t tick_count;                                         // 总刻度数量 -1 = 与点数一致
    int32_t tick_every;                                         // 每隔多少个小刻度显示一个大刻度（主刻度）
    int32_t  range_min_value;                                   // 刻度显示的最小值（例如：0）
    int32_t  range_max_value;                                   // 刻度显示的最大值（例如：100）
} tick_dsc_t;


/**
 * @brief 图表参数配置描述符
 *
 * 包含了创建带刻度的曲线图所需的所有配置参数，
 * 包括曲线本身的配置、Y轴刻度配置、X轴默认显示的是一帧的时间
 * 
 * @example 图表参数配置示例
 *  注意：配置的所有关于颜色的参数全部采用RGB888格式，在图表内部有需要的地方会自动转换为RGB565格式
    chart_param_dsc_t chart_param = {
        .chart_conf = {
            .name = "Temperature Monitor",                      // 图表名称
            .chart_bg_color = 0x353131,                         // 暗色背景 (RGB888格式)
            .point_cnt = 200,                                   // 共200个数据点
            .title_color = 0xFFFFFF,                            // 白色标题
            .series_type = {CHART_TYPE_LINE, CHART_TYPE_BAR, CHART_TYPE_SCATTER, CHART_TYPE_BAR}, //图表类型
            .series_enable = {1, 1, 1, 1},                      // 是否使能指定系列
            .series_color = {                                   // 各个系列颜色 (RGB888格式)
                0x3ABCE4,                                       // 系列0: 天蓝色 (亮蓝，用于温度数据)
                0x00FF00,                                       // 系列1: 鲜绿色 (亮绿，用于电流数据)
                0xE4B73A,                                       // 系列2: 琥珀橙色 (金黄，用于湿度数据)
                0xFF1493                                        // 系列3: 深粉红色 (洋红，用于电压数据)
            },
            .series_name = {                                    // 系列名称
                "Temperature",                                  // 系列0: 温度
                "Current",                                      // 系列1: 电流
                "Humidity",                                     // 系列2: 湿度
                "Voltage"                                       // 系列3: 电压
            }                           
            .timer_period = 60,                                 // 60ms 更新一次
            
        },
        .y_tick_conf = {
            .name = "Value",                                    // Y轴名称
            .tick_color = 0xE6D81A,                             // 黄色刻度
            .tick_width = 2,                                    // 刻度线宽度
            .tick_length = 10,                                  // 大刻度长度
            .tick_count = 11,                                   // 11个刻度点
            .tick_every = 2,                                    // 每2个小刻度显示一个大刻度
            .range_min_value = -60,                             // Y轴最小值， 请保留一些距离以显示横轴标签，这个值需要测试一下并且为负值，或者使用以下公式计算，满足下式才能绘制标签
                                                                // c->y_min_value / (c->y_max_value - c->y_min_value) * c->current_height > CHARACTER_HIGHT 
            .range_max_value = 1000                             // Y轴最大值
        }
    };  
 */
typedef struct
{
    /**
     * @brief 曲线图基本配置
     */
    struct{
        char name[CHART_SERIES_NAME_MAX_LEN];                    // 图表名称，显示在图表顶部
        uint32_t chart_bg_color;                                 // 图表背景颜色（RGB565格式），建议使用暗色背景
        uint32_t point_cnt;                                      // 数据点总数（决定了 X 轴的点数），超过屏幕宽度时会自动降采样
        uint32_t title_color;                                    // 标题文字颜色（RGB888格式），建议使用高对比度颜色
        uint32_t series_color[CHART_SERIES_NUM];                 // 系列颜色数组（RGB888格式），若某个系列的颜色设置为0，将使用随机颜色
        chart_type_t series_type[CHART_SERIES_NUM];              // 系列绘制类型（折线/柱状/散点），默认为折线图
        uint8_t series_enable[CHART_SERIES_NUM];                 // 系列启用标志（1=启用，0=禁用），默认不启用
        char series_name[CHART_SERIES_NUM][CHART_SERIES_NAME_MAX_LEN];  // 系列名称数组，显示在图例中
        uint32_t timer_period;                                   // UI线程刷新频率  该值不宜过小以免由于频繁刷新造成的性能开销
                                                                 // 若设置为1ms 也不会达到1000fps，这是因为刷新帧率受到 lv_timer_handler() 的调用频率的制约
        uint32_t vaddr[CHART_SERIES_NUM];                        // 各个系列绑定的虚拟地址， 0xffffffff表示未绑定
    }chart_conf;

    const lv_font_t* x_label_font;     
    uint32_t x_label_color;  
    tick_dsc_t y_tick_conf;                                      // Y 轴刻度配置（垂直刻度，通常表示数值范围）
}chart_param_dsc_t;

typedef struct {
    char name[CHART_SERIES_NAME_MAX_LEN];                        // 系列名称
    float *point_data;                                         // 数据点数组
    float *prepare_point;                                      // 通过降维算法（LTTB）处理后的数组，准备绘制到显示缓冲区
    uint32_t start_point;                                        // 存储最旧的点的索引
    chart_type_t series_type;                                    // 该系列的类型
    uint32_t vaddr;                                              // 虚拟地址
    lv_color_t color;     
                                                                 // 图列需要用的                                    
    lv_obj_t* legend_color;
    lv_obj_t* legend_label;
    lv_obj_t* legend_box;                                        // 图例容器引用，用于显隐控制

    
    series_bind_fun series_fun;                                  // 各个系列的回调函数
    void* user_data;
    uint8_t enabled;                                             // 是否启用该系列（复选框控制）
    uint8_t visible;                                             // 是否可见（眼睛按钮控制）
    bool need_update;                                            // 本次更新周期是否需要刷新该系列
    
    widget_data *wd;

}chart_series_t;

typedef struct {
    char     name[CHART_SERIES_NAME_MAX_LEN];
    uint32_t vaddr;
    lv_color_t color;
    bool     enabled;        // 复选框控制：是否启用（绑定数据源）
    bool     visible;        // 眼睛按钮控制：是否可见（临时隐藏）
} chart_series_conf_t;


typedef struct {
    volatile uint8_t is_resizing;                                         // 是否正在调整大小，为了避免在缓冲区分配完成之前就关闭程序造成意外错误。
    lv_obj_t *base_obj;
    lv_obj_t *canvas;
    uint16_t *buffer[2];                                         // 双缓冲区
    uint8_t active_buffer;                                       // 当前UI显示的buffer索引 (0或1)
    uint8_t draw_buffer;                                         // 当前绘制的buffer索引 (0或1)
    lv_timer_t *timer;
    lv_timer_t *resize_timer;
    uint32_t frame;                                              // 当前帧
    uint32_t current_width;
    uint32_t current_height;
    uint32_t pending_width;
    uint32_t pending_height;

    chart_color_t chart_bg_color;  
    int32_t y_min_value;
    int32_t y_max_value;
    uint32_t point_cnt;
    uint8_t   series_refresh_method_f;                           // 保留未使用                   

    // 图表的系列数据
    chart_series_t series[CHART_SERIES_NUM];

    struct{
        volatile uint8_t is_running;
        cthread t_handle_data;
        c_counting_semaphore semp_data_ready;
        c_counting_semaphore semp_draw_done;
        c_timed_mutex buffer_mutex;
        c_timed_mutex wd_mutex;                                      // 保护 series[i].wd 指针的读写，防止 UI 线程解绑与数据线程访问竞态
    } chart_thread;

    struct{
        x_bind_fun set_x_values_fun;                             // 保留未使用
        chart_color_t label_color;  
    } x_tick;

    lv_obj_t* legend;
    lv_obj_t* chart_titile;
    lv_obj_t* y_name;
#if CHART_FPS_ENABLE
    lv_obj_t* fps_label;
    uint32_t last_tick;
    uint32_t frame_cnt;
#endif

#if CHART_DEMO_DATA
    lv_timer_t* test_data_timer;
    uint32_t counter;
#endif

    // 参数配置
    chart_series_conf_t dlg_conf[CHART_SERIES_NUM];

    struct{                                                      // 自适应范围需要用的变量
        lv_timer_t* y_scale_timer;
        lv_obj_t* y_scale;
    }y_tick;
} chart_dsc_t;


// 默认参数
static chart_param_dsc_t def_chart_param = {
    .chart_conf = {
        .name = "Chart",
        .chart_bg_color = 0x12161E,
        .point_cnt = 1000,
        .title_color = 0xFFFFFF,
        .series_type = {
            CHART_TYPE_LINE,
            CHART_TYPE_LINE,
            CHART_TYPE_LINE,
            CHART_TYPE_LINE,
        },
        .vaddr  = {0, 0, 0, 0},
        .series_enable = {0, 0, 0, 0},
        .series_color  = {0x4A90E2, 0xC8FF00},
        .timer_period  = 33,
    },
    .x_label_color = 0xffffff,
    .y_tick_conf = {
        .name = "值",
        .tick_color      = 0xE6D81A,
        .tick_width      = 2,
        .tick_length     = 10,
        .tick_count      = 21,
        .tick_every      = 2,
        .range_min_value = -100,
        .range_max_value = 100
    }
};


void Chart_Save(cJSON* item ,void *widget_dsc);

void Chart_Load(void *widget_dsc, cJSON *item);

/**
 * @brief 创建一个新的图表对象
 *
 * 此函数用于创建一个完整的图表组件，包括：
 * - Canvas画布和双缓冲区
 * - 数据处理线程和同步机制
 * - 定时器用于定期更新显示
 * - 可选的FPS显示
 *
 * @param base_obj 父容器对象，图表将在此对象上创建
 * @param chart_param 图表配置参数描述符，包含图表的所有初始配置
 *                    包括：点数、颜色、系列配置、刻度配置等
 *
 * @return chart_dsc_t* 成功返回图表描述符指针，失败返回NULL
 *
 * @note 使用完毕后需要调用 Chart_Delete() 释放资源
 * @note 创建后图表会自动启动数据处理线程
 *
 * @example
 *   chart_param_dsc_t param = {...};
 *   chart_dsc_t* chart = Create_Chart(parent_obj, &param);
 *   if (chart) {
 *       // 使用图表
 *   }
 */
chart_dsc_t* Create_Chart(lv_obj_t* base_obj, const chart_param_dsc_t *chart_param);

/**
 * @brief 删除图表并释放所有资源
 * @param chart 图表描述符指针
 * @note 此函数会：
 *       1. 停止数据处理线程
 *       2. 删除定时器
 *       3. 销毁同步原语（互斥锁、信号量）
 *       4. 释放所有内存（缓冲区、系列数据）
 *       5. 删除Canvas对象
 */
void Chart_Delete(lv_event_t * e);

/**
 * @brief 设置用户数据
 * @param chart 图表描述符指针
 * @param series_id 该图表的哪一个系列
 * @param user_data 用户数据
 * 
 * @return 设置成功返回true,否则返回false
 */
bool Chart_Set_User_Data(chart_dsc_t *c,  uint32_t series_id, void* user_data);
/**
 * @brief 创建RGB565格式的颜色值
 *
 * 将RGB888格式的颜色值(r, g, b)转换为RGB565格式的16位颜色值。
 * RGB565格式：5位红色 + 6位绿色 + 5位蓝色
 *
 * @param r 红色分量，范围 0-255
 * @param g 绿色分量，范围 0-255
 * @param b 蓝色分量，范围 0-255
 *
 * @return uint16_t RGB565格式的颜色值
 *
 * @example
 *   uint16_t red = Chart_Color_Make(255, 0, 0);
 *   uint16_t green = Chart_Color_Make(0, 255, 0);
 *   uint16_t blue = Chart_Color_Make(0, 0, 255);
 */
uint16_t Chart_Color_Make(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 从十六进制颜色代码创建RGB565颜色值
 *
 * 将十六进制颜色代码(如 0xFF0000)转换为RGB565格式的颜色值
 *
 * @param h 十六进制颜色代码，格式：0xRRGGBB
 *
 * @return uint16_t RGB565格式的颜色值
 *
 * @example
 *   uint16_t red = Chart_Color_Hex(0xFF0000);
 *   uint16_t white = Chart_Color_Hex(0xFFFFFF);
 *   uint16_t cyan = Chart_Color_Hex(0x00FFFF);
 */
uint16_t Chart_Color_Hex(uint32_t h);

/**
 * @brief 设置指定数据系列在本次图表更新周期中是否需要刷新
 *
 * 通过设置更新标志位，控制指定数据系列在下一次图表渲染时是否重新绘制。
 * 当数据未发生变化时可关闭刷新以减少不必要的重绘，提升性能。
 *
 * @param c         图表描述符指针，指向目标图表实例
 * @param series_id 数据系列的索引编号（从0开始）
 * @param flag      更新标志，1：需要刷新，0：跳过刷新
 *
 * @example
 *   Chart_Series_Set_Update_Flag(&my_chart, 0, 1);  // 标记系列0需要刷新
 *   Chart_Series_Set_Update_Flag(&my_chart, 1, 0);  // 标记系列1跳过刷新
 */
void Chart_Series_Set_Update_Flag(chart_dsc_t* c, uint32_t series_id, uint8_t flag);

/**
 * @brief 向指定数据系列追加一个新的数据值
 *
 * 以循环缓冲区的方式向数据系列中写入新值，写入位置自动递增。
 * 当缓冲区写满后将从头覆盖旧数据，实现滚动更新效果。
 *
 * @param c         图表描述符指针，指向目标图表实例
 * @param series_id 数据系列的索引编号（从0开始）
 * @param value     要写入的数据值，支持负值
 *
 * @example
 *   Chart_Series_Set_Next_Value(&my_chart, 0, 75);   // 向系列0追加值75
 *   Chart_Series_Set_Next_Value(&my_chart, 0, -20);   // 向系列0追加负值-20
 *   Chart_Series_Set_Next_Value(&my_chart, 1, 100);   // 向系列1追加值100
 */
void Chart_Series_Set_Next_Value(chart_dsc_t* c, uint32_t series_id, float value);

/**
 * @brief 显示指定的图表系列
 *
 * 启用并显示之前被隐藏的系列。
 *
 * @param c 图表描述符指针
 * @param series_id 系列编号
 *
 * @return bool 成功返回true，失败返回false
 *
 * @note 系列必须已经通过 Chart_Series_Bind_Data 绑定了数据
 *
 * @example
 *   Chart_Series_Display(chart, 0);  // 显示第一条系列
 */
bool Chart_Series_Display(chart_dsc_t* c, uint32_t series_id);

/**
 * @brief 隐藏指定的图表系列
 *
 * 临时隐藏某个系列，但不删除其数据。
 *
 * @param c 图表描述符指针
 * @param series_id 系列编号
 *
 * @return bool 成功返回true，失败返回false
 *
 * @note 隐藏的系列可以通过 Chart_Series_Display 重新显示
 *
 * @example
 *   Chart_Series_Hide(chart, 1);  // 隐藏第二条系列
 */
bool Chart_Series_Hide(chart_dsc_t* c, uint32_t series_id);

/**
 * @brief 设置图表系列的颜色
 *
 * 动态更改系列的显示颜色。
 *
 * @param c 图表描述符指针
 * @param series_id 系列编号
 * @param color 新的颜色值（RGB565格式）
 *
 * @return bool 成功返回true，失败返回false
 *
 * @example
 *   uint16_t blue = Chart_Color_Make(0, 0, 255);
 *   Chart_Series_Set_Color(chart, 0, blue);
 */
bool Chart_Series_Set_Color(chart_dsc_t* c, uint32_t series_id, uint32_t color);

/**
 * @brief 设置图表系列的名称
 *
 * 为系列设置或更新显示名称。
 *
 * @param c 图表描述符指针
 * @param series_id 系列编号
 * @param name 系列名称字符串，最大长度 CHART_SERIES_NAME_MAX_LEN
 *
 * @return bool 成功返回true，失败返回false
 *
 * @note 名称字符串会被复制到内部缓冲区，超出长度部分会被截断
 *
 * @example
 *   Chart_Series_Set_Name(chart, 0, "温度");
 *   Chart_Series_Set_Name(chart, 1, "湿度");
 */
bool Chart_Series_Set_Name(chart_dsc_t* c, uint32_t series_id, const char* name);


#endif /* _CHART_H_ */




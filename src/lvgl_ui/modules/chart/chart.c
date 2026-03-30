#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include "modules/include/chart.h"
#include "modules/include/mpmc_queue.h"
#include "data_distribution.h"
#include "modules/include/widght_setting_pop.h"
#include "chart_setting_pop.h"
#include "smp_private.h"

// 以下包含是为了横轴的时间戳的功能
#ifdef _WIN32
    #include <windows.h>
#elif defined(__unix__)
    #include <sys/time.h>
#endif

typedef struct 
{
    lv_obj_t* fps;
    lv_obj_t* details; 
    lv_obj_t* y_name;
}title_area_t;

extern smp_ctx_t * g_smp_ctx;
// 静态函数声明
static chart_dsc_t* Create_Chart_Major(lv_obj_t* base_obj, const chart_param_dsc_t *chart_param);
static lv_obj_t*  Create_Chart_Legend(lv_obj_t* base_obj, chart_dsc_t* c, const chart_param_dsc_t *chart_param);
static lv_obj_t* Curve_Title_Area_Create(lv_obj_t *base_obj, const chart_param_dsc_t* p, title_area_t* title_obj);
static void Init_Series_Data(chart_dsc_t *c, const chart_param_dsc_t *p);
static void Size_Changed_Event_Cb(lv_event_t *e);
static void Reallocate_Buffer(chart_dsc_t *c, lv_coord_t new_width, lv_coord_t new_height);

static void* Handle_Data(void *arg);
static void Update_Data(lv_timer_t *t);
static widget_data* Chart_Series_Bindwd(uint32_t vaddr);
static void Chart_Series_Unbindwd(uint32_t vaddr, widget_data *wd);

static void Clear_Buffer(uint16_t *buffer, uint32_t width, uint32_t height, chart_color_t color);
static void Draw_Grid(chart_dsc_t* c, uint16_t *buffer, uint32_t width, uint32_t height);
static void Draw_All_Graph(chart_dsc_t *c, uint16_t *target_buffer);
static void Draw_Text(uint16_t* buffer, chart_dsc_t* c, int32_t x, int32_t y,
                      char* text, uint16_t color);
static void Draw_Loading(uint16_t* buffer, chart_dsc_t* c, int32_t x, int32_t y, uint16_t color);                      
static uint32_t Get_Milliseconds(void);
static void Y_Scale_Update_Timer_Cb(lv_timer_t *timer);
static void Format_Time_Buffer(char *buf, uint32_t size, uint32_t period,
                                uint32_t line_index, uint32_t line_cnt);
static uint32_t LTTB_Down_Sample(const float *input_data, uint32_t start_point, uint32_t input_size,
                         float *output_data, uint32_t threshold);                                
static inline int32_t Value_To_Pixel_Y(float value, int32_t y_min, int32_t y_max, uint32_t height);
static void Draw_Circle_Filled_Fast(uint16_t *buffer, uint32_t width, uint32_t height,
                                    int32_t cx, int32_t cy, uint32_t radius, uint16_t color);
static void Draw_Rectangle_Filled(uint16_t *buffer, uint32_t width, uint32_t height,
                                   int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color);
static void Draw_Line(chart_dsc_t *c, uint16_t *buffer, uint32_t width, uint32_t height,
                      float *data, uint32_t start_point, uint32_t point_count, uint16_t color, uint8_t thickness);
static void Draw_Bar(chart_dsc_t *c, uint16_t *buffer, uint32_t width, uint32_t height,
                     float *data, uint32_t start_point, uint32_t point_count, uint16_t color,
                     uint32_t series_index, uint32_t total_series);
static void Draw_Scatter(chart_dsc_t *c, uint16_t *buffer, uint32_t width, uint32_t height,
                         float *data, uint32_t start_point, uint32_t point_count, uint16_t color, const uint32_t dot_radius);


static void Chart_Details_Cb(lv_event_t *e);

void Chart_Save(cJSON* item ,void *widget_dsc)
{
    if(!item || !widget_dsc)    return;
    chart_dsc_t* c = (chart_dsc_t* )widget_dsc;
    cJSON* names = cJSON_CreateArray();
    cJSON* vaddrs = cJSON_CreateArray();
    cJSON* enables = cJSON_CreateArray();
    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        cJSON_AddItemToArray(names, cJSON_CreateString(c->series[i].name));
        cJSON_AddItemToArray(vaddrs, cJSON_CreateNumber(c->series[i].vaddr == 0xFFFFFFFF ? -1 : (int)c->series[i].vaddr));
        cJSON_AddItemToArray(enables, cJSON_CreateNumber(c->series[i].enabled));
    }
    cJSON_AddItemToObject(item, "series_names", names);
    cJSON_AddItemToObject(item, "vaddr", vaddrs);
    cJSON_AddItemToObject(item, "series_enable", enables);

    cJSON_AddStringToObject(item, "chart_title", lv_label_get_text(c->chart_titile));
    cJSON_AddStringToObject(item, "y_axis_name", lv_label_get_text(c->y_name));
}

void Chart_Load(void *widget_dsc, cJSON *item)
{
    if (!widget_dsc || !item) return;
    chart_param_dsc_t *p = (chart_param_dsc_t *)widget_dsc;

    cJSON *names   = cJSON_GetObjectItem(item, "series_names");
    cJSON *vaddrs  = cJSON_GetObjectItem(item, "vaddr");
    cJSON *enables = cJSON_GetObjectItem(item, "series_enable");

    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        cJSON *n = cJSON_GetArrayItem(names, i);
        if (cJSON_IsString(n)) {
            strncpy(p->chart_conf.series_name[i], n->valuestring,
                    CHART_SERIES_NAME_MAX_LEN - 1);
            p->chart_conf.series_name[i][CHART_SERIES_NAME_MAX_LEN - 1] = '\0';
        }

        cJSON *v = cJSON_GetArrayItem(vaddrs, i);
        if (cJSON_IsNumber(v)) {
            p->chart_conf.vaddr[i] = (v->valueint == -1) ? 0xFFFFFFFF : (uint32_t)v->valueint;
        }

        cJSON *en = cJSON_GetArrayItem(enables, i);
        if (cJSON_IsNumber(en)) {
            p->chart_conf.series_enable[i] = (uint8_t)en->valueint;
        }
    }

    cJSON *title = cJSON_GetObjectItem(item, "chart_title");
    if (cJSON_IsString(title)) {
        strncpy(p->chart_conf.name, title->valuestring,
                sizeof(p->chart_conf.name) - 1);
        p->chart_conf.name[sizeof(p->chart_conf.name) - 1] = '\0';
    }

    cJSON *yname = cJSON_GetObjectItem(item, "y_axis_name");
    if (cJSON_IsString(yname)) {
        strncpy(p->y_tick_conf.name, yname->valuestring,
                sizeof(p->y_tick_conf.name) - 1);
        p->y_tick_conf.name[sizeof(p->y_tick_conf.name) - 1] = '\0';
    }
}

chart_dsc_t* Create_Chart(lv_obj_t* base_obj, const chart_param_dsc_t *chart_param)
{
    LV_ASSERT_NULL(chart_param);
    chart_dsc_t* c = NULL;
    lv_obj_t* main_container = lv_obj_create(base_obj);
    lv_obj_set_size(main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(main_container, 0, LV_PART_MAIN);  
    lv_obj_set_style_bg_opa(main_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);

    title_area_t title_obj;
    lv_obj_t* title_lable = Curve_Title_Area_Create(main_container, chart_param, &title_obj);

    lv_obj_t* chart_container = lv_obj_create(main_container);
    lv_obj_set_size(chart_container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(chart_container, 1);
    lv_obj_set_style_bg_opa(chart_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_left(chart_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(chart_container, LV_FLEX_FLOW_ROW);

    // ========== Y 轴刻度 ==========
    lv_obj_t* y_scale = lv_scale_create(chart_container);
    lv_obj_set_size(y_scale, 60, lv_pct(95));
    lv_scale_set_mode(y_scale, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_scale_set_label_show(y_scale, true);
    lv_obj_set_style_bg_opa(y_scale, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(y_scale, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(y_scale, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(y_scale, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(y_scale, chart_param->y_tick_conf.tick_width, LV_PART_MAIN); 
    lv_obj_set_style_line_color(y_scale, lv_color_hex(chart_param->y_tick_conf.tick_color), LV_PART_MAIN); 
    lv_obj_set_style_pad_top(y_scale, 5, LV_PART_MAIN); 
    //lv_obj_set_style_pad_all(y_scale, chart_param->pad_size, LV_PART_MAIN); 
    
    // Y 轴刻度配置
    lv_scale_set_total_tick_count(y_scale, 
        (chart_param->y_tick_conf.tick_count < 0 ?  
         (chart_param->chart_conf.point_cnt + 1) :  
         chart_param->y_tick_conf.tick_count));
    lv_scale_set_major_tick_every(y_scale, chart_param->y_tick_conf.tick_every);  
    lv_scale_set_range(y_scale, 
        chart_param->y_tick_conf.range_min_value, 
        chart_param->y_tick_conf.range_max_value);  

    // Y 轴小刻度线样式
    int32_t y_minor_tick_len = chart_param->y_tick_conf.tick_length / 2; 
    lv_obj_set_style_length(y_scale, y_minor_tick_len, LV_PART_ITEMS);
    lv_obj_set_style_line_width(y_scale, chart_param->y_tick_conf.tick_width - 1, LV_PART_ITEMS); 
    lv_obj_set_style_line_color(y_scale, lv_color_hex(chart_param->y_tick_conf.tick_color), LV_PART_ITEMS);  

    // Y 轴大刻度线样式
    lv_obj_set_style_length(y_scale, chart_param->y_tick_conf.tick_length, LV_PART_INDICATOR);  
    lv_obj_set_style_line_width(y_scale, chart_param->y_tick_conf.tick_width, LV_PART_INDICATOR);  
    lv_obj_set_style_line_color(y_scale, lv_color_hex(chart_param->y_tick_conf.tick_color), LV_PART_INDICATOR);  
    
    // Y 轴文字标签样式
    lv_obj_set_style_text_color(y_scale, lv_color_hex(chart_param->y_tick_conf.tick_color), LV_PART_MAIN); 
    lv_obj_set_style_text_font(y_scale, &lv_font_montserrat_14, LV_PART_MAIN);
    
    // ========== 右侧容器 ==========
    lv_obj_t* right_container = lv_obj_create(chart_container);
    lv_obj_set_style_bg_opa(right_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_grow(right_container, 1);  
    lv_obj_set_height(right_container, lv_pct(100));
    lv_obj_set_style_bg_opa(right_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_container, 0, 0);
    lv_obj_set_style_pad_all(right_container, 0, 0);
    lv_obj_set_flex_flow(right_container, LV_FLEX_FLOW_COLUMN);

    // ========== 创建图表容器 ==========
    lv_obj_t* chart_wrapper = lv_obj_create(right_container);
    lv_obj_set_style_bg_opa(chart_wrapper, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_size(chart_wrapper, lv_pct(100), lv_pct(95));
    lv_obj_set_style_bg_opa(chart_wrapper, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(chart_wrapper, 0, LV_PART_MAIN);
    // 控制canvas左右pad
    lv_obj_set_style_pad_top(chart_wrapper, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_left(chart_wrapper, 7, LV_PART_MAIN);
    // lv_obj_set_style_pad_right(chart_wrapper, 5, LV_PART_MAIN);

    c = Create_Chart_Major(chart_wrapper, chart_param);
    c->y_tick.y_scale = y_scale;
#if CHART_FPS_ENABLE    
    c->fps_label = title_obj.fps;
#endif  
    lv_obj_add_event_cb(title_obj.details, Chart_Details_Cb, LV_EVENT_CLICKED, c);

    c->legend = Create_Chart_Legend(chart_wrapper, c, chart_param);
    c->chart_titile = title_lable;
    c->y_name = title_obj.y_name;

    lv_obj_remove_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(chart_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(right_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(chart_wrapper, LV_OBJ_FLAG_SCROLLABLE);
    
    return c;
}

void Chart_Delete(lv_event_t * e)
{
    chart_dsc_t* chart = (chart_dsc_t *)lv_event_get_user_data(e);
    LV_ASSERT_FORMAT_MSG(chart, "Chart_Delete: chart is NULL");

    if (chart->chart_thread.is_running) {
        chart->chart_thread.is_running = 0;  
        // 手动释放一次避免线程阻塞
        c_counting_semaphore_release(&chart->chart_thread.semp_draw_done, 1);
        cthread_join(&chart->chart_thread.t_handle_data);
    }


#if CHART_DEMO_DATA
    if (chart->test_data_timer) {
        lv_timer_delete(chart->test_data_timer);
    }
#endif
    if(chart->y_tick.y_scale_timer)
        lv_timer_delete(chart->y_tick.y_scale_timer);
     
    if(chart->timer)
        lv_timer_delete(chart->timer);
 
    c_timed_mutex_destroy(&chart->chart_thread.buffer_mutex);
    c_timed_mutex_destroy(&chart->chart_thread.wd_mutex);

    c_counting_semaphore_destroy(&chart->chart_thread.semp_data_ready);
    c_counting_semaphore_destroy(&chart->chart_thread.semp_draw_done);
    
    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        Chart_Series_Unbindwd(chart->series[i].vaddr, chart->series[i].wd);
        chart->series[i].wd = NULL;

        if (chart->series[i].point_data) {
            lv_free(chart->series[i].point_data);
            chart->series[i].point_data = NULL;
            lv_free(chart->series[i].prepare_point);
            chart->series[i].prepare_point = NULL;
        }
    }

    if (chart->buffer[0]) {
        lv_free(chart->buffer[0]);
        chart->buffer[0] = NULL;
    }
    if (chart->buffer[1]) {
        lv_free(chart->buffer[1]);
        chart->buffer[1] = NULL;
    }
    lv_free(chart);
    chart = NULL;
}


uint16_t Chart_Color_Make(uint8_t r, uint8_t g, uint8_t b) 
{  
    uint16_t res = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return res;
}

uint16_t Chart_Color_Hex(uint32_t h)
{
    uint8_t r = (h >> 16) & 0xFF;
    uint8_t g = (h >> 8)  & 0xFF;
    uint8_t b =  h        & 0xFF;

    return ((r & 0xF8) << 8) |   
           ((g & 0xFC) << 3) |   
           ( b >> 3);          
}

// ====== setter 函数 ====== 
void Chart_Series_Set_Update_Flag(chart_dsc_t* c, uint32_t series_id, uint8_t flag)
{
    LV_ASSERT_NULL(c);
    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,"series_id is already greater than CHART_SERIES_NUM in %s", __func__);
    }

    c->series[series_id].need_update = flag;
}

void Chart_Series_Set_Next_Value(chart_dsc_t* c, uint32_t series_id, float value)
{
    LV_ASSERT_NULL(c);

    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,"series_id is already greater than CHART_SERIES_NUM in %s", __func__);
    }

    uint32_t idx = c->series[series_id].start_point;
    c->series[series_id].point_data[idx] = value;
    c->series[series_id].start_point = (idx + 1) % c->point_cnt;
}

bool Chart_Series_Display(chart_dsc_t* c, uint32_t series_id)
{
    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,
                                "series_id is already greater than CHART_SERIES_NUM in %s", __func__);
        return false;
    }

    if(c->series[series_id].visible == 0)
    {
        c->series[series_id].visible = 1;
    }
    return true;
}

bool Chart_Series_Hide(chart_dsc_t* c, uint32_t series_id)
{
    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,
                                "series_id is already greater than CHART_SERIES_NUM in %s", __func__);
        return false;
    }

    if(c->series[series_id].visible == 1)
    {
        c->series[series_id].visible = 0;
    }

    return true;
}

bool Chart_Series_Set_Color(chart_dsc_t* c, uint32_t series_id, uint32_t color)
{
    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,
                                "series_id is already greater than CHART_SERIES_NUM in %s", __func__);
        return false;
    }

    // 设置系列颜色
    c->series[series_id].color = lv_color_hex(color);
    lv_obj_set_style_bg_color(c->series[series_id].legend_color, lv_color_hex(color), LV_PART_MAIN);
    return true;
}

bool Chart_Series_Set_Name(chart_dsc_t* c, uint32_t series_id, const char* name)
{
    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,
                                "series_id is already greater than CHART_SERIES_NUM in %s", __func__);
        return false;
    }

    if(name == NULL)
    {
        LV_ASSERT_FORMAT_MSG(name != NULL,
                                "name is NULL in %s", __func__);
        return false;
    }

    lv_memset(c->series[series_id].name, 0, CHART_SERIES_NAME_MAX_LEN);
    lv_strncpy(c->series[series_id].name, name, CHART_SERIES_NAME_MAX_LEN - 1);
    lv_label_set_text(c->series[series_id].legend_label, name);
    return true;
}

bool Chart_Set_User_Data(chart_dsc_t *c,  uint32_t series_id, void* user_data)
{
    if(series_id >= CHART_SERIES_NUM)
    {
        LV_ASSERT_FORMAT_MSG(series_id < CHART_SERIES_NUM,"series_id is already greater than CHART_SERIES_NUM in %s", __func__);
        return false;
    }
    c->series[series_id].user_data = user_data;
    return true;
}

#if CHART_DEMO_DATA
static void Test_Data_Timer_Cb(lv_timer_t *t)
{
    chart_dsc_t *c = (chart_dsc_t *)t->user_data;
    if (!c) return;

    c->counter++;
    uint32_t phase = c->counter % 8000;
    double amplitude = (phase < 4000) ? (100.0 + (3900.0 / 2000.0) * phase)       
                                      : (4000.0 - (3900.0 / 2000.0) * (phase - 2000)); 
    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        double freq = 0.05 * (i + 1);
        float val = (sin(c->counter * freq + i * 1.0) * amplitude);
        Chart_Series_Set_Next_Value(c, i, val);
    }
}
#endif

//=========== 工具函数 ===========
static chart_dsc_t* Create_Chart_Major(lv_obj_t* base_obj, const chart_param_dsc_t *chart_param)
{
       
    lv_obj_update_layout(base_obj);
    // 获取初始尺寸
    lv_coord_t width = lv_obj_get_width(base_obj);
    lv_coord_t height = lv_obj_get_height(base_obj);

    // 分配数据结构
    chart_dsc_t *m_chart = lv_malloc(sizeof(chart_dsc_t));
    LV_ASSERT_MALLOC(m_chart);

    // 初始化数据
    lv_memset(m_chart, 0, sizeof(chart_dsc_t));
    m_chart->is_resizing = 0;
    m_chart->base_obj = base_obj;
    m_chart->canvas = NULL;
    m_chart->buffer[0] = NULL;
    m_chart->buffer[1] = NULL;
    m_chart->current_width = width;
    m_chart->current_height = height;
    m_chart->frame = 0;
    m_chart->chart_bg_color = Chart_Color_Hex(chart_param->chart_conf.chart_bg_color);
    m_chart->timer = NULL;
    m_chart->active_buffer = 0;  
    m_chart->draw_buffer = 1;   
    m_chart->y_min_value = chart_param->y_tick_conf.range_min_value;
    m_chart->y_max_value = chart_param->y_tick_conf.range_max_value;

    // 默认使用10点的容量，避免除 0
    if(chart_param->chart_conf.point_cnt == 0)
        m_chart->point_cnt = 10;
    else
        m_chart->point_cnt = chart_param->chart_conf.point_cnt;

    m_chart->x_tick.label_color =  Chart_Color_Hex(chart_param->x_label_color);
    m_chart->x_tick.set_x_values_fun = NULL;
#if CHART_FPS_ENABLE
    m_chart->last_tick = 0;
    m_chart->frame_cnt = 0;
#endif
    // 分配双缓冲区
    m_chart->buffer[0] = lv_malloc(width * height * sizeof(chart_color_t));
    m_chart->buffer[1] = lv_malloc(width * height * sizeof(chart_color_t));
    if (!m_chart->buffer[0] || !m_chart->buffer[1]) {
        if (m_chart->buffer[0]) lv_free(m_chart->buffer[0]);
        if (m_chart->buffer[1]) lv_free(m_chart->buffer[1]);
        lv_free(m_chart);

        LV_ASSERT_MSG(m_chart->buffer[0] || m_chart->buffer[1], "Failed to allocate buffers");
        return NULL;
    }

    // 初始化系列数据
    Init_Series_Data(m_chart, chart_param);

    // 可选绘制初始内容到两个buffer
    Draw_Grid(m_chart, m_chart->buffer[0], width, height);
    Draw_Grid(m_chart, m_chart->buffer[1], width, height); 
    // 创建canvas，使用active_buffer (buffer[0])
    m_chart->canvas = lv_canvas_create(base_obj);
    lv_obj_set_size(m_chart->canvas, lv_pct(100), lv_pct(100));
    lv_canvas_set_buffer(m_chart->canvas, m_chart->buffer[m_chart->active_buffer], width, height, LV_COLOR_FORMAT_RGB565);
    
    // 注册尺寸变化事件监听
    lv_obj_add_event_cb(base_obj, Size_Changed_Event_Cb, LV_EVENT_SIZE_CHANGED, m_chart);

    m_chart->timer = lv_timer_create(Update_Data, chart_param->chart_conf.timer_period, m_chart);

    // 将数据存储到对象的用户数据中
    lv_obj_set_user_data(base_obj, m_chart);

    // y轴自适应范围定时器
    m_chart->y_tick.y_scale_timer = lv_timer_create(Y_Scale_Update_Timer_Cb, 300, m_chart); 
    /*
        m_chart->series_refresh_method_f = 0x00;
        for(int i = 0; i < CHART_SERIES_NUM; i++)
        {
            if(chart_param->chart_conf.series_refresh_method[i] == CHART_REFRESH_INITIATIVE)
                m_chart->series_refresh_method_f |= (0x01 << i);
        }
    */

    // 初始化线程相关
    m_chart->chart_thread.is_running = 1;
    
    // 初始化信号量
    c_counting_semaphore_init(&m_chart->chart_thread.semp_data_ready, 0);
    c_counting_semaphore_init(&m_chart->chart_thread.semp_draw_done, 1);

    cthread_errc err_code;
    // 初始化互斥锁
    if ((err_code = c_timed_mutex_init(&m_chart->chart_thread.buffer_mutex)) != CTHREAD_OK) {
        LV_ASSERT_MSG( err_code != CTHREAD_OK, "Failed to initialize buffer mutex");
        m_chart->chart_thread.is_running = 0;
    }
    if ((err_code = c_timed_mutex_init(&m_chart->chart_thread.wd_mutex)) != CTHREAD_OK) {
        LV_ASSERT_MSG( err_code != CTHREAD_OK, "Failed to initialize wd mutex");
        m_chart->chart_thread.is_running = 0;
    }

    // 创建数据生成线程
    if ((err_code = cthread_create(&m_chart->chart_thread.t_handle_data, Handle_Data, m_chart)) != CTHREAD_OK) {
            LV_ASSERT_MSG(err_code != CTHREAD_OK, "Failed to create data generation thread");
        m_chart->chart_thread.is_running = 0;
    }
    
    // 添加内存清理的函数回调
    lv_obj_add_event_cb(base_obj, Chart_Delete, LV_EVENT_DELETE, m_chart);

#if CHART_DEMO_DATA
    m_chart->counter = 0;
    if (m_chart) {
        m_chart->test_data_timer = lv_timer_create(Test_Data_Timer_Cb, 20, m_chart);
    }
#endif

    return m_chart;
}

static lv_obj_t* Create_Chart_Legend(lv_obj_t* base_obj, chart_dsc_t* c, const chart_param_dsc_t *chart_param)
{
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    // 图例容器
    lv_obj_t*  m_legend = lv_obj_create(base_obj);
    lv_obj_set_style_bg_opa(m_legend, LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_style_border_width(m_legend, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(m_legend, lv_color_hex(0xFFFFFF),LV_PART_MAIN);
    lv_obj_set_style_border_opa(m_legend, LV_OPA_30, LV_PART_MAIN);                                    
    lv_obj_align(m_legend, LV_ALIGN_TOP_RIGHT, -4, 5);

    int enabled_cnt = 0;
    for(int i = 0; i < CHART_SERIES_NUM; i++)
        if(c->series[i].enabled) enabled_cnt++;

    int box_hight = 30 * enabled_cnt;
    int par_hight = lv_obj_get_height(base_obj)*0.4;
    if(box_hight > par_hight)
    {
        lv_obj_set_size(m_legend, 100, par_hight);
    }
    else
    {
        lv_obj_set_size(m_legend, 100, box_hight);
        lv_obj_remove_flag(m_legend, LV_OBJ_FLAG_SCROLLABLE);
    }
  
    lv_obj_set_flex_flow(m_legend, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(m_legend, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for(int i = 0; i < CHART_SERIES_NUM; i++)
    {
        lv_obj_t* box = lv_obj_create(m_legend);
        lv_obj_set_size(box, lv_pct(100), 30);
        lv_obj_set_flex_grow(box, 0);
        lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_flex_flow(box, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, LV_PART_MAIN);

        lv_obj_t* line_color = lv_obj_create(box);
        lv_obj_set_size(line_color, lv_pct(40), 5);

        lv_obj_set_style_bg_color(line_color, c->series[i].color, LV_PART_MAIN);
        c->series[i].legend_color = line_color;
        lv_obj_set_style_radius(line_color, 2, LV_PART_MAIN);

        lv_obj_t* name = lv_label_create(box);
        lv_label_set_text(name, chart_param->chart_conf.series_name[i]);
        lv_obj_set_style_text_font(name, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(name, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        c->series[i].legend_label = name;
        c->series[i].legend_box = box;

        /* 未启用的系列图例隐藏 */
        if(!c->series[i].enabled)
            lv_obj_add_flag(box, LV_OBJ_FLAG_HIDDEN);
    }

    return m_legend;
}   

/* 清理 widget_data 中已部分分配的队列资源（memset 0 保证未分配字段为 NULL） */
static void Chart_Queue_Cleanup(widget_data *wd)
{
    if(!wd) return;
    for(int k = 0; k < RING_BUFFER_SIZE; k++) {
        if(wd->chart_queue.data[k])
            lv_free(wd->chart_queue.data[k]);
    }
    if(wd->chart_queue.queue)     MPMC_Queue_Destroy(&wd->chart_queue.queue);
    if(wd->chart_queue.free_pool) MPMC_Queue_Destroy(&wd->chart_queue.free_pool);
    lv_free(wd);
}

static widget_data* Chart_Series_Bindwd(uint32_t vaddr)
{
    widget_data *wd = (widget_data *)lv_malloc(sizeof(widget_data));
    if(!wd) return NULL;
    memset(wd, 0, sizeof(widget_data));
    wd->type = VALUE_TYPE_QUEUE;

    wd->chart_queue.free_pool = MPMC_Queue_Init(RING_BUFFER_SIZE);
    if(!wd->chart_queue.free_pool) {
        Chart_Queue_Cleanup(wd);
        return NULL;
    }

    wd->chart_queue.queue = MPMC_Queue_Init(RING_BUFFER_SIZE);
    if(!wd->chart_queue.queue) {
        Chart_Queue_Cleanup(wd);
        return NULL;
    }

    for(int k = 0; k < RING_BUFFER_SIZE; k++)
    {
        wd->chart_queue.data[k] = (float *)lv_malloc(sizeof(float));
        if(!wd->chart_queue.data[k]) {
            Chart_Queue_Cleanup(wd);
            return NULL;
        }
        MPMC_Queue_Enqueue(wd->chart_queue.free_pool, wd->chart_queue.data[k]);
    }
    return Data_Map_Addr_Bind(vaddr, wd);
}

static void Chart_Series_Unbindwd(uint32_t vaddr, widget_data *wd)
{
    if(!wd || vaddr == 0 || vaddr == 0xFFFFFFFF) return;
    
    Data_Map_Addr_Remove(vaddr, wd);

    for(int k = 0; k < RING_BUFFER_SIZE; k++)
    {
        if(wd->chart_queue.data[k])
            lv_free(wd->chart_queue.data[k]);
    }
    MPMC_Queue_Destroy(&wd->chart_queue.queue);
    MPMC_Queue_Destroy(&wd->chart_queue.free_pool);
    lv_free(wd);
}

static void Update_Legend_Height(chart_dsc_t *c)
{
    if(!c->legend) return;
    int enabled_cnt = 0;
    for(int i = 0; i < CHART_SERIES_NUM; i++)
        if(c->series[i].enabled) enabled_cnt++;

    int box_height = 30 * enabled_cnt;
    lv_obj_t *parent = lv_obj_get_parent(c->legend);
    int max_height = (int)(lv_obj_get_height(parent) * 0.4);

    if(box_height > max_height) {
        lv_obj_set_height(c->legend, max_height);
        lv_obj_add_flag(c->legend, LV_OBJ_FLAG_SCROLLABLE);
    } else {
        lv_obj_set_height(c->legend, box_height > 0 ? box_height : 0);
        lv_obj_remove_flag(c->legend, LV_OBJ_FLAG_SCROLLABLE);
    }
}

static void Chart_Series_Confirmed(chart_series_conf_t *conf, void *user_data)
{
    chart_dsc_t *c = (chart_dsc_t *)user_data;
    for(int i = 0; i < CHART_SERIES_NUM; i++)
    {
        uint8_t was_enabled = c->series[i].enabled;
        c->series[i].enabled = conf[i].enabled;
        c->series[i].visible = conf[i].visible;
        strncpy(c->series[i].name, conf[i].name, CHART_SERIES_NAME_MAX_LEN);
        if(c->series[i].legend_label)
            lv_label_set_text(c->series[i].legend_label, c->series[i].name);

        /* 图例显隐跟随 enabled */
        if(c->series[i].legend_box) {
            if(conf[i].enabled)
                lv_obj_remove_flag(c->series[i].legend_box, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_add_flag(c->series[i].legend_box, LV_OBJ_FLAG_HIDDEN);
        }

        if(!conf[i].enabled) {
            /* 禁用：解绑数据源 —— 先持锁断开引用，再释放资源 */
            if(was_enabled && c->series[i].wd) {
                widget_data *old_wd = c->series[i].wd;
                c_timed_mutex_lock(&c->chart_thread.wd_mutex);
                c->series[i].wd = NULL;
                c_timed_mutex_unlock(&c->chart_thread.wd_mutex);
                Chart_Series_Unbindwd(c->series[i].vaddr, old_wd);
            }
            c->series[i].vaddr = 0xFFFFFFFF;
        } else {
            /* 启用：地址有变化时重新绑定 */
            if(c->series[i].vaddr != conf[i].vaddr) {
                if(c->series[i].wd) {
                    widget_data *old_wd = c->series[i].wd;
                    c_timed_mutex_lock(&c->chart_thread.wd_mutex);
                    c->series[i].wd = NULL;
                    c_timed_mutex_unlock(&c->chart_thread.wd_mutex);
                    Chart_Series_Unbindwd(c->series[i].vaddr, old_wd);
                }
                c->series[i].wd = Chart_Series_Bindwd(conf[i].vaddr);
                c->series[i].vaddr = conf[i].vaddr;
            } else if(!was_enabled) {
                /* 新启用但地址相同（首次启用） */
                c->series[i].wd = Chart_Series_Bindwd(conf[i].vaddr);
                c->series[i].vaddr = conf[i].vaddr;
            }
        }
    }

    Update_Legend_Height(c);
}

static void Chart_Details_Cb(lv_event_t *e)
{
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    chart_dsc_t* c = (chart_dsc_t*)lv_event_get_user_data(e);
    if(c == NULL)
        return;

    for(int i = 0; i < CHART_SERIES_NUM; i++)
    {
        strncpy(c->dlg_conf[i].name, c->series[i].name, CHART_SERIES_NAME_MAX_LEN);
        c->dlg_conf[i].enabled = c->series[i].enabled;
        c->dlg_conf[i].visible = c->series[i].visible;
        c->dlg_conf[i].vaddr = c->series[i].vaddr;
    }

    Series_Dialog_Open(btn, c->dlg_conf, Chart_Series_Confirmed, c);
}

static lv_obj_t* Curve_Title_Area_Create(lv_obj_t *base_obj, const chart_param_dsc_t* p, title_area_t* title_obj)
{
    //LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    lv_obj_t* title_container =lv_obj_create(base_obj);
    lv_obj_set_size(title_container, lv_pct(100), 40);
    lv_obj_set_style_bg_opa(title_container, LV_OPA_TRANSP, LV_PART_MAIN);
    
    lv_obj_t* y_axis_label = lv_label_create(title_container);
    lv_obj_set_style_text_color(y_axis_label, lv_color_hex(p->y_tick_conf.tick_color), LV_PART_MAIN);
    lv_label_set_text(y_axis_label, p->y_tick_conf.name);
    lv_obj_add_flag(y_axis_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(y_axis_label, Label_Set_Name, LV_EVENT_CLICKED, NULL);
    /* 水平居中对齐到 y_scale 宽度(60px)范围内 */
    lv_obj_align(y_axis_label, LV_ALIGN_LEFT_MID, 30, 0);

    lv_obj_t* chart_label_title = lv_label_create(title_container);
    lv_label_set_text(chart_label_title, p->chart_conf.name); 
    lv_obj_set_style_text_color(chart_label_title, lv_color_hex(p->chart_conf.title_color), LV_PART_MAIN);
    lv_obj_add_flag(chart_label_title, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(chart_label_title, Label_Set_Name,LV_EVENT_CLICKED, NULL);
    lv_obj_align(chart_label_title, LV_ALIGN_CENTER,0,0);


    lv_obj_t* fps = NULL;
#if CHART_FPS_ENABLE
    fps = lv_label_create(title_container);
    lv_obj_set_size(fps, 100, lv_pct(100));
    lv_label_set_text(fps, "fps: 0"); 
    lv_obj_set_style_text_color(fps,  lv_color_hex(p->chart_conf.title_color), LV_PART_MAIN);
    lv_obj_align(fps, LV_ALIGN_RIGHT_MID, -20 ,5);
#endif

    lv_obj_t* details = lv_obj_create(title_container);
    lv_obj_set_size(details, 32, 32);
    lv_obj_set_style_bg_opa(details, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_image_src(details, g_smp_ctx->imgs[IMG_DETALS], LV_PART_MAIN);
    lv_obj_align(details, LV_ALIGN_RIGHT_MID,0,0);
    
    lv_obj_remove_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(details, LV_OBJ_FLAG_SCROLLABLE);

    title_obj->fps = fps;
    title_obj->details = details;
    title_obj->y_name = y_axis_label;

    return chart_label_title;
}

static void Clear_Buffer(uint16_t *buffer, uint32_t width, uint32_t height, chart_color_t color)
{
     for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x <  width; x++) {
            buffer[y * width + x] = color;
        }
    }
}

static void Draw_Loading(uint16_t* buffer, chart_dsc_t* c,
                            int32_t x, int32_t y,
                            uint16_t color)
{
    extern const unsigned char loading_bytes[];
    // 默认字节宽度为8bit
    #define CHARACTER_NUM_LOAD      7
    #define CHARACTER_WIDTH         2
    #define CHARACTER_HIGHT         16
    
    int cur_c = 0;
    int buf_idx = 0;

    for(int idx = 0; idx < CHARACTER_NUM_LOAD; idx++)
    {
        for(int i = 0; i < CHARACTER_HIGHT; i++)
        {
            for(int j = 0; j < 2; j++)
            {
                for(int k = 0; k < 8; k++)
                {
                    // 68 = 字的个数 x 字宽
                    // https://www.zhetao.com/fontarray.html
                    if(loading_bytes[(i*(CHARACTER_NUM_LOAD*CHARACTER_WIDTH) + j) + CHARACTER_WIDTH * idx] & (0x80 >> k))
                    {
                        buf_idx = (y+i) * c->current_width + x + cur_c +(j*8+k);
                        if(buf_idx < c->current_width * c->current_height)
                            buffer[buf_idx] = color;
                    }
                }
            }
        }
        cur_c += (CHARACTER_HIGHT);
    }
   
}

static void Init_Series_Data(chart_dsc_t *c, const chart_param_dsc_t *p)
{
    for (int i = 0; i < CHART_SERIES_NUM; i++) 
    {
        strncpy(c->series[i].name, p->chart_conf.series_name[i], CHART_SERIES_NAME_MAX_LEN);
        // 分配降维数据点的存储数组
        c->series[i].prepare_point = (float *) lv_malloc(c->current_width * sizeof(float));
        LV_ASSERT_MALLOC(c->series[i].prepare_point);

        c->series[i].point_data =(float *) lv_malloc(c->point_cnt* sizeof(float));
        LV_ASSERT_MALLOC(c->series[i].point_data);

        if (p->chart_conf.series_color[i] == 0) {
            static uint32_t s_color_index = 0;
            uint32_t h = (++s_color_index * 137) % 360;
            uint32_t s = 230; 
            uint32_t v = 230;
            uint32_t c_val = (s * v) / 255;
            uint32_t x   = c_val * (60 - (uint32_t)abs((int)(h % 120) - 60)) / 60;
            uint32_t m   = v - c_val;
            uint8_t r, g, b;
            if      (h < 60)  { r = (uint8_t)(c_val+m); g = (uint8_t)(x+m);     b = (uint8_t)m; }
            else if (h < 120) { r = (uint8_t)(x+m);     g = (uint8_t)(c_val+m); b = (uint8_t)m; }
            else if (h < 180) { r = (uint8_t)m;         g = (uint8_t)(c_val+m); b = (uint8_t)(x+m); }
            else if (h < 240) { r = (uint8_t)m;         g = (uint8_t)(x+m);     b = (uint8_t)(c_val+m); }
            else if (h < 300) { r = (uint8_t)(x+m);     g = (uint8_t)m;         b = (uint8_t)(c_val+m); }
            else              { r = (uint8_t)(c_val+m); g = (uint8_t)m;         b = (uint8_t)(x+m); }

            c->series[i].color = lv_color_make(r, g, b);
        } else {
            c->series[i].color = lv_color_hex(p->chart_conf.series_color[i]);
        }

        c->series[i].series_type = p->chart_conf.series_type[i];
        c->series[i].enabled = p->chart_conf.series_enable[i];
        lv_memset(c->series[i].point_data, 0, c->point_cnt * sizeof(float));
        lv_memset(c->series[i].prepare_point, 0, c->current_width * sizeof(float));

        c->series[i].series_fun  = NULL;
        c->series[i].user_data   = NULL;
        c->series[i].need_update = true;
        c->series[i].visible     = 1;
        c->series[i].start_point = 0;

        c->dlg_conf[i].color = c->series[i].color;
        c->series[i].vaddr = p->chart_conf.vaddr[i];

        if(p->chart_conf.series_enable[i])
        {
            c->series[i].wd = Chart_Series_Bindwd(p->chart_conf.vaddr[i]);
        }
    }


    Clear_Buffer(c->buffer[0], c->current_width, c->current_height, c->chart_bg_color);
    Clear_Buffer(c->buffer[1], c->current_width, c->current_height, c->chart_bg_color);

    Draw_Loading(c->buffer[0], c, (c->current_width/2 - 48), c->current_height/2, 0xffff);
    Draw_Loading(c->buffer[1], c, (c->current_width/2 - 48), c->current_height/2, 0xffff);
}

// TODO 若绘制出现负值可能会导致遮挡问题，导致字符看不清，目前采用的方式是直接在时间戳下绘制黑色底纹
static void Draw_Text(uint16_t* buffer, chart_dsc_t* c,
                            int32_t x, int32_t y,
                            char* text,
                            uint16_t color)
{
    extern const unsigned char bitmap_bytes[];
    // 默认字节宽度为8bit
    #define CHARACTER_NUM           12
    #define CHARACTER_WIDTH         2
    #define CHARACTER_HIGHT         16



    int idx = 0, cur_c = 0;
    char *ptr = text;
    int buf_idx = 0;
    for(; *ptr != '\0'; ptr++)
    {
        if(*ptr == ':')
            idx = 10;
        else if(*ptr == '.')
            idx = 11;
        else
            idx = *ptr - 48;

        for(int i = 0; i < CHARACTER_HIGHT; i++)
        {
            for(int j = 0; j < 2; j++)
            {
                for(int k = 0; k < 8; k++)
                {
                    buf_idx = (y+i) * c->current_width + x + cur_c +(j*8+k);
                    if(buf_idx < c->current_width * c->current_height)
                        buffer[buf_idx] = 0x0000;
                    else
                        continue;

                    // 68 = 字的个数 x 字宽
                    // https://www.zhetao.com/fontarray.html
                    if(bitmap_bytes[(i*(CHARACTER_NUM*CHARACTER_WIDTH) + j) + CHARACTER_WIDTH * idx] & (0x80 >> k))
                    {
                        buffer[buf_idx] = color;
                    }

                }
            }
        }
        cur_c += (CHARACTER_HIGHT/2);
    }
   
}

static uint32_t Get_Milliseconds(void)
{
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wMilliseconds;
#elif defined(__unix__) || defined(__APPLE__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec / 1000;
#else
    // 不支持毫秒的平台，返回0， 也就是时间的精度只有秒级精度
    return 0;
#endif
}

static void Format_Time_Buffer(char *buf, uint32_t size, uint32_t period, uint32_t line_index ,uint32_t line_cnt)
{
    time_t now;
    struct tm *local_time;
    uint32_t ms;

    time(&now);
    local_time = localtime(&now);

    ms = Get_Milliseconds();

    ms += (line_index * period / line_cnt);

    if(ms >= 1000)
        local_time->tm_sec++;

    lv_snprintf(buf, size, "%d:%d:%d.%d", 
                local_time->tm_hour,
                local_time->tm_min, 
                local_time->tm_sec, 
                ms % 1000);  
            
}

// 每个时间戳约占 12字符 * 8px = 96px，加间距取 120px 作为最小间距。
static uint32_t Calc_V_Line_Count(uint32_t width)
{
    uint32_t n = width / 120;
    if(n < 2)  n = 2;
    if(n > 20) n = 20;
    return n;
}

static void Draw_Time_Stamp(chart_dsc_t* c, uint16_t *buffer, uint32_t width, uint32_t height)
{

    const uint32_t v_offset = 4;
    const uint32_t v_line_count = Calc_V_Line_Count(width);

    uint32_t v_step = width / v_line_count;

     uint32_t line_index = 0;
    for (uint32_t x = v_offset; x < width; x += v_step) {
        // 绘制时间戳
        if (line_index < v_line_count) {

            if(c->y_min_value > 0 || fabs(c->y_min_value) / (c->y_max_value - c->y_min_value) * c->current_height < CHARACTER_HIGHT)
                continue;

            char label_text[14];
            int32_t text_x = x + 8;
            int32_t text_y = Value_To_Pixel_Y(0, c->y_min_value, c->y_max_value, c->current_height) + 5;
            
            Format_Time_Buffer(label_text, sizeof(label_text),  c->timer->period, line_index, v_line_count);
            // 直接绘制
            Draw_Text(buffer, c, text_x, text_y,
                            label_text, c->x_tick.label_color);   
        }
        
        line_index++;
    }
}

static void Draw_Grid(chart_dsc_t* c, uint16_t *buffer, uint32_t width, uint32_t height)
{
    uint16_t grid_color = Chart_Color_Hex(0x6C6C6C);
    
    // 偏移是由y刻度的pad 决定的
    const uint32_t h_offset = 3;  
    const uint32_t v_offset = 4;   
    const uint32_t v_line_count = Calc_V_Line_Count(width);

    uint32_t h_step = height / 10;
    uint32_t v_step = width / v_line_count;

    // 绘制水平线
    for (uint32_t y = h_offset; y < height; y += h_step) {
        for (uint32_t x = v_offset; x < width; x++) {
            buffer[y * width + x] = grid_color;
        }
    }

    // 绘制垂直线
    for (uint32_t x = v_offset; x < width; x += v_step) {
        for (uint32_t y = h_offset; y < height; y++) {
            buffer[y * width + x] = grid_color;
        }
    }
}

// 支持不同类型系列混合显示
static void Draw_All_Graph(chart_dsc_t *c, uint16_t *target_buffer)
{
    // 清空缓冲区并绘制网格
    Clear_Buffer(target_buffer, c->current_width, c->current_height, c->chart_bg_color);
    Draw_Grid(c, target_buffer, c->current_width, c->current_height);
    
    // 统计各类型的启用系列数量
    uint32_t bar_series_count = 0;
    uint32_t line_series_count = 0;
    uint32_t scatter_series_count = 0;
    
    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        if (c->series[i].enabled == 1) {
            switch (c->series[i].series_type) {
                case CHART_TYPE_BAR:
                    bar_series_count++;
                    break;
                case CHART_TYPE_LINE:
                    line_series_count++;
                    break;
                case CHART_TYPE_SCATTER:
                    scatter_series_count++;
                    break;
            }
        }
    }
    
    // 绘制图表（按类型分组绘制，先柱状图，再折线图，最后散点图）
    uint32_t current_bar_index = 0;
    
    // 先绘制所有柱状图（作为背景）
    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        //没有启用 不可见 无需更新的系列直接跳过
        if(!c->series[i].enabled || !c->series[i].visible || !c->series[i].need_update)
            continue;
    
        uint16_t color = Chart_Color_Make(c->series[i].color.red, c->series[i].color.green, c->series[i].color.blue);

        if (c->series[i].series_type == CHART_TYPE_BAR) {
            if(c->point_cnt > c->current_width)
            {
                Draw_Bar(c, target_buffer,
                        c->current_width, c->current_height,
                        c->series[i].prepare_point, 0, c->current_width,
                        color,
                        current_bar_index,
                        bar_series_count);  // 只计算柱状图系列数
            }
            else{
                Draw_Bar(c, target_buffer,
                        c->current_width, c->current_height,
                        c->series[i].point_data, c->series[i].start_point, c->point_cnt,
                        color,
                        current_bar_index,
                        bar_series_count);  
            }
                
            current_bar_index++;
        }

        if (c->series[i].series_type == CHART_TYPE_LINE) {
            if(c->point_cnt > c->current_width)
            {
                Draw_Line(c, target_buffer,
                        c->current_width, c->current_height,
                        c->series[i].prepare_point, 0, c->current_width,
                        color, 1);
            }else{
                 Draw_Line(c, target_buffer,
                        c->current_width, c->current_height,
                        c->series[i].point_data, c->series[i].start_point, c->point_cnt,
                        color, 1);
            }
        }

        if (c->series[i].series_type == CHART_TYPE_SCATTER) {
            if(c->point_cnt > c->current_width)
            {
                Draw_Scatter(c, target_buffer,
                        c->current_width, c->current_height,
                        c->series[i].prepare_point, 0, c->current_width,
                        color, 2);
            }else{
                 Draw_Scatter(c, target_buffer,
                        c->current_width, c->current_height,
                        c->series[i].point_data, c->series[i].start_point, c->point_cnt,
                        color, 2);
            }

        }
    }

    Draw_Time_Stamp(c, target_buffer, c->current_width, c->current_height);
}

static void* Handle_Data(void *arg)
{
    chart_dsc_t *c = (chart_dsc_t *)arg;
    while (c->chart_thread.is_running) {

        if(c->is_resizing) {
            cthread_timespec ts = {.tv_sec = 0, .tv_nsec = 1000000};
            cthread_sleep_for(&ts);
            continue;
        }

        for(int i = 0; i < CHART_SERIES_NUM; i++)
        {
            /* 持锁快照 wd 指针，防止 UI 线程解绑时 wd 被释放后仍被访问 */
            c_timed_mutex_lock(&c->chart_thread.wd_mutex);
            widget_data *wd = c->series[i].wd;
            c_timed_mutex_unlock(&c->chart_thread.wd_mutex);

            if(wd == NULL || !c->series[i].enabled)
                continue;

            void *slot;
            while (MPMC_Queue_Dequeue(wd->chart_queue.queue, &slot)) {
                Chart_Series_Set_Next_Value(c, i, (*(float *)slot));
                MPMC_Queue_Enqueue(wd->chart_queue.free_pool, slot);
            }

            if(c->series[i].enabled)
            {
                if(!c->series[i].need_update)
                    continue;
                // 判断是否需要降维
                // TODO 当柱状图的数量较多时，但是这里使用的是点数和屏幕的像素宽度判断，所以可能会出现柱状图重叠的情况，并不会采取下采样算法
                // 事实上，当c->point_cnt*bar_series_count > c->current_width时，就已经需要进行下采样了
                if(c->point_cnt > c->current_width)
                    LTTB_Down_Sample(c->series[i].point_data, c->series[i].start_point, c->point_cnt, c->series[i].prepare_point, c->current_width);
            }
        }
        
        c_counting_semaphore_acquire(&c->chart_thread.semp_draw_done);
        if(!c->chart_thread.is_running)
            break;

        Draw_All_Graph(c, c->buffer[c->draw_buffer]);

        c_counting_semaphore_release(&c->chart_thread.semp_data_ready, 1);
    
        c_timed_mutex_lock(&c->chart_thread.buffer_mutex);
        uint8_t temp = c->active_buffer;
        c->active_buffer = c->draw_buffer;
        c->draw_buffer = temp;
        c_timed_mutex_unlock(&c->chart_thread.buffer_mutex);

    }
    return NULL;
}

static void Update_Data(lv_timer_t *t)
{
    chart_dsc_t *c = (chart_dsc_t *)t->user_data;

    
    if (!c || !c->buffer[0] || !c->buffer[1] || !c->canvas) {
        return;
    }

    c->frame++;
    
    if (c_counting_semaphore_try_acquire(&c->chart_thread.semp_data_ready) == CTHREAD_OK) {

        c_timed_mutex_lock(&c->chart_thread.buffer_mutex);
        uint8_t current_active = c->active_buffer;
        c_timed_mutex_unlock(&c->chart_thread.buffer_mutex);
        lv_canvas_set_buffer(c->canvas, c->buffer[current_active],
                             c->current_width, c->current_height, LV_COLOR_FORMAT_RGB565);
        lv_obj_invalidate(c->canvas);
        c_counting_semaphore_release(&c->chart_thread.semp_draw_done, 1);
    }


#if CHART_FPS_ENABLE    
        /* ===== FPS 统计 ===== */
        c->frame_cnt++;
        uint32_t now = lv_tick_get();  

        if (c->last_tick == 0) {
            c->last_tick = now;
        } else if (now - c->last_tick >= 1000) {
            lv_label_set_text_fmt(c->fps_label, "fps: %u", c->frame_cnt);
            c->frame_cnt = 0;
            c->last_tick = now;
        }
#endif

}
static void Reallocate_Buffer(chart_dsc_t *c, lv_coord_t new_width, lv_coord_t new_height)
{
    // 让数据处理线程退出
    uint8_t thread_was_running = c->chart_thread.is_running;
    if (thread_was_running) {
        c->chart_thread.is_running = 0;     
        c_counting_semaphore_release(&c->chart_thread.semp_draw_done, 1);
        cthread_join(&c->chart_thread.t_handle_data);
    }

    // 停止定时器
    bool timer_was_running = true;
    if (c->timer) {
        if(lv_timer_get_paused(c->timer) == false)
        {
            lv_timer_pause(c->timer);
            timer_was_running = false;
        }
    }

    // 清空所有未处理的信号
    while (c_counting_semaphore_try_acquire(&c->chart_thread.semp_data_ready) == CTHREAD_OK) 
        ;

    while (c_counting_semaphore_try_acquire(&c->chart_thread.semp_draw_done) == CTHREAD_OK) 
        ;        

    c->is_resizing = 1;

    // 重新分配所需的缓冲区（用临时变量接收，防止 realloc 失败时丢失旧指针）
    size_t buf_size = (size_t)new_width * new_height * sizeof(uint16_t);
    bool alloc_ok = true;

    void *tmp0 = lv_realloc(c->buffer[0], buf_size);
    void *tmp1 = lv_realloc(c->buffer[1], buf_size);
    if(!tmp0 || !tmp1) {
        /* realloc 失败时旧指针仍然有效
         * 成功的那个需要缩回旧尺寸以保持双缓冲一致 */
        size_t old_size = (size_t)c->current_width * c->current_height * sizeof(uint16_t);
        if(tmp0) c->buffer[0] = lv_realloc(tmp0, old_size);
        if(tmp1) c->buffer[1] = lv_realloc(tmp1, old_size);
        alloc_ok = false;
    } else {
        c->buffer[0] = tmp0;
        c->buffer[1] = tmp1;
    }

    if(alloc_ok) {
        for (int i = 0; i < CHART_SERIES_NUM; i++) {
            float *new_pp = (float*)lv_malloc(new_width * sizeof(float));
            if(!new_pp) {
                alloc_ok = false;
                break;
            }
            // 预防未定义行为
            memset(new_pp, 0, new_width * sizeof(float)); 
            if (c->series[i].prepare_point)
                lv_free(c->series[i].prepare_point);
            c->series[i].prepare_point = new_pp;
        }
    }

    if(alloc_ok) {
        Clear_Buffer(c->buffer[0], new_width, new_height, c->chart_bg_color);
        Clear_Buffer(c->buffer[1], new_width, new_height, c->chart_bg_color);

        // 更新宽高
        c->current_width = new_width;
        c->current_height = new_height;
        c->active_buffer = 0;
        c->draw_buffer = 1;

        Draw_Loading(c->buffer[0], c, (new_width/2 - 48), new_height/2, 0xffff);
        Draw_Loading(c->buffer[1], c, (new_width/2 - 48), new_height/2, 0xffff);

        lv_canvas_set_buffer(c->canvas, c->buffer[c->active_buffer],
                            new_width, new_height, LV_COLOR_FORMAT_RGB565);
    }

    c->is_resizing = 0;
    // 无论成功与否，都恢复定时器和线程
    if (c->timer && !timer_was_running) {
        lv_timer_resume(c->timer);
    }

    if (thread_was_running) {
        c->chart_thread.is_running = 1;
        c_counting_semaphore_release(&c->chart_thread.semp_draw_done, 1);
        cthread_errc err = cthread_create(&c->chart_thread.t_handle_data, Handle_Data, c);
        if (err != CTHREAD_OK) {
            c->chart_thread.is_running = 0;
        }
    }
}

static void Deferred_Resize_Cb(lv_timer_t *t)
{
    chart_dsc_t *c = (chart_dsc_t *)t->user_data;
    c->resize_timer = NULL;

    //printf("pw: %d, ph: %d\n", c->pending_width, c->pending_height);
    if(c->pending_width < 200 || c->pending_height < 200)
    {
        return;
    }

    Reallocate_Buffer(c, c->pending_width, c->pending_height);
    
    lv_obj_move_foreground(c->legend);
}

static void Size_Changed_Event_Cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_SIZE_CHANGED) {
        chart_dsc_t *c = (chart_dsc_t *)lv_event_get_user_data(e);
        lv_obj_t *container = lv_event_get_target(e);
        
        if (!c || !container) return;

        lv_coord_t new_width = lv_obj_get_width(container);
        lv_coord_t new_height = lv_obj_get_height(container);  

        if (new_width == c->current_width && new_height == c->current_height) return;
        if (new_width <= 0 || new_height <= 0) return;

        // 只设置标志和目标尺寸，延迟执行 realloc， 否则线程会使用正在 realloc 的缓冲区
        // 若将容器的宽高都设为了百分比，则这个函数会被连续的两次调用，
        c->pending_width = new_width;
        c->pending_height = new_height;

        if (c->resize_timer == NULL) {
            c->resize_timer = lv_timer_create(Deferred_Resize_Cb, 50, c);
            lv_timer_set_repeat_count(c->resize_timer, 1);
        } else {
            lv_timer_reset(c->resize_timer);
        }
    }
}

// LTTB降采样算法
static uint32_t LTTB_Down_Sample(const float *input_data, uint32_t start_point, uint32_t input_size,
                         float *output_data, uint32_t threshold)
{
    /* 参数检查 */
    if (!input_data || !output_data || input_size < 3 || threshold < 3) {
        LV_ASSERT_MSG(0, "error! the argument is invalid\n");
    }

    #define CIDX(i) ((start_point + (i)) % input_size)

    /* 如果输入数据小于等于目标点数，直接复制 */
    if (input_size <= threshold) {
        for (uint32_t i = 0; i < input_size; i++) {
            output_data[i] = input_data[CIDX(i)];
        }
        return input_size;
    }

    output_data[0] = input_data[CIDX(0)];
    uint32_t output_idx = 1;

    double bucket_size = (double)(input_size - 2) / (double)(threshold - 2);

    /* 上一个选中点的逻辑索引及缓存值 */
    uint32_t prev_selected = 0;
    float prev_value = input_data[CIDX(0)];

    /* 处理中间的桶 */
    for (uint32_t i = 0; i < threshold - 2; i++) {
        /* 当前桶的范围 */
        uint32_t curr_bucket_start = (uint32_t)(floor(i * bucket_size)) + 1;
        uint32_t curr_bucket_end = (uint32_t)(floor((i + 1) * bucket_size)) + 1;

        /* 确保不越界 */
        if (curr_bucket_end > input_size - 1) {
            curr_bucket_end = input_size - 1;
        }
        if (curr_bucket_start >= curr_bucket_end) {
            curr_bucket_start = curr_bucket_end - 1;
        }

        /* 计算下一个桶的平均点 */
        uint32_t next_bucket_start, next_bucket_end;

        if (i < threshold - 3) {
            next_bucket_start = curr_bucket_end;
            next_bucket_end = (uint32_t)(floor((i + 2) * bucket_size)) + 1;
            if (next_bucket_end > input_size) {
                next_bucket_end = input_size;
            }
        } else {
            next_bucket_start = input_size - 1;
            next_bucket_end = input_size;
        }

        /* 计算下一个桶的平均点坐标 */
        double avg_x = 0.0;
        double avg_y = 0;
        uint32_t avg_count = next_bucket_end - next_bucket_start;

        for (uint32_t j = next_bucket_start; j < next_bucket_end; j++) {
            avg_x += (double)j;
            avg_y += (double)input_data[CIDX(j)];
        }
        avg_x /= (double)avg_count;
        avg_y /= (double)avg_count;

        /* 在当前桶中找面积最大的点 */
        double max_area = -1.0;
        uint32_t max_area_idx = curr_bucket_start;

        for (uint32_t j = curr_bucket_start; j < curr_bucket_end; j++) {
            double area = fabs(
                (double)prev_selected * ((double)input_data[CIDX(j)] - (double)avg_y) +
                (double)j * ((double)avg_y - (double)prev_value) +
                avg_x * ((double)prev_value - (double)input_data[CIDX(j)])
            );

            if (area > max_area) {
                max_area = area;
                max_area_idx = j;
            }
        }

        /* 保存选中的点，更新缓存 */
        output_data[output_idx++] = input_data[CIDX(max_area_idx)];
        prev_value = input_data[CIDX(max_area_idx)];
        prev_selected = max_area_idx;
    }

    /* 始终保留最后一个点 */
    output_data[output_idx++] = input_data[CIDX(input_size - 1)];

    #undef CIDX

    return output_idx;
}

/* 向下取整到 step 的倍数 */
static inline int32_t floor_to(int32_t v, int32_t step)
{
    if(step <= 0) step = 1;
    int32_t r = v % step;
    if(r == 0) return v;
    return (v < 0) ? v - (step + r) : v - r;
}

static void Y_Scale_Update_Timer_Cb(lv_timer_t *timer)
{
    chart_dsc_t *c = lv_timer_get_user_data(timer);

    /* 扫描所有启用且可见系列的整个缓冲区，获取真实 min/max 避免受当前值的影响*/
    float buf_min = FLT_MAX, buf_max = -FLT_MAX;
    bool has_data = false;

    for (int i = 0; i < CHART_SERIES_NUM; i++) {
        if (!c->series[i].enabled || !c->series[i].visible) continue;
        if (!c->series[i].point_data) continue;
        const float *data = c->series[i].point_data;
        for (uint32_t j = 0; j < c->point_cnt; j++) {
            float v = data[j];
            if (v < buf_min) buf_min = v;
            if (v > buf_max) buf_max = v;
        }
        has_data = true;
    }

    if (!has_data) return;

    int32_t raw_min = (int32_t)floorf(buf_min);
    int32_t raw_max = (int32_t)ceilf(buf_max);
    if (raw_min >= raw_max) raw_max = raw_min + 1;

    int32_t range  = raw_max - raw_min;
    int32_t margin = range / 10;
    if (margin == 0) margin = 1;

    int32_t padded_min = raw_min - margin;
    int32_t padded_max = raw_max + margin;
    int32_t padded_range = padded_max - padded_min;

    int32_t raw_step = (padded_range + 9) / 10;
    int32_t step;

    if (range >= 20) {
        step = ((raw_step + 4) / 5) * 5;
        if (step < 5) step = 5;
    } else {
        step = raw_step;
        if (step < 1) step = 1;
    }

    int32_t new_min = floor_to(padded_min, step);
    int32_t new_max = new_min + step * 10;

    if (abs(new_min - c->y_min_value) < step / 2 &&
        abs(new_max - c->y_max_value) < step / 2)
        return;

    c->y_min_value = new_min;
    c->y_max_value = new_max;
    lv_scale_set_range(c->y_tick.y_scale, new_min, new_max);
}


// ======= 以下是用于渲染图形的函数 =======
// 将数据值映射到像素Y坐标
static int32_t Value_To_Pixel_Y(float value, int32_t y_min, int32_t y_max, uint32_t height)
{
    // 防御 NaN / Inf：垃圾数据钳到边界
    if (!isfinite(value)) {
        return (value != value) ? 0 : (value > 0 ? 0 : (int32_t)(height - 1));
        // NaN → 顶部; +Inf → 顶部(y=0); -Inf → 底部
    }

    float range = (float)(y_max - y_min);
    if (range == 0.0f) range = 1.0f;

    float y = (float)height - ((value - (float)y_min) * (float)height / range);

    if (y < 0.0f)
        y = 0.0f;
    else if (y >= (float)height)
        y = (float)(height - 1);
    else if (!(y >= 0.0f && y < (float)height))  // 捕获残余 NaN
        y = 0.0f;

    return (int32_t)y;
}

// 使用中点圆算法绘制实心圆（用于散点图）
static void Draw_Circle_Filled_Fast(uint16_t *buffer, uint32_t width, uint32_t height,
                                   int32_t cx, int32_t cy, uint32_t radius, uint16_t color)
{
    if (radius == 0) {
        if (cx >= 0 && cx < (int32_t)width && cy >= 0 && cy < (int32_t)height) {
            buffer[cy * width + cx] = color;
        }
        return;
    }
    
    int32_t x = 0;
    int32_t y = radius;
    int32_t d = 3 - 2 * radius;
    
    #define DRAW_HLINE(x1, x2, y_pos) \
        do { \
            if ((y_pos) >= 0 && (y_pos) < (int32_t)height) { \
                int32_t start = (x1) < 0 ? 0 : (x1); \
                int32_t end = (x2) >= (int32_t)width ? (int32_t)width - 1 : (x2); \
                for (int32_t px = start; px <= end; px++) { \
                    buffer[(y_pos) * width + px] = color; \
                } \
            } \
        } while(0)
    
    while (x <= y) {
        DRAW_HLINE(cx - x, cx + x, cy + y);
        DRAW_HLINE(cx - x, cx + x, cy - y);
        DRAW_HLINE(cx - y, cx + y, cy + x);
        DRAW_HLINE(cx - y, cx + y, cy - x);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
    
    #undef DRAW_HLINE
}

// 绘制矩形（用于柱状图）
static void Draw_Rectangle_Filled(uint16_t *buffer, uint32_t width, uint32_t height,
                                 int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color)
{
    // 确保坐标顺序正确
    if (x1 > x2) {
        int32_t temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        int32_t temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    // 边界检查
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= (int32_t)width) x2 = width - 1;
    if (y2 >= (int32_t)height) y2 = height - 1;
    
    // 绘制填充矩形
    for (int32_t y = y1; y <= y2; y++) {
        for (int32_t x = x1; x <= x2; x++) {
            buffer[y * width + x] = color;
        }
    }
}

// Draw_Line函数
static void Draw_Line(chart_dsc_t *c, uint16_t *buffer, uint32_t width, uint32_t height, 
                        float *data, uint32_t start_point, uint32_t point_count, 
                        uint16_t color, uint8_t thickness)
{
    if (!c || !buffer || !data || point_count < 2) return;
    if (thickness < 1) thickness = 1;

    const uint32_t dot_radius = thickness + 1;
    float x_step = (float)width / (point_count - 1);
    int half = thickness / 2;

    /* 先绘制线段 */
    for (uint32_t i = 0; i < point_count - 1; i++) {
        int x1 = (int)(i * x_step);
        int x2 = (int)((i + 1) * x_step);

        int y1 = Value_To_Pixel_Y(data[(start_point + i) % point_count], c->y_min_value, c->y_max_value, height);
        int y2 = Value_To_Pixel_Y(data[(start_point + i + 1) % point_count], c->y_min_value, c->y_max_value, height);

        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        while (1) {
            if (thickness == 1) {
                /* 单像素 - 快速路径 */
                if (x1 >= 0 && x1 < (int)width && y1 >= 0 && y1 < (int)height) {
                    buffer[y1 * width + x1] = color;
                }
            } else {
                /* 粗线条 - 绘制方块区域 */
                for (int dy_offset = -half; dy_offset <= half; dy_offset++) {
                    for (int dx_offset = -half; dx_offset <= half; dx_offset++) {
                        int px = x1 + dx_offset;
                        int py = y1 + dy_offset;
                        if (px >= 0 && px < (int)width && py >= 0 && py < (int)height) {
                            buffer[py * width + px] = color;
                        }
                    }
                }
            }

            if (x1 == x2 && y1 == y2) break;

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }

    /* 再绘制每个数据点上的圆点 */
    // for (uint32_t i = 0; i < point_count; i++) {
    //     uint32_t x = (uint32_t)(i * x_step);
    //     int y = Value_To_Pixel_Y(data[(start_point + i) % point_count], c->y_min_value, c->y_max_value, height);

    //     Draw_Circle_Filled_Fast(buffer, width, height, x, y, dot_radius, color);
    // }
}


// 绘制柱状图
static void Draw_Bar(chart_dsc_t *c, uint16_t *buffer, uint32_t width, uint32_t height,
                    float *data, uint32_t start_point, uint32_t point_count, 
                    uint16_t color, uint32_t series_index, uint32_t total_series)
{
    if (point_count == 0) return;
    
    // 计算柱状图参数
    const float bar_gap_ratio = 0.2f;      // 柱子组之间的间距比例
    const float series_gap_ratio = 0.05f;  // 同一组内不同系列之间的间距
    
    // 计算每组柱子的总宽度
    float group_width = (float)width / point_count;
    
    // 计算每组的有效宽度（减去组间间距）
    float effective_group_width = group_width * (1.0f - bar_gap_ratio);
    
    float bar_width;
    float series_offset;
    
    if (total_series == 1) {
        // 单系列：柱子占满整个有效宽度
        bar_width = effective_group_width;
        series_offset = 0;
    } else {
        // 多系列：计算每个柱子的宽度和偏移
        bar_width = (effective_group_width / total_series) * (1.0f - series_gap_ratio);
        float gap_between_bars = (effective_group_width - bar_width * total_series) / (total_series - 1);
        series_offset = series_index * (bar_width + gap_between_bars);
    }
    
    // 确保柱子宽度至少为1像素
    if (bar_width < 1.0f) bar_width = 1.0f;
    
    int32_t zero_y = Value_To_Pixel_Y(0, c->y_min_value, c->y_max_value, height);
    
    // 绘制每个数据点的柱子
    for (uint32_t i = 0; i < point_count; i++) {
        // 计算当前组的起始X坐标
        float group_x = i * group_width + group_width * bar_gap_ratio * 0.5f;
        
        // 计算当前柱子的X坐标
        int32_t bar_x1 = (int32_t)(group_x + series_offset);
        int32_t bar_x2 = (int32_t)(bar_x1 + bar_width);
        
        // 确保不超出边界
        if (bar_x2 >= (int32_t)width) bar_x2 = width - 1;
        
        // 获取数据值
        float value = data[(start_point + i) % point_count];
        
        // 计算柱子的Y坐标
        int32_t bar_y = Value_To_Pixel_Y(value, c->y_min_value, c->y_max_value, height);
        
        // 确定柱子的起点和终点
        int32_t y1, y2;
        if (value >= 0) {
            // 正值：从零线向上
            y1 = bar_y;
            y2 = zero_y;
        } else {
            // 负值：从零线向下
            y1 = zero_y;
            y2 = bar_y;
        }
        
        // 确保y1 <= y2
        if (y1 > y2) {
            int32_t temp = y1;
            y1 = y2;
            y2 = temp;
        }
        
        // 跳过高度为0的柱子
        if (y1 >= y2) continue;
        
        // 绘制柱子
        Draw_Rectangle_Filled(buffer, width, height, bar_x1, y1, bar_x2, y2, color);
    }
}

// 绘制散点图
static void Draw_Scatter(chart_dsc_t *c, uint16_t *buffer, uint32_t width, uint32_t height,
                        float *data, uint32_t start_point, uint32_t point_count,
                        uint16_t color, const uint32_t dot_radius)
{
    if (point_count == 0) return;
    
    // 计算每个数据点的X坐标
    float x_step = (float)width / (point_count - 1);
    if (point_count == 1) x_step = width / 2.0f;  // 单点情况
    
    // 绘制每个散点
    for (uint32_t i = 0; i < point_count; i++) {
        uint32_t x = (point_count == 1) ? (width / 2) : (uint32_t)(i * x_step);
        
        int32_t y = Value_To_Pixel_Y(data[(start_point + i) % point_count], c->y_min_value, c->y_max_value, height);
        
        Draw_Circle_Filled_Fast(buffer, width, height, x, y, dot_radius, color);
    }
}


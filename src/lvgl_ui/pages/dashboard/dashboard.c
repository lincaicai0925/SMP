//TODO
/*
    针对图标控件，当订阅某个偏移地址后，设置新的数据。当所有数据已经设置完成（一帧数据已经设置完成）
通知图表强制刷新（设置信号量，设置定时器）。
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cjson/cJSON.h"
#include "clabez/include/clist.h"
#include "pages/include/dashboard.h"
#include "pages/dashboard/arrange_dialog.h"
#include "modules/include/chart.h"
#include "modules/include/arc.h"
#include "modules/include/slider.h"
#include "modules/include/styles.h"
#include "modules/include/checkbox.h"
#include "modules/include/dropdown.h"
#include "modules/include/button.h"
#include "modules/include/switch.h"
#include "modules/include/textarea.h"
#include "modules/include/led.h"
#include "modules/include/spinner.h"
#include "modules/include/spinbox.h"
#include "modules/include/tips_box.h"

#define HANDLE_SIZE     15       /* 缩放手柄直径 */
#define WRAPPER_PAD     15       /* wrapper 四边 padding (拖拽热区) */
#define MIN_WRAPPER_W   100      /* wrapper 最小尺寸 */
#define MIN_WRAPPER_H   100
#define GHOST_W         90
#define GHOST_H         32
#define CANVAS_MARGIN   40       /* canvas 超出最远控件的额外余量 */
#define WIDGET_JSON_PATH    "./conf/tabview.json"

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

enum {
    WIDGET_TYPE_CHART = 0,
    WIDGET_TYPE_LED,
    WIDGET_TYPE_SLIDER,
    WIDGET_TYPE_BUTTON,
    // WIDGET_TYPE_SWITCH,
    // WIDGET_TYPE_CHECKBOX,
    // WIDGET_TYPE_ARC,
    // WIDGET_TYPE_DROPDOWN,
    // WIDGET_TYPE_TEXTAREA,
    WIDGET_TYPE_COUNT,
};

typedef struct {
    const char *name;
    int         type;
    int32_t     default_w;
    int32_t     default_h;
} Palette_Item_t;

/* 数据监控控件 */
static const Palette_Item_t s_Monitor_Items[] = {
    {"图表",     WIDGET_TYPE_CHART,     400, 300},
    {"指示器",   WIDGET_TYPE_LED,       120,  120},
};

/* 可操作控件 */
static const Palette_Item_t s_Operate_Items[] = {
    {"滑动条",  WIDGET_TYPE_SLIDER,    200,  80},
    {"按钮",   WIDGET_TYPE_BUTTON,    100,  70}
};

/* 根据 type 查找默认尺寸 */
static const Palette_Item_t *Find_Palette_Item(int type)
{
    for(int i = 0; i < (int)(sizeof(s_Monitor_Items)/sizeof(s_Monitor_Items[0])); i++)
        if(s_Monitor_Items[i].type == type) return &s_Monitor_Items[i];
    for(int i = 0; i < (int)(sizeof(s_Operate_Items)/sizeof(s_Operate_Items[0])); i++)
        if(s_Operate_Items[i].type == type) return &s_Operate_Items[i];
    return NULL;
}

// 每种控件实现的保存/加载接口
typedef void (*Widget_Load_Fn)(void *widget_dsc, cJSON *item);
typedef void (*Widget_Save_Fn)(cJSON *item, void *widget_dsc);

typedef struct {
    Widget_Save_Fn  save;
    Widget_Load_Fn  load;           /* 将 JSON 字段写入 param 结构体 */
    const void     *default_param;  /* 指向静态默认参数 */
    size_t          param_size;     /* sizeof(该类型的 param 结构体) */
} Widget_Ops_t;

static const Widget_Ops_t s_Widget_Ops[WIDGET_TYPE_COUNT] = {
    [WIDGET_TYPE_CHART] = {
        .save          = Chart_Save,
        .load          = Chart_Load,
        .default_param = &def_chart_param,
        .param_size    = sizeof(chart_param_dsc_t)
    },
    [WIDGET_TYPE_LED] = {
        .save          = Led_Save,
        .load          = Led_Load,
        .default_param = &def_led_param,
        .param_size    = sizeof(led_param_dsc_t)
    },
    [WIDGET_TYPE_BUTTON] = {
        .save          = Btn_Save,
        .load          = Btn_Load,
        .default_param = &def_btn_param,
        .param_size    = sizeof(btn_param_dsc_t)
    }
};

typedef struct {
    lv_obj_t *wrapper;
    lv_obj_t *resize_handle;
    lv_obj_t *delete_handle;
    int        widget_type;
    void     *widget_dsc_t;           //   组件结构
    // 响应式布局：相对于 activity_content 的百分比 (可 > 1.0，表示在滚动区域)
    float      pct_x;
    float      pct_y;
    float      pct_w;
    float      pct_h;
} Editor_Widget_t;

static struct {
    lv_obj_t *ghost;              /* 拖拽过程中的半透明预览 */
    lv_obj_t *activity_content;   /* 可滚动视口 */
    lv_obj_t *canvas;             /* 无限画布, activity_content 的唯一子对象 */
    int        widget_type;       /* 当前拖拽的控件类型 */
} s_Editor;

typedef Editor_Widget_t* ewt;
DEFINE_CLIST(ewt);
static clist_ewt *ew_list;

extern smp_ctx_t * g_smp_ctx;
/*
 *  拖拽共享状态 (同时只有一个手柄被操作)
 */
static lv_point_t s_Press_Pt;
static lv_point_t s_Drag_Offset;
static int32_t    s_Orig_W, s_Orig_H;

static void             Style_Handle(lv_obj_t *handle, lv_palette_t color);
static void*            Create_Widget(lv_obj_t *parent, int type, void* arg);
static void             Sync_Resize_Handle_Pos(Editor_Widget_t *ew, int32_t wx, int32_t wy, int32_t ww, int32_t wh);
static void             Update_Canvas_Size(void);
static Editor_Widget_t* Create_Editor_Widget(lv_obj_t *canvas, int type, void* arg,
                                              int32_t x, int32_t y, int32_t w, int32_t h);
static void             Create_Palette(lv_obj_t *menu);
static void             Palette_Event_Cb(lv_event_t *e);
static void             Wrapper_Drag_Event_Cb(lv_event_t *e);
static void             Resize_Handle_Event_Cb(lv_event_t *e);
static void             Delete_Handle_Event_Cb(lv_event_t *e);

static void Style_Handle(lv_obj_t *handle, lv_palette_t color)
{
    lv_obj_set_style_radius(handle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(handle, lv_palette_main(color), 0);
    lv_obj_set_style_bg_opa(handle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(handle, 2, 0);
    lv_obj_set_style_border_color(handle, lv_color_white(), 0);
    lv_obj_set_style_shadow_width(handle, 6, 0);
    lv_obj_set_style_shadow_opa(handle, LV_OPA_30, 0);
    lv_obj_set_style_pad_all(handle, 0, 0);
}

static void* Create_Widget(lv_obj_t *parent, int type, void* arg)
{
    switch(type) {
        case WIDGET_TYPE_CHART: {
            // 使用默认参数
            chart_dsc_t* dsc;
            if(arg == NULL)
            {
                dsc = Create_Chart(parent, &def_chart_param);
            }else{
                chart_param_dsc_t* c_param = (chart_param_dsc_t*)arg;
                dsc = Create_Chart(parent, c_param);
            }
            return dsc;
        }
        case WIDGET_TYPE_LED: {
            led_dsc_t *dsc;
            if (arg == NULL) {
                dsc = Create_Led(parent, &def_led_param);
            } else {
                dsc = Create_Led(parent, (led_param_dsc_t *)arg);
            }
            return dsc;
        }
        case WIDGET_TYPE_SLIDER:
            Create_Slider(parent, "名称");
            break;
        case WIDGET_TYPE_BUTTON: {
            btn_dsc_t *dsc;
            if(arg == NULL) {
                dsc = Create_Button_Widget(parent, &def_btn_param);
            } else {
                dsc = Create_Button_Widget(parent, (btn_param_dsc_t *)arg);
            }
            return dsc;
        }
        }

    return NULL;
}

static void Sync_Resize_Handle_Pos(Editor_Widget_t *ew, int32_t wx, int32_t wy, int32_t ww, int32_t wh)
{
    if(!ew || !ew->resize_handle || !ew->delete_handle) return;
    lv_obj_set_pos(ew->resize_handle,
                   wx + ww - HANDLE_SIZE / 2,
                   wy + wh - HANDLE_SIZE / 2);
    lv_obj_set_pos(ew->delete_handle,
                   wx + ww - 12,
                   wy - 12);
}

static void Update_Canvas_Size(void)
{
    if(!s_Editor.canvas || !s_Editor.activity_content) return;

    /* 确保布局已更新, 否则 content width/height 可能为 0 */
    lv_obj_update_layout(s_Editor.activity_content);

    int32_t max_r = 0, max_b = 0;
    uint32_t cnt = lv_obj_get_child_count(s_Editor.canvas);
    for(uint32_t i = 0; i < cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(s_Editor.canvas, i);
        if(!child) continue;
        int32_t r = lv_obj_get_x(child) + lv_obj_get_width(child);
        int32_t b = lv_obj_get_y(child) + lv_obj_get_height(child);
        if(r > max_r) max_r = r;
        if(b > max_b) max_b = b;
    }

    int32_t vis_w = lv_obj_get_content_width(s_Editor.activity_content);
    int32_t vis_h = lv_obj_get_content_height(s_Editor.activity_content);
    if(vis_w <= 0) vis_w = 800;
    if(vis_h <= 0) vis_h = 600;

    /* canvas 至少填满视口, 有内容时扩展到 内容+余量 */
    int32_t canvas_w = LV_MAX(vis_w, max_r + CANVAS_MARGIN);
    int32_t canvas_h = LV_MAX(vis_h, max_b + CANVAS_MARGIN);

    lv_obj_set_size(s_Editor.canvas, canvas_w, canvas_h);
}

static void Get_Ref_Size(int32_t *ref_w, int32_t *ref_h)
{
    lv_obj_update_layout(s_Editor.activity_content);
    *ref_w = lv_obj_get_content_width(s_Editor.activity_content);
    *ref_h = lv_obj_get_content_height(s_Editor.activity_content);
    if(*ref_w <= 0) *ref_w = 800;
    if(*ref_h <= 0) *ref_h = 600;
}

static void Pixels_To_Pct_Ex(Editor_Widget_t *ew, int32_t ref_w, int32_t ref_h)
{
    ew->pct_x = (float)lv_obj_get_x(ew->wrapper)     / (float)ref_w;
    ew->pct_y = (float)lv_obj_get_y(ew->wrapper)     / (float)ref_h;
    ew->pct_w = (float)lv_obj_get_width(ew->wrapper)  / (float)ref_w;
    ew->pct_h = (float)lv_obj_get_height(ew->wrapper) / (float)ref_h;
}

static void Pixels_To_Pct(Editor_Widget_t *ew)
{
    int32_t ref_w, ref_h;
    Get_Ref_Size(&ref_w, &ref_h);
    Pixels_To_Pct_Ex(ew, ref_w, ref_h);
}

static void Pct_To_Pixels_Ex(Editor_Widget_t *ew, int32_t ref_w, int32_t ref_h)
{
    int32_t px_w = (int32_t)(ew->pct_w * ref_w);
    int32_t px_h = (int32_t)(ew->pct_h * ref_h);

    int32_t min_w = MIN_WRAPPER_W, min_h = MIN_WRAPPER_H;
    const Palette_Item_t *pi = Find_Palette_Item(ew->widget_type);
    if(pi) { min_w = pi->default_w; min_h = pi->default_h; }

    if(px_w < min_w) px_w = min_w;
    if(px_h < min_h) px_h = min_h;

    int32_t px_x = (int32_t)(ew->pct_x * ref_w);
    int32_t px_y = (int32_t)(ew->pct_y * ref_h);
    if(px_x < 0) px_x = 0;
    if(px_y < 0) px_y = 0;

    lv_obj_set_pos(ew->wrapper, px_x, px_y);
    lv_obj_set_size(ew->wrapper, px_w, px_h);
    Sync_Resize_Handle_Pos(ew, px_x, px_y, px_w, px_h);
}

static void Pct_To_Pixels(Editor_Widget_t *ew)
{
    int32_t ref_w, ref_h;
    Get_Ref_Size(&ref_w, &ref_h);
    Pct_To_Pixels_Ex(ew, ref_w, ref_h);
}

static Editor_Widget_t* Create_Editor_Widget(lv_obj_t *canvas, int type, void* arg,
                                              int32_t x, int32_t y, int32_t w, int32_t h)
{
    if(!canvas) return NULL;

    Editor_Widget_t *ew = lv_malloc(sizeof(Editor_Widget_t));
    if(!ew) return NULL;
    lv_memzero(ew, sizeof(Editor_Widget_t));
    ew->widget_type = type;

    /* 查找默认尺寸 */
    int32_t size_w = MIN_WRAPPER_W, size_h = MIN_WRAPPER_H;
    if(w == -1 || h == -1)
    {
        const Palette_Item_t *pi = Find_Palette_Item(type);
        if(pi) { size_w = pi->default_w; size_h = pi->default_h; }
    }
    else{
        size_w = w;
        size_h = h;
    }

    ew->wrapper = lv_obj_create(canvas);
    if(!ew->wrapper) {
        lv_free(ew);
        return NULL;
    }

    lv_obj_set_size(ew->wrapper, size_w, size_h);
    int32_t pos_x = x - size_w / 2;
    int32_t pos_y = y - size_h / 2;
    if(pos_x < 0) pos_x = 0;
    if(pos_y < 0) pos_y = 0;
    lv_obj_set_pos(ew->wrapper, pos_x, pos_y);

    lv_obj_remove_flag(ew->wrapper, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(ew->wrapper, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_radius(ew->wrapper, 10, 0);
    lv_obj_set_style_pad_all(ew->wrapper, WRAPPER_PAD, 0);
    lv_obj_set_user_data(ew->wrapper, ew);
    lv_obj_set_style_bg_color(ew->wrapper, STYLE_BG_PANEL, LV_PART_MAIN);
    lv_obj_set_style_border_width(ew->wrapper, 1, 0);
    lv_obj_set_style_border_color(ew->wrapper, STYLE_BORDER, 0);
    lv_obj_set_style_border_opa(ew->wrapper, LV_OPA_50, 0);

    lv_obj_set_style_outline_width(ew->wrapper, 2, LV_STATE_PRESSED);
    lv_obj_set_style_outline_color(ew->wrapper, STYLE_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(ew->wrapper, LV_OPA_COVER, LV_STATE_PRESSED);

    /* wrapper 的 padding 区域可拖拽 (点击 padding 区触发, 子控件区不触发) */
    lv_obj_add_flag(ew->wrapper, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
    
    lv_obj_add_event_cb(ew->wrapper, Wrapper_Drag_Event_Cb,
                        LV_EVENT_PRESSED, ew);
    lv_obj_add_event_cb(ew->wrapper, Wrapper_Drag_Event_Cb,
                        LV_EVENT_PRESSING, ew);
    lv_obj_add_event_cb(ew->wrapper, Wrapper_Drag_Event_Cb,
                    LV_EVENT_RELEASED, ew);

    ew->widget_dsc_t = Create_Widget(ew->wrapper, type, arg);

    // TODO 添加组件数据
    ew->resize_handle = lv_obj_create(canvas);
    if(ew->resize_handle) {
        lv_obj_set_size(ew->resize_handle, HANDLE_SIZE, HANDLE_SIZE);
        Style_Handle(ew->resize_handle, LV_PALETTE_GREEN);
        lv_obj_add_flag(ew->resize_handle,
                        LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_remove_flag(ew->resize_handle, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(ew->resize_handle, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_add_event_cb(ew->resize_handle, Resize_Handle_Event_Cb,
                            LV_EVENT_PRESSED, ew);
        lv_obj_add_event_cb(ew->resize_handle, Resize_Handle_Event_Cb,
                            LV_EVENT_PRESSING, ew);
        lv_obj_add_event_cb(ew->resize_handle, Resize_Handle_Event_Cb,
                    LV_EVENT_RELEASED, ew);
        lv_obj_update_layout(ew->wrapper);
        Sync_Resize_Handle_Pos(ew,
            lv_obj_get_x(ew->wrapper), lv_obj_get_y(ew->wrapper),
            lv_obj_get_width(ew->wrapper), lv_obj_get_height(ew->wrapper));
    }

    ew->delete_handle = lv_obj_create(canvas);
    if(ew->delete_handle) {
        lv_obj_set_style_bg_image_src(ew->delete_handle, g_smp_ctx->imgs[IMG_DASHBOARD_DELETE], LV_PART_MAIN);
        lv_obj_set_size(ew->delete_handle, 24, 24);
        lv_obj_set_style_bg_opa(ew->delete_handle, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(ew->delete_handle,
                        LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_remove_flag(ew->delete_handle, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(ew->delete_handle, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

        lv_obj_add_event_cb(ew->delete_handle, Delete_Handle_Event_Cb,
                            LV_EVENT_CLICKED, ew);
        lv_obj_update_layout(ew->wrapper);
        Sync_Resize_Handle_Pos(ew,
            lv_obj_get_x(ew->wrapper), lv_obj_get_y(ew->wrapper),
            lv_obj_get_width(ew->wrapper), lv_obj_get_height(ew->wrapper));
    }

    Update_Canvas_Size();

    clist_ewt_push_back(ew_list, ew);
    Pixels_To_Pct(ew);
    return ew;
}

static void Widgets_Load_All(void)
{
    FILE *fp = fopen(WIDGET_JSON_PATH, "r");
    if (!fp) return;                       // 首次运行无配置文件，跳过

    fseek(fp, 0, SEEK_END);
    long flen = ftell(fp);
    rewind(fp);

    if (flen <= 0) { fclose(fp); return; }

    char *buf = lv_malloc(flen + 1);
    if (!buf) { fclose(fp); return; }
    fread(buf, 1, flen, fp);
    buf[flen] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buf);
    lv_free(buf);

    if (!root) {
        Create_Tips_Box("布局文件解析失败", 0xff0000);
        return;
    }

    cJSON *arr = cJSON_GetObjectItem(root, "widgets");
    if (!arr || !cJSON_IsArray(arr)) {
        cJSON_Delete(root);
        return;
    }

    cJSON *jver = cJSON_GetObjectItem(root, "version");
    int version = (jver && cJSON_IsNumber(jver)) ? jver->valueint : 1;

    int32_t ref_w, ref_h;
    Get_Ref_Size(&ref_w, &ref_h);

    int count = cJSON_GetArraySize(arr);

    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        if (!item) continue;

        cJSON *jtype = cJSON_GetObjectItem(item, "type");
        if (!jtype) {
            Create_Tips_Box("控件缺少必要属性, 已跳过", 0xffff00);
            continue;
        }

        int type = jtype->valueint;
        if (type < 0 || type >= WIDGET_TYPE_COUNT) {
            Create_Tips_Box("未知控件类型, 已跳过", 0xffff00);
            continue;
        }

        float pct_x, pct_y, pct_w, pct_h;

        if (version >= 2) {
            cJSON *jpx = cJSON_GetObjectItem(item, "pct_x");
            cJSON *jpy = cJSON_GetObjectItem(item, "pct_y");
            cJSON *jpw = cJSON_GetObjectItem(item, "pct_w");
            cJSON *jph = cJSON_GetObjectItem(item, "pct_h");
            if (!jpx || !jpy || !jpw || !jph) {
                Create_Tips_Box("控件缺少必要属性, 已跳过", 0xffff00);
                continue;
            }
            pct_x = (float)jpx->valuedouble;
            pct_y = (float)jpy->valuedouble;
            pct_w = (float)jpw->valuedouble;
            pct_h = (float)jph->valuedouble;
        } else {
            /* v1 旧格式: 绝对像素 → 转百分比 */
            cJSON *jx = cJSON_GetObjectItem(item, "x");
            cJSON *jy = cJSON_GetObjectItem(item, "y");
            cJSON *jw = cJSON_GetObjectItem(item, "w");
            cJSON *jh = cJSON_GetObjectItem(item, "h");
            if (!jx || !jy || !jw || !jh) {
                Create_Tips_Box("控件缺少必要属性, 已跳过", 0xffff00);
                continue;
            }
            pct_x = (float)jx->valueint / (float)ref_w;
            pct_y = (float)jy->valueint / (float)ref_h;
            pct_w = (float)jw->valueint / (float)ref_w;
            pct_h = (float)jh->valueint / (float)ref_h;
        }

        /* 百分比 → 像素，enforce 最小尺寸 */
        int32_t px_w = (int32_t)(pct_w * ref_w);
        int32_t px_h = (int32_t)(pct_h * ref_h);
        int32_t min_w = MIN_WRAPPER_W, min_h = MIN_WRAPPER_H;
        const Palette_Item_t *pi = Find_Palette_Item(type);
        if (pi) { min_w = pi->default_w; min_h = pi->default_h; }
        if (px_w < min_w) px_w = min_w;
        if (px_h < min_h) px_h = min_h;

        int32_t px_x = (int32_t)(pct_x * ref_w);
        int32_t px_y = (int32_t)(pct_y * ref_h);
        if (px_x < 0) px_x = 0;
        if (px_y < 0) px_y = 0;

        const Widget_Ops_t *ops = &s_Widget_Ops[type];
        void *param = NULL;

        if (ops->default_param && ops->param_size > 0) {
            param = lv_malloc(ops->param_size);
            if (!param) continue;
            memcpy(param, ops->default_param, ops->param_size);

            if (ops->load) {
                ops->load(param, item);
            }
        }

        Editor_Widget_t *ew = Create_Editor_Widget(
            s_Editor.canvas, type, param,
            px_x + px_w / 2,
            px_y + px_h / 2,
            px_w, px_h
        );

        if (!ew) {
            char tip[64];
            snprintf(tip, sizeof(tip), "第 %d 个控件创建失败", i + 1);
            Create_Tips_Box(tip, 0xffff00);
        } else {
            /* 覆盖 pct 字段为文件中的值 */
            ew->pct_x = pct_x;
            ew->pct_y = pct_y;
            ew->pct_w = pct_w;
            ew->pct_h = pct_h;
        }

        if (param) lv_free(param);
    }

    cJSON_Delete(root);
}

static void Wrapper_Drag_Event_Cb(lv_event_t *e)
{
    Editor_Widget_t *ew = (Editor_Widget_t *)lv_event_get_user_data(e);
    if(!ew || !ew->wrapper || !ew->resize_handle) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_active();
    if(!indev) return;

    lv_point_t pt;
    lv_indev_get_point(indev, &pt);

    if(code == LV_EVENT_PRESSED) {
        /* 记录鼠标按下点 与 wrapper 左上角 的屏幕距离 */
        lv_obj_move_foreground(ew->wrapper);
        lv_obj_move_foreground(ew->resize_handle);
        lv_obj_move_foreground(ew->delete_handle);

        lv_obj_remove_flag(s_Editor.activity_content, LV_OBJ_FLAG_SCROLLABLE);
        lv_area_t wrap_area;
        lv_obj_get_coords(ew->wrapper, &wrap_area);
        s_Drag_Offset.x = pt.x - wrap_area.x1;
        s_Drag_Offset.y = pt.y - wrap_area.y1;
    }
    else if(code == LV_EVENT_PRESSING) {
        if(!s_Editor.canvas) return;

        /*
         * canvas 的屏幕坐标 (已自动包含 activity_content 的 scroll 偏移)
         * 无论 activity_content 怎么滚动, 这个值都是 canvas 当前帧的实时位置
         */
        lv_area_t canvas_area;
        lv_obj_get_coords(s_Editor.canvas, &canvas_area);


        int32_t new_x = (pt.x - s_Drag_Offset.x) - canvas_area.x1;
        int32_t new_y = (pt.y - s_Drag_Offset.y) - canvas_area.y1;

        if(new_x < 0) new_x = 0;
        if(new_y < 0) new_y = 0;

        lv_obj_set_pos(ew->wrapper, new_x, new_y);
        Sync_Resize_Handle_Pos(ew,
            new_x, new_y,
            lv_obj_get_width(ew->wrapper), lv_obj_get_height(ew->wrapper));
        Update_Canvas_Size();
    }else if(code == LV_EVENT_RELEASED)
    {
        lv_obj_add_flag(s_Editor.activity_content, LV_OBJ_FLAG_SCROLLABLE);
        Pixels_To_Pct(ew);
    }
}


static void Delete_Handle_Event_Cb(lv_event_t *e)
{
    Editor_Widget_t *ew = (Editor_Widget_t *)lv_event_get_user_data(e);
    if(!ew || !ew->wrapper || !ew->resize_handle || !ew->delete_handle) return;

    clist_ewt_node *iter = clist_ewt_find(ew_list, ew);
    if(iter)
        clist_ewt_erase(ew_list, iter);

    ew->wrapper->user_data = NULL;

    lv_obj_delete(ew->resize_handle);
    lv_obj_delete(ew->delete_handle);
    lv_obj_delete(ew->wrapper);

    lv_free(ew);
}

static void Resize_Handle_Event_Cb(lv_event_t *e)
{
    Editor_Widget_t *ew = (Editor_Widget_t *)lv_event_get_user_data(e);
    if(!ew || !ew->wrapper) return;

    lv_event_code_t code = lv_event_get_code(e);
    static int32_t s_Min_W = MIN_WRAPPER_W;
    static int32_t s_Min_H = MIN_WRAPPER_H;

    lv_indev_t *indev = lv_indev_active();
    if(!indev) return;
    lv_point_t pt;
    lv_indev_get_point(indev, &pt);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_remove_flag(s_Editor.activity_content, LV_OBJ_FLAG_SCROLLABLE);

        s_Press_Pt = pt;
        s_Orig_W = lv_obj_get_width(ew->wrapper);
        s_Orig_H = lv_obj_get_height(ew->wrapper);

        /* 查默认尺寸作为缩放下限 */
        s_Min_W = MIN_WRAPPER_W;
        s_Min_H = MIN_WRAPPER_H;
        const Palette_Item_t *pi = Find_Palette_Item(ew->widget_type);
        if(pi) { s_Min_W = pi->default_w; s_Min_H = pi->default_h; }
    }
    else if(code == LV_EVENT_PRESSING) {
        int32_t new_w = s_Orig_W + (pt.x - s_Press_Pt.x);
        int32_t new_h = s_Orig_H + (pt.y - s_Press_Pt.y);

        if(new_w < s_Min_W) new_w = s_Min_W;
        if(new_h < s_Min_H) new_h = s_Min_H;

        lv_obj_set_size(ew->wrapper, new_w, new_h);
        Sync_Resize_Handle_Pos(ew,
            lv_obj_get_x(ew->wrapper), lv_obj_get_y(ew->wrapper),
            new_w, new_h);
        Update_Canvas_Size();
    }else if(code == LV_EVENT_RELEASED)
    {
        lv_obj_add_flag(s_Editor.activity_content, LV_OBJ_FLAG_SCROLLABLE);
        Pixels_To_Pct(ew);
    }
}


static void Palette_Event_Cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int type = (int)(intptr_t)lv_event_get_user_data(e);

    lv_indev_t *indev = lv_indev_active();
    if(!indev) return;
    lv_point_t pt;
    lv_indev_get_point(indev, &pt);

    if(code == LV_EVENT_PRESSED) {
        s_Editor.widget_type = type;

        s_Editor.ghost = lv_obj_create(lv_layer_top());
        if(!s_Editor.ghost) return;
        lv_obj_set_size(s_Editor.ghost, GHOST_W, GHOST_H);
        lv_obj_set_pos(s_Editor.ghost, pt.x - GHOST_W / 2, pt.y - GHOST_H / 2);
        lv_obj_set_style_bg_opa(s_Editor.ghost, LV_OPA_TRANSP, 0);

        if(g_smp_ctx->imgs[IMG_WIDGET_CHART + type])
            lv_obj_set_style_bg_image_src(s_Editor.ghost,
                g_smp_ctx->imgs[IMG_WIDGET_CHART + type], LV_PART_MAIN);

        lv_obj_remove_flag(s_Editor.ghost, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(s_Editor.ghost, LV_OBJ_FLAG_SCROLLABLE);
    }
    else if(code == LV_EVENT_PRESSING) {
        if(s_Editor.ghost) {
            lv_obj_set_pos(s_Editor.ghost,
                           pt.x - GHOST_W / 2, pt.y - GHOST_H / 2);
        }
    }
    else if(code == LV_EVENT_RELEASED) {
        if(!s_Editor.ghost) return;

        lv_area_t view_area;
        lv_obj_get_coords(s_Editor.activity_content, &view_area);

        if(s_Editor.canvas &&
           pt.x >= view_area.x1 && pt.x <= view_area.x2 &&
           pt.y >= view_area.y1 && pt.y <= view_area.y2)
        {
            /*
             * 屏幕坐标 → canvas 本地坐标
             * canvas 的屏幕位置已包含 scroll 偏移, 不需要手动加 scroll
             */
            lv_area_t canvas_area;
            lv_obj_get_coords(s_Editor.canvas, &canvas_area);

            int32_t local_x = pt.x - canvas_area.x1;
            int32_t local_y = pt.y - canvas_area.y1;

            Create_Editor_Widget(s_Editor.canvas,
                                 s_Editor.widget_type, NULL, local_x, local_y, -1, -1);
        }

        lv_obj_delete(s_Editor.ghost);
        s_Editor.ghost = NULL;
    }
}

/* 创建分组标题 */
static void Create_Section_Label(lv_obj_t *parent, const char *text)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, STYLE_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_pad_top(lbl, 4, 0);
}

/* 创建单个控件项 */
static void Create_Palette_Item(lv_obj_t *menu, const Palette_Item_t *pi)
{
    lv_obj_t *item = lv_obj_create(menu);
    lv_obj_set_size(item, lv_pct(100), 45);
    lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(item, 6, 0);
    lv_obj_add_flag(item, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_remove_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(item, 5, 0);
    lv_obj_set_style_bg_opa(item, 15, 0);
    lv_obj_set_style_bg_color(item, STYLE_BG_HEADER, 0);
    lv_obj_set_style_radius(item, 8, 0);

    lv_obj_set_style_bg_opa(item, 60, LV_STATE_PRESSED);

    lv_obj_t *img = lv_obj_create(item);
    lv_obj_set_size(img, 32, 32);
    lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 0);
    if(g_smp_ctx->imgs[IMG_WIDGET_CHART + pi->type])
        lv_obj_set_style_bg_image_src(img, g_smp_ctx->imgs[IMG_WIDGET_CHART + pi->type], LV_PART_MAIN);
    lv_obj_add_flag(img, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t *spander = lv_obj_create(item);
    lv_obj_set_style_bg_opa(spander, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_grow(spander, 1);
    lv_obj_add_flag(spander, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t *lbl = lv_label_create(item);
    lv_obj_set_style_text_color(lbl, STYLE_TEXT_PRIMARY, 0);
    lv_label_set_text(lbl, pi->name);

    lv_obj_t *sep = lv_obj_create(menu);
    lv_obj_set_size(sep, lv_pct(100), 1);
    lv_obj_set_style_bg_color(sep, STYLE_BORDER, 0);
    lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);

    void *ud = (void*)(intptr_t)pi->type;
    lv_obj_add_event_cb(item, Palette_Event_Cb, LV_EVENT_PRESSED,  ud);
    lv_obj_add_event_cb(item, Palette_Event_Cb, LV_EVENT_PRESSING, ud);
    lv_obj_add_event_cb(item, Palette_Event_Cb, LV_EVENT_RELEASED, ud);
}

/* ── 自动排列：按类型升序比较 ── */
static int Ew_Type_Cmp_Asc(const ewt *a, const ewt *b)
{
    if((*a)->widget_type < (*b)->widget_type) return -1;
    if((*a)->widget_type > (*b)->widget_type) return  1;
    return 0;
}

static void Auto_Arrange_Widgets(int rows, int cols, int gap)
{
    if(rows <= 0 || cols <= 0) return;
    if(!s_Editor.canvas || !s_Editor.activity_content) return;
    if(clist_ewt_size(ew_list) == 0) return;

    clist_ewt_sort_with_cmp(ew_list, Ew_Type_Cmp_Asc);

    lv_obj_update_layout(s_Editor.activity_content);
    int32_t canvas_w = lv_obj_get_content_width(s_Editor.activity_content);
    int32_t canvas_h = lv_obj_get_content_height(s_Editor.activity_content);
    if(canvas_w <= 0) canvas_w = 800;
    if(canvas_h <= 0) canvas_h = 600;

    // 图表网格布局 
    int32_t cell_w = (canvas_w - (cols + 1) * gap) / cols;
    int32_t cell_h = (canvas_h - (rows + 1) * gap) / rows;

    const Palette_Item_t *chart_pi = Find_Palette_Item(WIDGET_TYPE_CHART);
    int32_t chart_min_w = chart_pi ? chart_pi->default_w : MIN_WRAPPER_W;
    int32_t chart_min_h = chart_pi ? chart_pi->default_h : MIN_WRAPPER_H;
    if(cell_w < chart_min_w) cell_w = chart_min_w;
    if(cell_h < chart_min_h) cell_h = chart_min_h;

    int chart_idx = 0;
    int chart_total = rows * cols;
    int32_t chart_bottom = 0;       /* 记录图表区域最下沿 */

    clist_foreach(ew_list, ewt, ew) {
        if(ew->widget_type != WIDGET_TYPE_CHART) continue;
        if(chart_idx >= chart_total) break;

        int r = chart_idx / cols;
        int c = chart_idx % cols;
        int32_t px_x = gap + c * (cell_w + gap);
        int32_t px_y = gap + r * (cell_h + gap);

        lv_obj_set_pos(ew->wrapper, px_x, px_y);
        lv_obj_set_size(ew->wrapper, cell_w, cell_h);
        Sync_Resize_Handle_Pos(ew, px_x, px_y, cell_w, cell_h);
        Pixels_To_Pct(ew);

        int32_t bot = px_y + cell_h;
        if(bot > chart_bottom) chart_bottom = bot;
        chart_idx++;
    }

    // 非图表控件保持原始尺寸，流式排列在图表下方
    int32_t cursor_x = gap;
    int32_t cursor_y = chart_bottom + gap;
    int32_t row_h = 0;          

    clist_foreach(ew_list, ewt, ew) {
        if(ew->widget_type == WIDGET_TYPE_CHART) continue;

        int32_t w = lv_obj_get_width(ew->wrapper);
        int32_t h = lv_obj_get_height(ew->wrapper);

        /* 超出行宽则换行 */
        if(cursor_x + w + gap > canvas_w) {
            cursor_x  = gap;
            cursor_y += row_h + gap;
            row_h     = 0;
        }

        lv_obj_set_pos(ew->wrapper, cursor_x, cursor_y);
        Sync_Resize_Handle_Pos(ew, cursor_x, cursor_y, w, h);
        Pixels_To_Pct(ew);

        cursor_x += w + gap;
        if(h > row_h) row_h = h;
    }

    Update_Canvas_Size();
}

static void Arrange_Btn_Cb(lv_event_t *e)
{
    (void)e;
    Arrange_Dialog_Open(Auto_Arrange_Widgets);
}

static void Create_Palette(lv_obj_t *menu)
{
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(menu, 8, 0);
    lv_obj_set_style_pad_row(menu, 6, 0);

    /* ── palette 容器 (可滚动，占据 menu 剩余空间) ── */
    lv_obj_t *palette_container = lv_obj_create(menu);
    lv_obj_set_width(palette_container, lv_pct(100));
    lv_obj_set_flex_grow(palette_container, 1);
    lv_obj_set_flex_flow(palette_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(palette_container, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(palette_container, 0, 0);
    lv_obj_set_style_pad_row(palette_container, 6, 0);
    lv_obj_set_style_bg_opa(palette_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(palette_container, 0, 0);
    lv_obj_add_flag(palette_container, LV_OBJ_FLAG_SCROLLABLE);

    /* ── 数据监控 ── */
    Create_Section_Label(palette_container, "数据监控");
    int mon_cnt = sizeof(s_Monitor_Items) / sizeof(s_Monitor_Items[0]);
    for(int i = 0; i < mon_cnt; i++)
        Create_Palette_Item(palette_container, &s_Monitor_Items[i]);

    /* ── 可操作 ── */
    Create_Section_Label(palette_container, "可操作");
    int op_cnt = sizeof(s_Operate_Items) / sizeof(s_Operate_Items[0]);
    for(int i = 0; i < op_cnt; i++)
        Create_Palette_Item(palette_container, &s_Operate_Items[i]);

    /* ── 自动排列按钮 ── */
    lv_obj_t *btn_arrange = lv_button_create(menu);
    lv_obj_set_size(btn_arrange, lv_pct(100), 40);
    lv_obj_set_style_bg_color(btn_arrange, lv_color_hex(0x3A4260), 0);
    lv_obj_set_style_bg_color(btn_arrange,  STYLE_BTN_PRIMARY, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_arrange, 8, 0);
    lv_obj_set_flex_flow(btn_arrange, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_arrange, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_arrange, 6, 0);

    lv_obj_t *icon = lv_image_create(btn_arrange);
    lv_image_set_src(icon, g_smp_ctx->imgs[IMG_ARRANGEMENT]);

    lv_obj_t *lbl = lv_label_create(btn_arrange);
    lv_label_set_text(lbl, "自动排列");
    lv_obj_set_style_text_color(lbl, STYLE_TEXT_WHITE, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_add_event_cb(btn_arrange, Arrange_Btn_Cb, LV_EVENT_CLICKED, NULL);
}

static void Activity_Content_Size_Changed_Cb(lv_event_t *e)
{
    (void)e;

    lv_obj_update_layout(s_Editor.activity_content);
    int32_t ref_w = lv_obj_get_content_width(s_Editor.activity_content);
    int32_t ref_h = lv_obj_get_content_height(s_Editor.activity_content);
    if(ref_w <= 0) ref_w = 800;
    if(ref_h <= 0) ref_h = 600;

    /*   用 pct 值直接计算控件边界，避免读取尚未生效的 lv_obj 坐标，特别是在响应式布局的时候该函数会被调用两次
     * 使用lv_obj_get_xx等函数读到的往往是旧值。
     */
    int32_t max_r = 0, max_b = 0;
    clist_foreach(ew_list, ewt, ew) {
        Pct_To_Pixels_Ex(ew, ref_w, ref_h);
        int32_t r = (int32_t)(ew->pct_x * ref_w + ew->pct_w * ref_w);
        int32_t b = (int32_t)(ew->pct_y * ref_h + ew->pct_h * ref_h);
        if(r > max_r) max_r = r;
        if(b > max_b) max_b = b;
    }

    int32_t canvas_w = LV_MAX(ref_w, max_r + CANVAS_MARGIN);
    int32_t canvas_h = LV_MAX(ref_h, max_b + CANVAS_MARGIN);
    lv_obj_set_size(s_Editor.canvas, canvas_w, canvas_h);
}

static void Dashboard_Delete(lv_event_t* e)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "version", 2);
    cJSON *arr  = cJSON_AddArrayToObject(root, "widgets");

    clist_foreach(ew_list, ewt, ew){
        cJSON *item = cJSON_CreateObject();

        cJSON_AddNumberToObject(item, "type", ew->widget_type);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.6f", ew->pct_x);
        cJSON_AddRawToObject(item, "pct_x", buf);
        snprintf(buf, sizeof(buf), "%.6f", ew->pct_y);
        cJSON_AddRawToObject(item, "pct_y", buf);
        snprintf(buf, sizeof(buf), "%.6f", ew->pct_w);
        cJSON_AddRawToObject(item, "pct_w", buf);
        snprintf(buf, sizeof(buf), "%.6f", ew->pct_h);
        cJSON_AddRawToObject(item, "pct_h", buf);

        if (s_Widget_Ops[ew->widget_type].save && ew->widget_dsc_t) {
            s_Widget_Ops[ew->widget_type].save(item , ew->widget_dsc_t);
        }

        cJSON_AddItemToArray(arr, item);
    }

    char *str = cJSON_PrintUnformatted(root);
    FILE *fp = fopen(WIDGET_JSON_PATH, "w");
    if (fp) { fputs(str, fp); fclose(fp); }

    cJSON_free(str);
    cJSON_Delete(root);

    if(ew_list != NULL)
        clist_ewt_destroy(&ew_list);
}


void Dashboard_Weight(smp_ctx_t *ctx)
{
    lv_obj_t *activity_content = ctx->pages[NAV_PAGE_DASHBOARD]->activity_content;
    lv_obj_t *menu             = ctx->pages[NAV_PAGE_DASHBOARD]->menu;

    lv_obj_set_style_pad_all(activity_content, 0, 0);
    lv_obj_set_style_bg_color(activity_content, STYLE_BG_DARK, 0);

    s_Editor.activity_content = activity_content;
    s_Editor.ghost            = NULL;
    s_Editor.canvas           = NULL;

    s_Editor.canvas = lv_obj_create(activity_content);
    if(!s_Editor.canvas) return;

    lv_obj_set_pos(s_Editor.canvas, 0, 0);
    lv_obj_remove_flag(s_Editor.canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(s_Editor.canvas, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_bg_opa(s_Editor.canvas, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_Editor.canvas, 0, 0);
    lv_obj_set_style_pad_all(s_Editor.canvas, 0, 0);
    lv_obj_set_style_radius(s_Editor.canvas, 0, 0);

    lv_obj_update_layout(activity_content);
    int32_t init_w = lv_obj_get_content_width(activity_content);
    int32_t init_h = lv_obj_get_content_height(activity_content);
    lv_obj_set_size(s_Editor.canvas, init_w > 0 ? init_w : 800,
                                      init_h > 0 ? init_h : 600);

    Create_Palette(menu);
    
    // 初始化widget链表
    ew_list = clist_ewt_create();
    lv_obj_add_event_cb(s_Editor.canvas, Dashboard_Delete, LV_EVENT_DELETE, NULL);
    lv_obj_add_event_cb(activity_content, Activity_Content_Size_Changed_Cb, LV_EVENT_SIZE_CHANGED, NULL);

    Widgets_Load_All();
}

int Dashboard_Get_Chart_Snapshots(chart_snapshot_t *snapshots, int max_count)
{
    if (!snapshots || max_count <= 0 || !ew_list) return 0;

    int count = 0;
    clist_foreach(ew_list, ewt, ew) {
        if (count >= max_count) break;
        if (ew->widget_type != WIDGET_TYPE_CHART) continue;
        if (!ew->widget_dsc_t) continue;

        chart_dsc_t *c = (chart_dsc_t *)ew->widget_dsc_t;
        if (!c->buffer[c->active_buffer]) continue;

        snapshots[count].buffer = c->buffer[c->active_buffer];
        snapshots[count].width  = c->current_width;
        snapshots[count].height = c->current_height;
        count++;
    }
    return count;
}
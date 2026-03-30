#include "modules/include/window_module.h"
#include "smp_private.h"
#include "SDL2/SDL.h"

static lv_obj_t *g_window = NULL;


static void close_window_cb(lv_event_t *e)
{
    if (g_window) 
    {
        lv_obj_del(g_window);
        g_window = NULL;
    }
}
//窗口拖动函数 
/*
static void window_drag_cb(lv_event_t *e)
{
    static bool dragging = false;
    static lv_point_t mouse_offset;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);     
    lv_obj_t *current = lv_event_get_current_target(e);

    if (target != current) return;

    lv_obj_t *window = lv_obj_get_parent(current);
    if (!window) return;

    if (code == LV_EVENT_PRESSED)
    {
        lv_point_t mouse_pos;
        lv_indev_get_point(lv_indev_get_act(), &mouse_pos);

        lv_area_t window_coords;
        lv_obj_get_coords(window, &window_coords);

        mouse_offset.x = mouse_pos.x - window_coords.x1;
        mouse_offset.y = mouse_pos.y - window_coords.y1;

        dragging = true;
    }
    else if (code == LV_EVENT_PRESSING && dragging)
    {
        lv_point_t mouse_pos;
        lv_indev_get_point(lv_indev_get_act(), &mouse_pos);

        int32_t new_x = mouse_pos.x - mouse_offset.x;
        int32_t new_y = mouse_pos.y - mouse_offset.y;

        lv_obj_set_pos(window, new_x, new_y);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        dragging = false;
    }
}
*/

void Window_Module_Init(void)
{
    // 初始化窗口模块
    g_window = NULL;
}

void Window_Module_Cleanup(void)
{
    // 清理窗口模块资源
    if (g_window) 
    {
        lv_obj_del(g_window);
        g_window = NULL;
    }
}

void Window_Module_SetTitle(const char *title)
{
    if (!g_window) return;
    
    // 查找标题标签
    lv_obj_t *title_bar = lv_obj_get_child(g_window, 0);
    if (!title_bar) return;
    
    lv_obj_t *title_label = lv_obj_get_child(title_bar, 0);
    if (!title_label) return;
    
    lv_label_set_text(title_label, title);
}

void Window_Module_SetContent(lv_obj_t *content)
{
    if (!g_window || !content) return;
    
    // 查找内容区域
    lv_obj_t *content_area = lv_obj_get_child(g_window, 1);
    if (!content_area) return;
    
    // 清除现有内容
    lv_obj_clean(content_area);
    
    // 设置内容的父对象为内容区域
    lv_obj_set_parent(content, content_area);
}

void Window_Module_SetStyle(lv_style_t *style)
{
    if (!g_window || !style) return;
    lv_obj_add_style(g_window, style, 0);
}

void Window_Module_SetTitleBarStyle(lv_style_t *style)
{
    if (!g_window || !style) return;
    
    lv_obj_t *title_bar = lv_obj_get_child(g_window, 0);
    if (!title_bar) return;
    
    lv_obj_add_style(title_bar, style, 0);
}

void Window_Module_SetContentStyle(lv_style_t *style)
{
    if (!g_window || !style) return;
    
    lv_obj_t *content_area = lv_obj_get_child(g_window, 1);
    if (!content_area) return;
    
    lv_obj_add_style(content_area, style, 0);
}

void Window_Module_Weight(smp_ctx_t *ctx)
{
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    // 如果窗口已存在，先关闭
    if (g_window) 
    {
        lv_obj_del(g_window);
        g_window = NULL;
    }
    
    // 获取屏幕尺寸
    lv_coord_t screen_w = lv_obj_get_width(lv_scr_act());
    lv_coord_t screen_h = lv_obj_get_height(lv_scr_act());
    
    // 计算窗口尺寸（根据屏幕尺寸的比例）
    lv_coord_t win_w = screen_w * 0.5; // 屏幕宽度的50%
    lv_coord_t win_h = screen_h * 0.4; // 屏幕高度的40%
    
    // 确保窗口尺寸在合理范围内
    if (win_w < 300) win_w = 300;
    if (win_h < 200) win_h = 200;
    if (win_w > screen_w - 20) win_w = screen_w - 20;
    if (win_h > screen_h - 20) win_h = screen_h - 20;
    
    // 创建独立窗口
    g_window = lv_obj_create(lv_layer_top());
    if (!g_window) return; // 错误处理
    
    lv_obj_set_size(g_window, win_w, win_h);
    lv_obj_center(g_window);
    lv_obj_set_align(g_window, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(g_window, lv_color_hex(0x2a2b2c), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(g_window, lv_color_hex(0x444444), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_window, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_window, 10, LV_STATE_DEFAULT);
    lv_obj_clear_flag(g_window, LV_OBJ_FLAG_SCROLLABLE);  

    
    // 创建标题栏
    lv_obj_t *title_bar = lv_obj_create(g_window);
    if (!title_bar) { lv_obj_delete(g_window); g_window = NULL; return; }
    
    lv_obj_set_size(title_bar, lv_pct(100), 50);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x121212), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(title_bar, lv_color_hex(0x2c2c2c), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(title_bar, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(title_bar, LV_BORDER_SIDE_BOTTOM, LV_STATE_DEFAULT);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(title_bar, LV_OBJ_FLAG_CLICKABLE);
    // 拖动事件注册
    //lv_obj_add_event_cb(title_bar, window_drag_cb, LV_EVENT_ALL, g_window);
    //lv_obj_add_event_cb(title_bar, window_drag_cb, LV_EVENT_PRESSED, NULL);
    //lv_obj_add_event_cb(title_bar, window_drag_cb, LV_EVENT_PRESSING, NULL);
    //lv_obj_add_event_cb(title_bar, window_drag_cb, LV_EVENT_RELEASED, NULL);
    //lv_obj_add_event_cb(title_bar, window_drag_cb, LV_EVENT_PRESS_LOST, NULL);  
    // 创建标题
    lv_obj_t *title = lv_label_create(title_bar);
    if (!title) { lv_obj_delete(g_window); g_window = NULL; return; }
    
    lv_label_set_text(title, "窗口");
    lv_obj_set_style_text_color(title, lv_color_hex(0x3A7AFE), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);
    
    // 创建关闭按钮
    lv_obj_t *close_btn = lv_btn_create(title_bar);
    if (!close_btn) { lv_obj_delete(g_window); g_window = NULL; return; }
    
    lv_obj_set_size(close_btn, 30, 30);
    lv_obj_align(close_btn, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x1a1b1c), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x1a1b1c), LV_STATE_PRESSED);
    lv_obj_set_style_bg_image_src(close_btn, ctx->imgs[IMG_CONTRL_BTN_CLOSE_SELECTED], LV_STATE_DEFAULT);
    lv_obj_add_event_cb(close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
   
    // 创建内容区域
    lv_obj_t *content = lv_obj_create(g_window);
    if (!content) { lv_obj_delete(g_window); g_window = NULL; return; }
    
    lv_obj_set_size(content, lv_pct(100), win_h - 50);
    lv_obj_set_y(content, 50);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x1a1b1c), LV_STATE_DEFAULT);
    
    // 创建确定按钮
    lv_obj_t *btn = lv_btn_create(content);
    if (!btn) { lv_obj_delete(g_window); g_window = NULL; return; }
    
    lv_obj_set_size(btn, 80, 36);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3A7AFE), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn, 4, LV_STATE_DEFAULT);
    
    lv_obj_t *btn_label = lv_label_create(btn);
    if (!btn_label) { lv_obj_delete(g_window); g_window = NULL; return; }
    
    lv_label_set_text(btn_label, "确定");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(btn_label, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(btn_label);  
}                                                                                                       

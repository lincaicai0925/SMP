#include "main_window.h"
#include "smp_private.h"
#include "pages/include/log_manage.h"
#include "pages/include/dashboard.h"
#include "pages/include/data_dictionary.h"
#include "pages/include/experimental_data.h"
#include "pages/include/topology_display.h"
#include "data_distribution.h"
#include <stdio.h>

smp_ctx_t * g_smp_ctx = NULL;
static const struct{
    char *name;
    lv_color_format_t color_format;
}image_details[IMG_CNT] = {
    {"navigation_dashboard", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_dashboard_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_dashboard_dark", LV_COLOR_FORMAT_ARGB8888},

    {"navigation_data_dictionary", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_data_dictionary_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_data_dictionary_dark", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_topology_display", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_topology_display_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_topology_display_dark", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_experimental_data", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_experimental_data_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_experimental_data_dark", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_log_management", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_log_management_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_log_management_dark", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_about", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_about_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_about_dark", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_setting", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_setting_selected", LV_COLOR_FORMAT_ARGB8888},
    {"navigation_setting_dark", LV_COLOR_FORMAT_ARGB8888},

    {"contrl_btn_small", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_small_selected", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_small_dark", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_min", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_min_selected", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_min_dark", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_max", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_max_selected", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_max_dark", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_close", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_close_selected", LV_COLOR_FORMAT_ARGB8888},
    {"contrl_btn_close_dark", LV_COLOR_FORMAT_ARGB8888},
    
    {"logo", LV_COLOR_FORMAT_ARGB8888},
    {"dialog_bg", LV_COLOR_FORMAT_ARGB8888},

    {"dict_arrow_up", LV_COLOR_FORMAT_ARGB8888},
    {"dict_arrow_top", LV_COLOR_FORMAT_ARGB8888},
    {"dict_arrow_down", LV_COLOR_FORMAT_ARGB8888},
    {"dict_send", LV_COLOR_FORMAT_ARGB8888},

    {"log_delete", LV_COLOR_FORMAT_ARGB8888},
    {"log_download", LV_COLOR_FORMAT_ARGB8888},
    {"log_search", LV_COLOR_FORMAT_ARGB8888},

    {"dashboard_chart", LV_COLOR_FORMAT_ARGB8888},
    {"dashboard_led", LV_COLOR_FORMAT_ARGB8888},
    {"dashboard_slider", LV_COLOR_FORMAT_ARGB8888},
    {"dashboard_btn", LV_COLOR_FORMAT_ARGB8888},
    {"dashboard_switch", LV_COLOR_FORMAT_ARGB8888},
    {"dashboard_arrange", LV_COLOR_FORMAT_ARGB8888},

    {"tips_close", LV_COLOR_FORMAT_ARGB8888},
    {"tips_warning", LV_COLOR_FORMAT_ARGB8888},

    {"details", LV_COLOR_FORMAT_ARGB8888},
    
    {"dashboard_delete", LV_COLOR_FORMAT_ARGB8888},

    {"eye_open", LV_COLOR_FORMAT_ARGB8888},
    {"eye_off", LV_COLOR_FORMAT_ARGB8888},

    {"dict_search", LV_COLOR_FORMAT_ARGB8888}

};


// UI 相关函数声明
static void SMP_Init(lv_obj_t* base_obj);
static void SMP_Content_Layout_Create(lv_obj_t* base_obj);
static void SMP_Navigation_Create(lv_obj_t* base_obj);

// 回调函数
static void Nav_obj_Click_Cb(lv_event_t  * e);
static void Smp_App_Setting_Cb(lv_event_t *e);
static void Smp_App_About_Cb(lv_event_t  * e);
static void Close_About_Dialog_Cb(lv_event_t *e);
static void Window_Contrl_Small_Cb(lv_event_t *e);
static void Window_Contrl_Min_Max_Cb(lv_event_t *e);
static void Window_Contrl_Close_Cb(lv_event_t *e);

// 工具函数
static lv_image_dsc_t * Image_Preload(const void * src, lv_color_format_t cf, int32_t scale);
static void Nav_Obj_Create(lv_obj_t * cur_obj,  Nav_Page_t page, uint8_t is_need_menu, 
                                const lv_image_dsc_t * icon_img_dsc);
static void Titlebar_Event_Cb(lv_event_t* e);

#if THEME_CHANGE_ENABLE
/*
 *    以下是主题切换的函数，由于每个对象主题切换时需要改变的样式不同。
 * 所以对于每个对象的主题切换，将交由对象自己处理。主线程将仅通知
 * 每个对象发生了主题切换。当然如果是同一类主题切换 例如：更改背景
 * 颜色也可以同时处理。
 */

// 背景主题回调
static void BG_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject);

// 导航栏图标对象
static void Nav_Icon_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject);

// 三个控制按钮的主题切换
static void Contrl_Btn_Icon_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject);

// 文本主题
static void Text_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject);

#endif

int Main_Window_Create(lv_obj_t* screen_act)
{
    // 创建整个主窗口对象
    lv_obj_set_style_bg_color(screen_act, lv_color_hex(0x1a1b1c), LV_PART_MAIN);
    
    lv_obj_t* smp_window = lv_obj_create(screen_act);
    lv_obj_set_size(smp_window, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(smp_window, LV_FLEX_FLOW_ROW);

    Data_Map_Init();

    // 初始化SMP上下文
    SMP_Init(smp_window);
    // 创建左边导航栏
    SMP_Navigation_Create(smp_window);
  
    // 创建内容布局区
    SMP_Content_Layout_Create(smp_window);

    // 调用每个页面各自的处理函数, 将控制权交由指定的页面
    Dashboard_Weight(g_smp_ctx);
    Data_Dictionary_Weight(g_smp_ctx);
    Topology_Display_Weight(g_smp_ctx);
    Experimental_Data_Weight(g_smp_ctx);
    Log_Managment_Weight(g_smp_ctx);

    return 0;
}

/*
 * 初始化SMP上下文
 */
static void SMP_Init(lv_obj_t* base_obj)
{
    g_smp_ctx = lv_malloc_zeroed(sizeof(smp_ctx_t));
    LV_ASSERT_MALLOC(g_smp_ctx);
    g_smp_ctx->ui = lv_malloc_zeroed(sizeof(*(g_smp_ctx->ui)));
    LV_ASSERT_MALLOC(g_smp_ctx->ui);

#if THEME_CHANGE_ENABLE
    lv_subject_init_int(&g_smp_ctx->theme_subject, SMP_THEME_LIGHT);
#endif
    for(uint32_t i = 0; i < NAV_PAGE_CNT; i++)
    {
        g_smp_ctx->pages[i]= lv_malloc_zeroed(sizeof(smp_page_t));
        LV_ASSERT_MALLOC(g_smp_ctx->pages[i]);
    }

    for(uint32_t i = 0; i < NAV_ICON_OBJ_CNT; i++)
    {
        g_smp_ctx->ui->nav_icon[i] = lv_malloc_zeroed(sizeof(icons_obj_ctx_t));
        LV_ASSERT_MALLOC(g_smp_ctx->ui->nav_icon[i]);
    }

    for(uint32_t i = 0; i < IMG_CNT; i++)
    {
        /*
         * 如果遇到图片无法显示的问题，为可执行文件路径与图标路径的相对位置不对,请将图片放置在当前可执行文件的同级目录 当前配置的目录结构为
         * out
         *  |__img
         *  |    |___ all images
         *  |___ smp.exe
         *  
         * 特别注意在终端运行时，请进入out目录中运行程序, 否则图标将不能够按照预期显示.
         */ 
        char path_buf[256];
        int chars = lv_snprintf(path_buf, sizeof(path_buf), "%c:img/%s.png", LV_FS_STDIO_LETTER, image_details[i].name);
        LV_ASSERT(chars < (int)sizeof(path_buf));
        g_smp_ctx->imgs[i] = Image_Preload(path_buf, image_details[i].color_format, LV_SCALE_NONE);
    }
    
    g_smp_ctx->window_data = lv_display_get_driver_data( lv_display_get_default());
    LV_ASSERT_NULL(g_smp_ctx->window_data);

#if THEME_CHANGE_ENABLE
    g_smp_ctx->theme[SMP_THEME_LIGHT].bg = lv_color_hex(0xfafafa);
    g_smp_ctx->theme[SMP_THEME_LIGHT].text = lv_color_black();

    g_smp_ctx->theme[SMP_THEME_DARK].bg = NAV_BG_COLOR;
    g_smp_ctx->theme[SMP_THEME_DARK].text = lv_color_white();
#endif

}

/*
 * 创建导航栏
 */
static void SMP_Navigation_Create(lv_obj_t* base_obj)
{
    lv_obj_t* nav_bar = lv_obj_create(base_obj);
    lv_obj_set_size(nav_bar, NAVIGATION_BAR_WIDTH, lv_pct(100));
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(nav_bar, NAV_BG_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(nav_bar, 10, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(nav_bar, GAP_BORDER_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(nav_bar, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(nav_bar, LV_BORDER_SIDE_RIGHT, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(nav_bar, LV_OPA_10, LV_STATE_DEFAULT);
    lv_obj_clear_flag(nav_bar, LV_OBJ_FLAG_SCROLLABLE);
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, BG_Theme_Change_Cb, nav_bar);
#endif

    // 设置默认显示的页面是 dashboard
    g_smp_ctx->ui->cur_selected_page = NAV_PAGE_DASHBOARD;

    // 创建图标的容器
    lv_obj_t * icon_dashboard = lv_obj_create(nav_bar);
    lv_obj_add_flag(icon_dashboard, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(icon_dashboard, NAV_PAGE_DASHBOARD, TRUE, g_smp_ctx->imgs[IMG_NAVIGATION_DASHBOARD_SELECTED]);
    // 第一个图标作为默认选中
    lv_strcpy(g_smp_ctx->ui->nav_icon[NAV_ICON_OBJ_DASHBOARD]->icon_name, "仪表盘");
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, icon_dashboard);
#endif


    lv_obj_t * icon_data_dictionary = lv_obj_create(nav_bar);
    lv_obj_add_flag(icon_data_dictionary, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(icon_data_dictionary, NAV_PAGE_DATA_DICTIONARY, TRUE, g_smp_ctx->imgs[IMG_NAVIGATION_DATA_DICTIONARY]);
    lv_strcpy(g_smp_ctx->ui->nav_icon[NAV_ICON_OBJ_DATA_DICTIONARY]->icon_name, "数据字典");
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, icon_data_dictionary);
#endif

    lv_obj_t * icon_topology_display = lv_obj_create(nav_bar);
    lv_obj_add_flag(icon_topology_display, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(icon_topology_display, NAV_PAGE_TOPOLOGY, TRUE, g_smp_ctx->imgs[IMG_NAVIGATION_TOPOLOGY_DISPLAY]);

    lv_strcpy(g_smp_ctx->ui->nav_icon[NAV_ICON_OBJ_TOPOLOGY]->icon_name, "拓扑展示");
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, icon_topology_display);
#endif


    lv_obj_t * icon_experimental_data = lv_obj_create(nav_bar);
    lv_obj_add_flag(icon_experimental_data, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(icon_experimental_data, NAV_PAGE_EXPERIMENT, FALSE, g_smp_ctx->imgs[IMG_NAVIGATION_EXPERIMENTAL_DATA]);

    lv_strcpy(g_smp_ctx->ui->nav_icon[NAV_ICON_OBJ_EXPERIMENT]->icon_name, "实验管理");
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, icon_experimental_data);
#endif

    lv_obj_t * icon_log_management = lv_obj_create(nav_bar);
    lv_obj_add_flag(icon_log_management, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(icon_log_management, NAV_PAGE_LOG, TRUE, g_smp_ctx->imgs[IMG_NAVIGATION_LOG_MANAGEMENT]);
    lv_strcpy(g_smp_ctx->ui->nav_icon[NAV_ICON_OBJ_LOG]->icon_name, "日志管理");
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, icon_log_management);
#endif

    lv_obj_t * spacer = lv_obj_create(nav_bar);
    lv_obj_set_flex_grow(spacer, 1);  
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(spacer, 0, LV_STATE_DEFAULT); 
    lv_obj_set_style_pad_all(spacer, 0, LV_STATE_DEFAULT);  

    lv_obj_t *nav_about = lv_obj_create(nav_bar);
    lv_obj_add_flag(nav_about, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(nav_about, NAV_PAGE_ABOUT, FALSE, g_smp_ctx->imgs[IMG_NAVIGATION_ABOUT]);
    lv_obj_set_style_bg_image_src(nav_about, g_smp_ctx->imgs[IMG_NAVIGATION_ABOUT_SELECTED], LV_STATE_PRESSED);
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, nav_about);
#endif

    lv_obj_t * icon_setting = lv_obj_create(nav_bar);
    lv_obj_add_flag(icon_setting, LV_OBJ_FLAG_CHECKABLE);
    Nav_Obj_Create(icon_setting, NAV_PAGE_SETTING, FALSE, g_smp_ctx->imgs[IMG_NAVIGATION_SETTING]);
    lv_obj_set_style_bg_image_src(icon_setting, g_smp_ctx->imgs[IMG_NAVIGATION_SETTING_SELECTED], LV_STATE_PRESSED);
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, Nav_Icon_Theme_Change_Cb, icon_setting);
#endif

}

static void SMP_Content_Layout_Create(lv_obj_t* base_obj)
{
    lv_obj_t* smp_content_window = lv_obj_create(base_obj);
    lv_obj_set_size(smp_content_window, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(smp_content_window, 1);
    lv_obj_set_flex_flow(smp_content_window, LV_FLEX_FLOW_COLUMN);

    lv_obj_t* smp_content_top_bar = lv_obj_create(smp_content_window);
    lv_obj_set_size(smp_content_top_bar, lv_pct(100), TOP_BAR_HEIGHT);
    lv_obj_set_flex_flow(smp_content_top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(smp_content_top_bar, TOP_BAR_BG_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_opa(smp_content_top_bar,LV_OPA_COVER,LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(smp_content_top_bar, GAP_BORDER_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(smp_content_top_bar, 1,  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(smp_content_top_bar, LV_BORDER_SIDE_BOTTOM, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(smp_content_top_bar, LV_OPA_10, LV_STATE_DEFAULT);

    lv_obj_t* smp_content_top_bar_drag_area = lv_obj_create(smp_content_top_bar);
    lv_obj_set_size(smp_content_top_bar_drag_area, lv_pct(100), TOP_BAR_HEIGHT-1);
    lv_obj_set_style_bg_color(smp_content_top_bar_drag_area, TOP_BAR_BG_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_flex_grow(smp_content_top_bar_drag_area, 1); 
    lv_obj_add_event_cb(smp_content_top_bar_drag_area, Titlebar_Event_Cb, LV_EVENT_ALL, g_smp_ctx);
#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, BG_Theme_Change_Cb, smp_content_top_bar_drag_area);
#endif

    g_smp_ctx->ui->top_bar_label = lv_label_create(smp_content_top_bar_drag_area);
    lv_label_set_text(g_smp_ctx->ui->top_bar_label, "仪表盘");
    lv_obj_set_style_text_color(g_smp_ctx->ui->top_bar_label, TEXT_COLOR, LV_PART_MAIN);
    lv_obj_set_align(g_smp_ctx->ui->top_bar_label,LV_ALIGN_LEFT_MID);
    lv_obj_set_x(g_smp_ctx->ui->top_bar_label, 20);

    lv_obj_t* contrl_btn_area = lv_obj_create(smp_content_top_bar);
    lv_obj_set_size(contrl_btn_area, 150, TOP_BAR_HEIGHT-1);
    lv_obj_set_flex_flow(contrl_btn_area, LV_FLEX_FLOW_ROW);

    // 窗口控制按钮
    lv_obj_t* contrl_btn_small = lv_obj_create(contrl_btn_area);
    g_smp_ctx->ui->contrl_btn_min_max = lv_obj_create(contrl_btn_area);
    lv_obj_t* contrl_btn_close = lv_obj_create(contrl_btn_area);
    lv_obj_set_size(contrl_btn_small, lv_pct(100), TOP_BAR_HEIGHT);
    lv_obj_set_size(g_smp_ctx->ui->contrl_btn_min_max, lv_pct(100), TOP_BAR_HEIGHT);
    lv_obj_set_size(contrl_btn_close,  lv_pct(100), TOP_BAR_HEIGHT);  
    lv_obj_add_event_cb(contrl_btn_small, Window_Contrl_Small_Cb,LV_EVENT_CLICKED, g_smp_ctx);
    lv_obj_add_event_cb(g_smp_ctx->ui->contrl_btn_min_max, Window_Contrl_Min_Max_Cb,LV_EVENT_PRESSED, g_smp_ctx);
    lv_obj_add_event_cb(g_smp_ctx->ui->contrl_btn_min_max, Window_Contrl_Min_Max_Cb,LV_EVENT_RELEASED, g_smp_ctx);
    lv_obj_add_event_cb(contrl_btn_close, Window_Contrl_Close_Cb,LV_EVENT_CLICKED, g_smp_ctx);


    lv_obj_set_flex_grow(contrl_btn_small,   1);
    lv_obj_set_flex_grow(g_smp_ctx->ui->contrl_btn_min_max, 1);
    lv_obj_set_flex_grow(contrl_btn_close,   1);
    lv_obj_set_style_bg_color(contrl_btn_small, TOP_BAR_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_smp_ctx->ui->contrl_btn_min_max, TOP_BAR_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(contrl_btn_close, TOP_BAR_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

#if THEME_CHANGE_ENABLE
    lv_subject_add_observer(&g_smp_ctx->theme_subject, BG_Theme_Change_Cb, contrl_btn_small);
    lv_subject_add_observer(&g_smp_ctx->theme_subject, BG_Theme_Change_Cb, g_smp_ctx->ui->contrl_btn_min_max);
    lv_subject_add_observer(&g_smp_ctx->theme_subject, BG_Theme_Change_Cb, contrl_btn_close);
#endif

    lv_obj_set_style_bg_image_src(contrl_btn_small, g_smp_ctx->imgs[IMG_CONTRL_BTN_SMALL], LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(g_smp_ctx->ui->contrl_btn_min_max, g_smp_ctx->imgs[IMG_CONTRL_BTN_MAX], LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(contrl_btn_close, g_smp_ctx->imgs[IMG_CONTRL_BTN_CLOSE], LV_STATE_DEFAULT);

    lv_obj_set_style_bg_image_src(contrl_btn_small, g_smp_ctx->imgs[IMG_CONTRL_BTN_SMALL_SELECTED], LV_STATE_PRESSED);
    lv_obj_set_style_bg_image_src(contrl_btn_close, g_smp_ctx->imgs[IMG_CONTRL_BTN_CLOSE_SELECTED], LV_STATE_PRESSED);

    g_smp_ctx->ui->smp_content_area = lv_obj_create(smp_content_window);
    lv_obj_set_flex_grow(g_smp_ctx->ui->smp_content_area, 1);
    lv_obj_set_size(g_smp_ctx->ui->smp_content_area, lv_pct(100), lv_pct(100));

    lv_obj_remove_flag(smp_content_top_bar, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_remove_flag(smp_content_top_bar_drag_area, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_remove_flag(contrl_btn_area, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_remove_flag(contrl_btn_small, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_remove_flag(g_smp_ctx->ui->contrl_btn_min_max, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_remove_flag(contrl_btn_close, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_remove_flag(g_smp_ctx->ui->smp_content_area, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);


    for(int i = 0; i < NAV_PAGE_CNT; i++)
    {
        g_smp_ctx->pages[i]->show_content = lv_obj_create(g_smp_ctx->ui->smp_content_area);
        LV_ASSERT_NULL(g_smp_ctx->pages[i]->show_content);

        lv_obj_set_size(g_smp_ctx->pages[i]->show_content, lv_pct(100), lv_pct(100));
        lv_obj_set_pos(g_smp_ctx->pages[i]->show_content, 0, 0);
        lv_obj_set_flex_flow(g_smp_ctx->pages[i]->show_content , LV_FLEX_FLOW_ROW);
        lv_obj_clear_flag(g_smp_ctx->pages[i]->show_content, LV_OBJ_FLAG_SCROLLABLE);

         
        // 除了仪表盘页面，其余页面全部隐藏
        if(i != 0) {
            lv_obj_add_flag(g_smp_ctx->pages[i]->show_content, LV_OBJ_FLAG_HIDDEN);
        }
        
        if(g_smp_ctx->pages[i]->is_need_menu)
        {
            g_smp_ctx->pages[i]->menu = lv_obj_create(g_smp_ctx->pages[i]->show_content);
            LV_ASSERT_NULL(g_smp_ctx->pages[i]->menu);
            
            lv_obj_t *smp_menu = g_smp_ctx->pages[i]->menu;
            
            lv_obj_set_size(smp_menu, NAVIGATION_MENU_WIDTH, lv_pct(100));
            lv_obj_set_style_bg_color(smp_menu, MENU_BG_COLOR, LV_STATE_DEFAULT);
            lv_obj_set_flex_grow(smp_menu, 0);
        }
        
        // 该区域是必有的    
        g_smp_ctx->pages[i]->activity_content = lv_obj_create(g_smp_ctx->pages[i]->show_content);
        LV_ASSERT_NULL(g_smp_ctx->pages[i]->activity_content);
        lv_obj_t *activity_content = g_smp_ctx->pages[i]->activity_content;
        lv_obj_set_flex_grow(activity_content, 1);
        lv_obj_set_height(activity_content, lv_pct(100));
        lv_obj_set_size(activity_content, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_color(activity_content, ACTIVITY_CONTENT_BG_COLOR, LV_STATE_DEFAULT);  
    }

}


/*
 * 导航栏图标点击回调函数
 */
static void Nav_obj_Click_Cb(lv_event_t * e)
{
    
    lv_obj_t * cur_target = lv_event_get_target(e);
    lv_obj_t * prv_obj    = g_smp_ctx->ui->nav_icon[g_smp_ctx->ui->cur_selected_page]->icons_obj;
    Nav_Page_t prv_page = g_smp_ctx->ui->cur_selected_page;
    
    // 更新当前选中的页面
    for(uint32_t i = 0; i < NAV_ICON_OBJ_CNT_GAP; i++)
    {
        if(cur_target == g_smp_ctx->ui->nav_icon[i]->icons_obj)
        {
            g_smp_ctx->ui->cur_selected_page = (Nav_Page_t)i;
            break;
        }
    }
    
    if(prv_page == g_smp_ctx->ui->cur_selected_page)      
        return;

#if THEME_CHANGE_ENABLE    
    if(lv_subject_get_int(&g_smp_ctx->theme_subject) == SMP_THEME_LIGHT)
    {
        // 取消先前选中的图标
        lv_obj_set_style_bg_image_src(prv_obj, g_smp_ctx->imgs[prv_page * 3 + 2], LV_STATE_DEFAULT); 
    }
    else
    {
#endif
        lv_obj_set_style_bg_image_src(prv_obj, g_smp_ctx->imgs[prv_page * 3], LV_STATE_DEFAULT); 

#if THEME_CHANGE_ENABLE
    }
#endif

    // 切换页面
    lv_obj_add_flag(g_smp_ctx->pages[prv_page]->show_content, LV_OBJ_FLAG_HIDDEN);
    g_smp_ctx->pages[prv_page]->cur_page_is_hidden = TRUE;
    lv_obj_clear_flag(g_smp_ctx->pages[g_smp_ctx->ui->cur_selected_page]->show_content, LV_OBJ_FLAG_HIDDEN);
    g_smp_ctx->pages[g_smp_ctx->ui->cur_selected_page ]->cur_page_is_hidden = FALSE;

    if (g_smp_ctx->ui->setting_panel && !lv_obj_has_flag(g_smp_ctx->ui->setting_panel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_move_foreground(g_smp_ctx->ui->setting_panel);
    }
    // 设置当前选中的图标并设置样式
    lv_obj_set_style_bg_image_src(cur_target, g_smp_ctx->imgs[g_smp_ctx->ui->cur_selected_page * 3 + 1], LV_STATE_DEFAULT);

    // 设置文本
    lv_label_set_text(g_smp_ctx->ui->top_bar_label, g_smp_ctx->ui->nav_icon[g_smp_ctx->ui->cur_selected_page]->icon_name);

}

static void Smp_App_Setting_Cb(lv_event_t *e) 
{
    // 如果对象不存在，创建它
    if (g_smp_ctx->ui->setting_panel == NULL) {
        g_smp_ctx->ui->setting_panel = lv_obj_create(g_smp_ctx->ui->smp_content_area);
        lv_obj_t *setting = g_smp_ctx->ui->setting_panel;
        
        lv_obj_add_flag(setting, LV_OBJ_FLAG_FLOATING);
        lv_obj_move_foreground(setting);
        lv_obj_set_size(setting, SETTING_OBJ_WIDTH, SETTING_OBJ_HIGHT);
        lv_obj_align(setting, LV_ALIGN_BOTTOM_LEFT, 5, -5);
        lv_obj_set_style_bg_color(setting, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_border_color(setting, GAP_BORDER_COLOR, LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(setting, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(setting, LV_OPA_10, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(setting, 8, LV_PART_MAIN);
        return;
    }
    
    // 切换显示/隐藏状态
    if (lv_obj_has_flag(g_smp_ctx->ui->setting_panel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(g_smp_ctx->ui->setting_panel, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(g_smp_ctx->ui->setting_panel, LV_OBJ_FLAG_HIDDEN);
    }
}

static void Window_Contrl_Small_Cb(lv_event_t *e)
{
    SDL_Window* window = g_smp_ctx->window_data->window;
    SDL_MinimizeWindow(window);

}
static void Window_Contrl_Min_Max_Cb(lv_event_t *e)
{
    SDL_Window* window = g_smp_ctx->window_data->window;
    lv_event_code_t code = lv_event_get_code(e);
    
    // 当前窗口时默认状态还是最大化状态 0 --> 默认 1 --> 最大化
    static uint8_t flag_state = 0;

    if (code == LV_EVENT_PRESSED) {
        if(flag_state == 0)
        {
            lv_obj_set_style_bg_image_src(g_smp_ctx->ui->contrl_btn_min_max, g_smp_ctx->imgs[IMG_CONTRL_BTN_MAX_SELECTED], LV_PART_MAIN);
        }
        else{
            lv_obj_set_style_bg_image_src(g_smp_ctx->ui->contrl_btn_min_max, g_smp_ctx->imgs[IMG_CONTRL_BTN_MIN_SELECTED], LV_PART_MAIN);
        }
    }

    if (code == LV_EVENT_RELEASED) {
        if(flag_state == 0)
        {
            lv_obj_set_style_bg_image_src(g_smp_ctx->ui->contrl_btn_min_max, g_smp_ctx->imgs[IMG_CONTRL_BTN_MIN], LV_PART_MAIN);
            SDL_MaximizeWindow(window);
            flag_state = 1;
        }else{
            lv_obj_set_style_bg_image_src(g_smp_ctx->ui->contrl_btn_min_max, g_smp_ctx->imgs[IMG_CONTRL_BTN_MAX], LV_PART_MAIN);
            SDL_RestoreWindow(window);
            flag_state = 0;
        }
    }
    
}

extern void Data_Map_Test_Stop(void);

static void Window_Contrl_Close_Cb(lv_event_t *e)
{
    // TODO 停止数据注入线程避免段错误，待删除
    Data_Map_Test_Stop();
    // 向各个页面发送删除事件，通知各个页面做好删除工作
    for(int i = 0; i < NAV_PAGE_CNT; i++)
    {
        if(g_smp_ctx->pages[i]->is_need_menu)
            lv_obj_delete(g_smp_ctx->pages[i]->menu); 
        lv_obj_delete(g_smp_ctx->pages[i]->activity_content);
    }

    Data_Map_Destroy();

    for(int i = 0; i < IMG_CNT; i++)
    {
        if(g_smp_ctx->imgs[i])
            lv_draw_buf_destroy((lv_draw_buf_t *)g_smp_ctx->imgs[i]);
    }

    lv_display_t * disp = lv_display_get_default();
    lv_display_delete(disp);
    
    for(int i = 0; i < NAV_ICON_OBJ_CNT; i++)
    {
        if(g_smp_ctx->ui->nav_icon[i])
            lv_free(g_smp_ctx->ui->nav_icon[i]);
    }
        
    for(int i = 0; i < NAV_PAGE_CNT; i++)
    {
        if(g_smp_ctx->pages[i])
            lv_free(g_smp_ctx->pages[i]);
    }
    
    lv_free(g_smp_ctx->ui);
    lv_free(g_smp_ctx);
    
    SDL_Quit();
    lv_deinit();
    exit(0);  
}

// 关闭弹窗的回调函数
static void Close_About_Dialog_Cb(lv_event_t *e)
{
    lv_obj_t *dialog = lv_event_get_user_data(e);
    uint8_t* f_data = lv_obj_get_user_data(lv_event_get_target_obj(e));
    LV_ASSERT_NULL(dialog);
    *f_data = 0;
    lv_obj_del(dialog);  
}

/*
 * 关于弹窗
 */
static void Smp_App_About_Cb(lv_event_t * e)
{
    static uint8_t f_already_clicked = 0;

    if(f_already_clicked)
        return;
    
    lv_obj_t *dialog = lv_obj_create(lv_layer_top());
    f_already_clicked = 1;
    lv_obj_set_size(dialog, 400, 260);
    lv_obj_center(dialog);

    lv_obj_set_style_bg_color(dialog, DIALOG_BG_COLOR, 0);
    lv_obj_set_style_radius(dialog, 12, 0);
    lv_obj_set_style_shadow_width(dialog, 30, 0);
    lv_obj_set_style_shadow_opa(dialog, LV_OPA_20, 0);
    lv_obj_set_style_border_width(dialog, 1, LV_PART_MAIN);
    lv_obj_set_flex_flow(dialog, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *header = lv_obj_create(dialog);
    lv_obj_set_height(header, 45);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(header, GAP_BORDER_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(header, 1,  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(header, LV_OPA_10, LV_STATE_DEFAULT);

    lv_obj_t *logo_container = lv_obj_create(header);
    lv_obj_set_size(logo_container, 370, 44);
    lv_obj_set_style_bg_color(logo_container, DIALOG_BG_COLOR, LV_STATE_DEFAULT);

    lv_obj_t *img = lv_img_create(logo_container);
    lv_img_set_src(img, g_smp_ctx->imgs[IMG_LOGO]);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 4, 5);
    

    lv_obj_t *close_btn = lv_obj_create(header);
    lv_obj_set_size(close_btn, 32, 32);
    lv_obj_set_user_data(close_btn, (void *)&f_already_clicked);
    lv_obj_set_style_radius(close_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(close_btn, DIALOG_BG_COLOR, 0);
    lv_obj_set_style_shadow_width(close_btn, 0, 0);
    lv_obj_set_style_border_width(close_btn, 0, 0);
    lv_obj_get_style_margin_right(close_btn, 8);
    lv_obj_set_style_bg_image_src(close_btn,g_smp_ctx->imgs[IMG_CONTRL_BTN_CLOSE], LV_PART_MAIN);
    lv_obj_align(close_btn, LV_ALIGN_RIGHT_MID,-6, 0);

    lv_obj_add_event_cb(close_btn, Close_About_Dialog_Cb, LV_EVENT_CLICKED,  dialog);

    lv_obj_t *content = lv_obj_create(dialog);
    lv_obj_set_width(content, LV_PCT(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_style_bg_color(content, DIALOG_BG_COLOR, 0);
    lv_obj_set_style_radius(content, 8, 0);
    lv_obj_set_style_pad_all(content, 8, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    
    lv_obj_t *dialog_pic = lv_obj_create(content);
    lv_obj_set_size(dialog_pic,128,128);
    lv_obj_set_style_bg_color(dialog_pic,DIALOG_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_image_src(dialog_pic, g_smp_ctx->imgs[ABOUT_BG], LV_PART_MAIN);
    lv_obj_align(dialog_pic,LV_ALIGN_BOTTOM_RIGHT,0,0);

    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scroll_snap_x(content, LV_SCROLL_SNAP_CENTER);

    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    /* 说明文字 */
    lv_obj_t *text = lv_label_create(content);
    lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(text, LV_PCT(100));

    lv_label_set_text(text, DIALOG_ILLUSTRATE_TEXT);
    lv_obj_set_style_text_font(text, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(text, 4, 0);
    lv_obj_set_style_text_color(text, TEXT_COLOR, LV_PART_MAIN);
}

/*
 * 创建导航栏图标对象
 */
static void Nav_Obj_Create(lv_obj_t * cur_obj,  Nav_Page_t page, uint8_t is_need_menu, 
                                const lv_image_dsc_t * icon_img_dsc)
{
    lv_obj_remove_style_all(cur_obj);
    lv_obj_set_size(cur_obj, NAVIGATION_BAR_WIDTH, NAVIGATION_BAR_WIDTH);
    lv_obj_add_flag(cur_obj, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_set_style_bg_image_src(cur_obj, icon_img_dsc, LV_STATE_DEFAULT);
    lv_obj_set_user_data(cur_obj, g_smp_ctx);
    g_smp_ctx->ui->nav_icon[page]->icons_obj = cur_obj;

    if(page == NAV_PAGE_SETTING)
        lv_obj_add_event_cb(cur_obj, Smp_App_Setting_Cb, LV_EVENT_CLICKED, g_smp_ctx);
    else if(page == NAV_PAGE_ABOUT)
         lv_obj_add_event_cb(cur_obj, Smp_App_About_Cb, LV_EVENT_CLICKED, g_smp_ctx);
    else
        lv_obj_add_event_cb(cur_obj, Nav_obj_Click_Cb, LV_EVENT_CLICKED, NULL);

   g_smp_ctx->pages[page]->is_need_menu = is_need_menu;
}

/*
 * 预加载图片 
 */
static lv_image_dsc_t * Image_Preload(const void * src, lv_color_format_t cf, int32_t scale)
{
    // 读取png图片的头信息
    lv_image_header_t header;
    lv_result_t res = lv_image_decoder_get_info(src, &header);
    
    if(res == LV_RESULT_INVALID) {
        LV_LOG_WARN("Couldn't read the header info of source");
        return NULL;
    }

    lv_draw_buf_t * dest;
    int32_t dest_w = header.w * scale / 256;
    int32_t dest_h = header.h * scale / 256;
    dest = lv_draw_buf_create(dest_w, dest_h, cf, LV_STRIDE_AUTO);
    if(!dest) return NULL;

    lv_obj_t * canvas = lv_canvas_create(lv_screen_active());
    if(!canvas) {
        lv_draw_buf_destroy(dest);
        return NULL;
    }
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_HIDDEN);
    lv_canvas_set_draw_buf(canvas, dest);
    lv_canvas_fill_bg(canvas, lv_color_hex3(0x000), LV_OPA_TRANSP);

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_draw_image_dsc_t dsc;
    lv_draw_image_dsc_init(&dsc);
    dsc.src = src;
    dsc.scale_x = scale;
    dsc.scale_y = scale;

    // 绘制图片到canvas的layer上
    lv_area_t coords = {0, 0, header.w - 1, header.h - 1};
    lv_draw_image(&layer, &dsc, &coords);
    lv_canvas_finish_layer(canvas, &layer);

    lv_obj_delete(canvas);

    return (lv_image_dsc_t *) dest;
}

static void Titlebar_Event_Cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    static bool dragging = false;
    static int drag_offset_x = 0;
    static int drag_offset_y = 0;
    static uint32_t last_update_time = 0;
    static int last_win_x = 0;
    static int last_win_y = 0;
    
    if (code == LV_EVENT_PRESSED) {
        int global_x, global_y;
        SDL_GetGlobalMouseState(&global_x, &global_y);
        
        int win_x, win_y;
        SDL_GetWindowPosition(g_smp_ctx->window_data->window, &win_x, &win_y);
        
        drag_offset_x = global_x - win_x;
        drag_offset_y = global_y - win_y;
        last_win_x = win_x;
        last_win_y = win_y;
        last_update_time = lv_tick_get();
        dragging = true;
    }
    else if (code == LV_EVENT_PRESSING) {
        if (!dragging) return;
        
        uint32_t now = lv_tick_get();
        if (now - last_update_time < 16) {
            return;
        }
        
        int global_x, global_y;
        SDL_GetGlobalMouseState(&global_x, &global_y);
        
        int new_x = global_x - drag_offset_x;
        int new_y = global_y - drag_offset_y;
        
        /* 只有位置真正变化时才更新 */
        if (new_x != last_win_x || new_y != last_win_y) {
            SDL_SetWindowPosition(g_smp_ctx->window_data->window, new_x, new_y);
            last_win_x = new_x;
            last_win_y = new_y;
            last_update_time = now;
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        dragging = false;
    }
}

#if THEME_CHANGE_ENABLE
/*
 * 主题切换回调
 */
static void BG_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject)
{
    smp_theme_t theme = lv_subject_get_int(subject);
    LV_ASSERT_NULL(observer);
    lv_obj_t* obj = observer->user_data;

    switch(theme) {
        case SMP_THEME_LIGHT:
            // 设置浅色样式
            lv_obj_set_style_bg_color(obj, g_smp_ctx->theme[SMP_THEME_LIGHT].bg, LV_PART_MAIN);
            break;
        case SMP_THEME_DARK:
            // 设置深色样式
            lv_obj_set_style_bg_color(obj, g_smp_ctx->theme[SMP_THEME_DARK].bg, LV_PART_MAIN);
            break;
    }
}

static void Text_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject)
{
    // TODO 文本的主题切换，需要添加观察者
    smp_theme_t theme = lv_subject_get_int(subject);
    LV_ASSERT_NULL(observer);
    lv_obj_t* obj = observer->user_data;

    switch(theme) {
        case SMP_THEME_LIGHT:
            // 设置浅色样式
            lv_obj_set_style_text_color(obj, g_smp_ctx->theme[SMP_THEME_LIGHT].text, LV_PART_MAIN);
            break;
        case SMP_THEME_DARK:
            // 设置深色样式
            lv_obj_set_style_bg_color(obj, g_smp_ctx->theme[SMP_THEME_DARK].text, LV_PART_MAIN);
            break;
    }
}

static void Nav_Icon_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject)
{
    // 这里这样做的目的是避免取消仪表盘的选中状态
    static uint8_t flag = 1;
    if(flag)
    {
        flag = 0;
        return;
    }
    smp_theme_t theme = lv_subject_get_int(subject);
    LV_ASSERT_NULL(observer);
    lv_obj_t* obj = observer->user_data;
    //当前对象的索引
    uint8_t index = 0; 
    for(; index < NAV_ICON_OBJ_CNT; index++)
    {
        if(obj == g_smp_ctx->ui->nav_icon[index]->icons_obj)
            break;
    }

    if(index == NAV_ICON_OBJ_SETTING)
        --index;

    switch(theme) {
    case SMP_THEME_LIGHT:
        // 设置深色图标
        lv_obj_set_style_bg_image_src(obj, g_smp_ctx->imgs[index*3+2] , LV_STATE_DEFAULT);
        break;
    case SMP_THEME_DARK:
        // 设置浅色图标
        lv_obj_set_style_bg_image_src(obj,g_smp_ctx->imgs[index*3] , LV_STATE_DEFAULT);
        break;
    }
    
}

#if THEME_CHANGE_ENABLE && 0
static void Contrl_Btn_Icon_Theme_Change_Cb(lv_observer_t * observer, lv_subject_t * subject)
{
    // TODO 右上角的3个按钮的主题切换需完成，并且最大化和最小化有点麻烦，他们有不同的图标
    smp_theme_t theme = lv_subject_get_int(subject);
    LV_ASSERT_NULL(observer);
    lv_obj_t* obj = observer->user_data;
    // 当前对象的索引
    uint8_t index = 0; 
    for(; index < 3; index++)
    {
        if(obj == g_smp_ctx->ui->nav_icon[index]->icons_obj)
            break;
    }

    switch(theme) {
        case SMP_THEME_LIGHT:
            // 设置深色图标
            lv_obj_set_style_bg_image_src(obj, g_smp_ctx->imgs[index*3+2] , LV_STATE_DEFAULT);
            break;
        case SMP_THEME_DARK:
            // 设置浅色图标
            lv_obj_set_style_bg_image_src(obj,g_smp_ctx->imgs[index*3] , LV_STATE_DEFAULT);
            break;
    }
}
#endif


#endif

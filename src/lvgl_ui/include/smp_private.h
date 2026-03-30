#ifndef SMP_PRIVATE_H
#define SMP_PRIVATE_H

#include "lvgl.h"

// 是否启用主题切换
#define THEME_CHANGE_ENABLE             0     
//#15b1e0
#define NAVIGATION_BAR_WIDTH            75
#define NAVIGATION_MENU_WIDTH           200 
#define TOP_BAR_HEIGHT                  60
#define SETTING_OBJ_WIDTH               200
#define SETTING_OBJ_HIGHT               100

#define MY_TEST_COLOR_ONE               lv_color_hex(0x474554)
#define MY_TEST_COLOR_TWO               lv_color_hex(0xaca7cb)
#define MY_TEST_COLOR_THERE             lv_color_hex(0xdedbee)

#define DIALOG_BG_COLOR                 lv_color_hex(0x1a1b1c)
#define DIALOG_BOADER_COLOR             lv_color_hex(0x1e1e1e)

#define NAV_BG_COLOR                    lv_color_hex(0x1a1b1c)
#define MENU_BG_COLOR                   lv_color_hex(0x1A1B1C)
#define TOP_BAR_BG_COLOR                lv_color_hex(0x1a1b1c)
#define ACTIVITY_CONTENT_BG_COLOR       lv_color_hex(0x262424)
#define TEXT_COLOR                      lv_color_hex(0xffffff)
#define GAP_BORDER_COLOR                lv_color_hex(0xfafafa)

#define DIALOG_ILLUSTRATE_TEXT          ("        高集成度阀门驱控综合测试系统说明 \n软件版本: Version 1.0.0\n\n 本软件用于设备数据管理与可视化展示, 支持拓扑展示, 实验数据分析及日志管理等功能. \n                              Copyright  2026")

#define FALSE                           0   
#define TRUE                            1

typedef enum
{
    NAV_PAGE_DASHBOARD,
    NAV_PAGE_DATA_DICTIONARY,
    NAV_PAGE_TOPOLOGY,
    NAV_PAGE_EXPERIMENT,
    NAV_PAGE_LOG,
    NAV_PAGE_ABOUT,
    NAV_PAGE_SETTING,
    NAV_PAGE_CNT
} Nav_Page_t;

enum
{
    NAV_ICON_OBJ_DASHBOARD,
    NAV_ICON_OBJ_DATA_DICTIONARY,
    NAV_ICON_OBJ_TOPOLOGY,
    NAV_ICON_OBJ_EXPERIMENT,
    NAV_ICON_OBJ_LOG,
    NAV_ICON_OBJ_ABOUT,
    NAV_ICON_OBJ_CNT_GAP,
    NAV_ICON_OBJ_SETTING,
    NAV_ICON_OBJ_CNT
};

enum
{
    IMG_NAVIGATION_DASHBOARD = 0,
    IMG_NAVIGATION_DASHBOARD_SELECTED,
    IMG_NAVIGATION_DASHBOARD_DARK,

    IMG_NAVIGATION_DATA_DICTIONARY,
    IMG_NAVIGATION_DATA_DICTIONARY_SELECTED,
    IMG_NAVIGATION_DATA_DICTIONARY_DARK,

    IMG_NAVIGATION_TOPOLOGY_DISPLAY,
    IMG_NAVIGATION_TOPOLOGY_DISPLAY_SELECTED,
    IMG_NAVIGATION_TOPOLOGY_DISPLAY_DARK,

    IMG_NAVIGATION_EXPERIMENTAL_DATA,
    IMG_NAVIGATION_EXPERIMENTAL_DATA_SELECTED,
    IMG_NAVIGATION_EXPERIMENTAL_DATA_DARK,

    IMG_NAVIGATION_LOG_MANAGEMENT,
    IMG_NAVIGATION_LOG_MANAGEMENT_SELECTED,
    IMG_NAVIGATION_LOG_MANAGEMENT_DARK,

    IMG_NAVIGATION_ABOUT,
    IMG_NAVIGATION_ABOUT_SELECTED,
    IMG_NAVIGATION_ABOUT_DARK,

    IMG_NAVIGATION_SETTING,
    IMG_NAVIGATION_SETTING_SELECTED,
    IMG_NAVIGATION_SETTING_DARK,

    IMG_CONTRL_BTN_SMALL,
    IMG_CONTRL_BTN_SMALL_SELECTED,
    IMG_CONTRL_BTN_SMALL_DARK,
    IMG_CONTRL_BTN_MIN,
    IMG_CONTRL_BTN_MIN_SELECTED,
    IMG_CONTRL_BTN_MIN_DARK,
    IMG_CONTRL_BTN_MAX,
    IMG_CONTRL_BTN_MAX_SELECTED,
    IMG_CONTRL_BTN_MAX_DARK,
    IMG_CONTRL_BTN_CLOSE,
    IMG_CONTRL_BTN_CLOSE_SELECTED,
    IMG_CONTRL_BTN_CLOSE_DARK,

    IMG_LOGO,
    ABOUT_BG,

    IMG_DICT_ARROW_UP,
    IMG_DICT_ARROW_TOP,
    IMG_DICT_ARROW_DOWN,
    IMG_DICT_SEND,

    IMG_DELETE_LOGS,
    IMG_DOWNLOAD_LOG,
    IMG_SEARCH,

    /* weight 图标*/
    IMG_WIDGET_CHART,
    IMG_WIDGET_LED,
    IMG_WIDGET_SLIDER,
    IMG_WIDGET_BTN,
    IMG_WIDGET_SWEITCH,
    IMG_ARRANGEMENT,

    IMG_TIPS_CLOSE,
    IMG_TIPS_WARNING,
    IMG_DETALS,

    IMG_DASHBOARD_DELETE,

    IMG_EYES_OPEN,
    IMG_EYES_OFF,

    IMG_DICT_SEARCH,
    IMG_CNT
};

typedef enum {
    SMP_THEME_LIGHT = 0,
    SMP_THEME_DARK,
    SMP_THEME_CNT
} smp_theme_t;

typedef struct {
    lv_obj_t *show_content;                                                 // 该区域是用于控制页面切换
    lv_obj_t *menu;                                                         // 二级菜单对象
    lv_obj_t *activity_content;                                             // 活动内容显示区对象

    uint8_t is_need_menu : 1;
    uint8_t cur_page_is_hidden : 1;                                         // 用于表征当前页面是否隐藏，给一些需要刷新显示的页面使用 TRUE --> 已隐藏
    uint8_t reverse : 6;
}smp_page_t;

typedef struct {
    lv_color_t bg;
    lv_color_t text;
    lv_color_t border;
} smp_theme_dsc_t;

typedef struct
{
    lv_obj_t *icons_obj;                                                        // 存储导航栏图标对象指针
    char     icon_name[64];                                                            // 存储导航栏图标名称
} icons_obj_ctx_t;

typedef struct
{
    lv_image_dsc_t *imgs[IMG_CNT];                                              // 存储全部的图片
    struct
    {
        icons_obj_ctx_t *nav_icon[NAV_ICON_OBJ_CNT];                            // 导航栏图标相关数据
        Nav_Page_t cur_selected_page;                                           // 当前选中的页面
        
        
        lv_obj_t *smp_content_area;                                             // 内容布局区对象, 该区域包含菜单和活动区域
        lv_obj_t *top_bar_label;                                                // 顶部栏标题标签对象
        lv_obj_t *contrl_btn_min_max;
        lv_obj_t *setting_panel;
    } *ui;                                                                      // ui对象指针

    smp_page_t* pages[NAV_PAGE_CNT];
    lv_sdl_window_t* window_data; 

#if THEME_CHANGE_ENABLE
    lv_subject_t  theme_subject;  
    smp_theme_dsc_t theme[SMP_THEME_CNT];
#endif

} smp_ctx_t;


#endif /*SMP_PRIVATE_H*/

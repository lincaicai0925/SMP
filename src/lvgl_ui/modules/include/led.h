#ifndef LED_H
#define LED_H

#include "lvgl.h"
#include "cjson/cJSON.h"

#define LED_NAME_MAX_LEN    32


/* ---- 运行时配置（可被弹窗编辑、被 Save/Load 读写） ---- */
typedef struct {
    char        name[LED_NAME_MAX_LEN];   /* 显示名称 */
    uint32_t    vaddr;                    /* 虚拟地址（绑定数据源） */
    lv_color_t  color;                    /* LED 颜色 */
} led_conf_t;

/* ---- 创建参数（传给 Create_Led） ---- */
typedef struct {
    led_conf_t  conf;
    uint32_t    size;           /* LED 直径 (px) */
    uint8_t     brightness;     /* 初始亮度 0~255 */
} led_param_dsc_t;

/* ---- 运行时描述符（Create_Led 返回，存入 widget_dsc_t） ---- */
typedef struct {
    lv_obj_t   *led;            /* LED 控件对象 */
    lv_obj_t   *label;  
    led_conf_t  conf;           /* 运行时副本 */
} led_dsc_t;


static led_param_dsc_t def_led_param = {
    .conf = {
        .name  = "LED",
        .vaddr = 0,
        .color = {0},      
    },
    .size       = 40,
    .brightness = 255,
};

/* ---- 公开接口 ---- */
led_dsc_t*  Create_Led(lv_obj_t *parent, const led_param_dsc_t *param);
void        Led_Save(cJSON *item, void *widget_dsc);
void        Led_Load(void *widget_dsc, cJSON *item);

#endif /* LED_H */
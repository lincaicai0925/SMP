#ifndef BUTTON_H
#define BUTTON_H

#include "lvgl.h"
#include "cjson/cJSON.h"

lv_obj_t* Create_Button(lv_obj_t* base_obj, uint32_t width, uint32_t height, const char* label_text);

/* ---- Dashboard 按钮控件 ---- */
#define BTN_NAME_MAX_LEN    128

typedef struct {
    char        name[BTN_NAME_MAX_LEN];
    uint32_t    vaddr;
} btn_conf_t;

typedef struct {
    btn_conf_t  conf;
} btn_param_dsc_t;

typedef struct {
    lv_obj_t   *btn;
    lv_obj_t   *label;
    btn_conf_t  conf;
} btn_dsc_t;

static btn_param_dsc_t def_btn_param = {
    .conf = {
        .name  = "按钮",
        .vaddr = 0,
    },
};

btn_dsc_t*  Create_Button_Widget(lv_obj_t *parent, const btn_param_dsc_t *param);
void        Btn_Save(cJSON *item, void *widget_dsc);
void        Btn_Load(void *widget_dsc, cJSON *item);

#endif /* BUTTON_H */

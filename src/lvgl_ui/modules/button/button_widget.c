#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "modules/include/button.h"
#include "modules/pop/vaddr_dialog.h"

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

static lv_style_transition_dsc_t s_btn_trans;
static lv_style_prop_t s_btn_trans_props[] = {LV_STYLE_BG_COLOR, LV_STYLE_BG_OPA, 0};
static lv_style_t s_btn_style;
static bool s_style_inited = false;

static void init_btn_style(void)
{
    if(s_style_inited) return;
    lv_style_transition_dsc_init(&s_btn_trans, s_btn_trans_props,
                                  lv_anim_path_ease_in_out, 180, 0, NULL);
    lv_style_init(&s_btn_style);
    lv_style_set_transition(&s_btn_style, &s_btn_trans);
    s_style_inited = true;
}

static void btn_config_confirmed(uint32_t vaddr, const char *name, void *user_data)
{
    btn_dsc_t *dsc = (btn_dsc_t *)user_data;
    dsc->conf.vaddr = vaddr;
    if(name) {
        strncpy(dsc->conf.name, name, BTN_NAME_MAX_LEN - 1);
        dsc->conf.name[BTN_NAME_MAX_LEN - 1] = '\0';
        lv_label_set_text(dsc->label, dsc->conf.name);
    }
}

static void Btn_Delete_Cb(lv_event_t *e)
{
    btn_dsc_t *dsc = lv_event_get_user_data(e);
    if(dsc) lv_free(dsc);
}

static void Btn_Config_Event_Cb(lv_event_t *e)
{
    btn_dsc_t *dsc = lv_event_get_user_data(e);
    Vaddr_Dialog_Open_Ex(lv_event_get_target(e),
                         dsc->conf.vaddr,
                         dsc->conf.name,
                         BTN_NAME_MAX_LEN - 1,
                         btn_config_confirmed, dsc);
}

btn_dsc_t* Create_Button_Widget(lv_obj_t *parent, const btn_param_dsc_t *param)
{
    if(!parent || !param) return NULL;

    btn_dsc_t *dsc = lv_malloc(sizeof(btn_dsc_t));
    if(!dsc) return NULL;
    lv_memzero(dsc, sizeof(btn_dsc_t));

    memcpy(&dsc->conf, &param->conf, sizeof(btn_conf_t));

    init_btn_style();

    /* 容器 */
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);

    /* 按钮 */
    dsc->btn = lv_button_create(container);
    lv_obj_set_size(dsc->btn, lv_pct(100), lv_pct(100));
    lv_obj_add_flag(dsc->btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(dsc->btn, lv_color_hex(0x216883),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dsc->btn, LV_OPA_COVER,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dsc->btn, lv_color_hex(0x00C8FF),
                              LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(dsc->btn, LV_OPA_COVER,
                            LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(dsc->btn, &s_btn_style, LV_PART_MAIN);
    lv_obj_set_style_radius(dsc->btn, 8, LV_PART_MAIN);
    lv_obj_remove_flag(dsc->btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    /* 按钮上的名称标签 */
    dsc->label = lv_label_create(dsc->btn);
    lv_label_set_text(dsc->label, dsc->conf.name);
    lv_obj_set_style_text_color(dsc->label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(dsc->label, &lv_font_founder_kaiti_simplified_16,
                               LV_PART_MAIN);
    lv_obj_center(dsc->label);

    /* 长按打开设置弹窗（名称+虚拟地址） */
    lv_obj_add_event_cb(dsc->btn, Btn_Config_Event_Cb,
                        LV_EVENT_LONG_PRESSED, dsc);

    lv_obj_add_event_cb(container, Btn_Delete_Cb, LV_EVENT_DELETE, dsc);
    return dsc;
}

void Btn_Save(cJSON *item, void *widget_dsc)
{
    if(!item || !widget_dsc) return;
    btn_dsc_t *dsc = (btn_dsc_t *)widget_dsc;

    strncpy(dsc->conf.name, lv_label_get_text(dsc->label), BTN_NAME_MAX_LEN - 1);
    dsc->conf.name[BTN_NAME_MAX_LEN - 1] = '\0';
    cJSON_AddStringToObject(item, "btn_name",  dsc->conf.name);
    cJSON_AddNumberToObject(item, "btn_vaddr", dsc->conf.vaddr);
}

void Btn_Load(void *widget_dsc, cJSON *item)
{
    if(!widget_dsc || !item) return;
    btn_param_dsc_t *p = (btn_param_dsc_t *)widget_dsc;

    cJSON *jname  = cJSON_GetObjectItem(item, "btn_name");
    cJSON *jvaddr = cJSON_GetObjectItem(item, "btn_vaddr");

    if(cJSON_IsString(jname)) {
        strncpy(p->conf.name, jname->valuestring, BTN_NAME_MAX_LEN - 1);
        p->conf.name[BTN_NAME_MAX_LEN - 1] = '\0';
    }

    if(cJSON_IsNumber(jvaddr)) {
        p->conf.vaddr = (uint32_t)jvaddr->valueint;
    }
}

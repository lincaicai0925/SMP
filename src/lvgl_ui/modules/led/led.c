#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "modules/include/led.h"
#include "modules/include/widght_setting_pop.h"
#include "led_config_dialog.h"

static void Led_Delete_Cb(lv_event_t *e)
{
    led_dsc_t *dsc = lv_event_get_user_data(e);
    if(dsc) lv_free(dsc);
}

static void Led_Config_Event_Cb(lv_event_t *e)
{
    led_dsc_t *dsc = lv_event_get_user_data(e);
    Led_Vaddr_Dialog_Open(lv_event_get_target(e), dsc, NULL, NULL);
}


led_dsc_t* Create_Led(lv_obj_t *parent, const led_param_dsc_t *param)
{
    if (!parent || !param) return NULL;

    led_dsc_t *dsc = lv_malloc(sizeof(led_dsc_t));
    if (!dsc) return NULL;
    lv_memzero(dsc, sizeof(led_dsc_t));

    /* 保存运行时副本 */
    memcpy(&dsc->conf, &param->conf, sizeof(led_conf_t));

    lv_color_t color = param->conf.color;
    uint32_t   size  = param->size;

    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);


    /* 创建 LED 对象 */
    dsc->led = lv_led_create(container); 
    if (!dsc->led) {
        lv_free(dsc);
        return NULL;
    }

    if (color.red == 0 && color.green == 0 && color.blue == 0) {
        static uint32_t s_color_seed = 0;
        srand((unsigned)(time(NULL) ^ (++s_color_seed * 2654435761u)));
        color = lv_color_make(rand() % 256, rand() % 256, rand() % 256);
        dsc->conf.color = color;
    }

    lv_obj_set_size(dsc->led, size, size);
    lv_led_set_color(dsc->led, color);
    lv_led_set_brightness(dsc->led, param->brightness);

    /* 样式 */
    lv_obj_set_style_radius(dsc->led, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dsc->led, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(dsc->led, color, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(dsc->led, size, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(dsc->led, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(dsc->led, LV_ALIGN_CENTER, 0, -10);

    /* 名称标签（居中在 LED 下方） */
    dsc->label = lv_label_create(container);
    lv_label_set_text(dsc->label, dsc->conf.name);
    lv_obj_set_style_text_color(dsc->label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(dsc->label, lv_obj_get_style_text_font(parent, 0), 0);
    lv_obj_align(dsc->label, LV_ALIGN_CENTER, 0, 30);
    lv_obj_add_flag(dsc->label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(dsc->label, Label_Set_Name,LV_EVENT_CLICKED, NULL);

    lv_obj_add_flag(dsc->led, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(dsc->led, Led_Config_Event_Cb, LV_EVENT_CLICKED, dsc);

    lv_obj_add_event_cb(container, Led_Delete_Cb, LV_EVENT_DELETE, dsc);
    return dsc;
}

void Led_Save(cJSON *item, void *widget_dsc)
{
    if (!item || !widget_dsc) return;
    led_dsc_t *dsc = (led_dsc_t *)widget_dsc;

    const char *text = lv_label_get_text(dsc->label);
    if(text) {
        strncpy(dsc->conf.name, text, LED_NAME_MAX_LEN - 1);
        dsc->conf.name[LED_NAME_MAX_LEN - 1] = '\0';
    }
    cJSON_AddStringToObject(item, "led_name",  dsc->conf.name);
    cJSON_AddNumberToObject(item, "led_vaddr", dsc->conf.vaddr);

    /* 颜色存为 24bit 整数 */
    uint32_t c = ((uint32_t)dsc->conf.color.red   << 16)
           | ((uint32_t)dsc->conf.color.green  << 8)
           |  (uint32_t)dsc->conf.color.blue;
    cJSON_AddNumberToObject(item, "led_color", c);
}

void Led_Load(void *widget_dsc, cJSON *item)
{
    if (!widget_dsc || !item) return;
    led_param_dsc_t *p = (led_param_dsc_t *)widget_dsc;

    cJSON *jname  = cJSON_GetObjectItem(item, "led_name");
    cJSON *jvaddr = cJSON_GetObjectItem(item, "led_vaddr");
    cJSON *jcolor = cJSON_GetObjectItem(item, "led_color");

    if (cJSON_IsString(jname)) {
        strncpy(p->conf.name, jname->valuestring, LED_NAME_MAX_LEN - 1);
        p->conf.name[LED_NAME_MAX_LEN - 1] = '\0';
    }

    if (cJSON_IsNumber(jvaddr)) {
        p->conf.vaddr = (uint32_t)jvaddr->valueint;
    }

    if (cJSON_IsNumber(jcolor)) {
        uint32_t c = (uint32_t)jcolor->valueint;
        p->conf.color = lv_color_make((c >> 16) & 0xFF,
                                      (c >> 8)  & 0xFF,
                                       c        & 0xFF);
    }
}
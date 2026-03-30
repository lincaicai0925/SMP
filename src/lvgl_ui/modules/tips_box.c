#include "lvgl.h"
#include "modules/include/tips_box.h"

#define TIPS_BOX_WIDTH       500
#define TIPS_BOX_AUTO_CLOSE  5000  /* ms */

static lv_obj_t *tips_container = NULL;

static void reset_container_style(lv_obj_t *obj)
{
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void Toast_Delete_Cb(lv_event_t *e)
{
    lv_obj_t *toast = lv_event_get_target(e);
    lv_timer_t *timer = (lv_timer_t *)lv_obj_get_user_data(toast);
    if (timer) {
        lv_timer_delete(timer);
        lv_obj_set_user_data(toast, NULL);
    }
}


static void Auto_Close_Tips_Cb(lv_timer_t *timer)
{
    lv_obj_t *toast = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (toast) {
        lv_obj_set_user_data(toast, NULL);   /* 断开关联，防止递归 */
        lv_obj_delete(toast);
    }
}

static void ensure_tips_container(void)
{
    if (tips_container && lv_obj_is_valid(tips_container)) {
        return;
    }

    tips_container = lv_obj_create(lv_layer_top());
    lv_obj_set_size(tips_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(tips_container, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_set_flex_flow(tips_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tips_container,
        LV_FLEX_ALIGN_START,     /* main axis: 从上往下排 */
        LV_FLEX_ALIGN_CENTER,    /* cross axis: 水平居中  */
        LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(tips_container, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_row(tips_container, 6, LV_PART_MAIN);

    reset_container_style(tips_container);
    lv_obj_remove_flag(tips_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(tips_container, LV_OBJ_FLAG_IGNORE_LAYOUT);
}


void Create_Tips_Box(const char *msg, uint32_t color)
{
    ensure_tips_container();

    /* ---- toast 本体 ---- */
    lv_obj_t *toast = lv_obj_create(tips_container);
    lv_obj_set_size(toast, TIPS_BOX_WIDTH, LV_SIZE_CONTENT);
    lv_obj_remove_flag(toast, LV_OBJ_FLAG_SCROLLABLE);

    /* 背景 */
    lv_obj_set_style_bg_color(toast, lv_color_hex(0x2B2B2B), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(toast, 8, LV_PART_MAIN);

    /* 边框 */
    lv_obj_set_style_border_width(toast, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(toast, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_border_opa(toast, LV_OPA_60, LV_PART_MAIN);

    /* 阴影 */
    lv_obj_set_style_shadow_width(toast, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(toast, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(toast, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(toast, 4, LV_PART_MAIN);

    /* 内边距 */
    lv_obj_set_style_pad_all(toast, 14, LV_PART_MAIN);

    /* ---- 消息文本 ---- */
    lv_obj_t *label = lv_label_create(toast);
    lv_obj_set_width(label, LV_PCT(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, msg);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(label, 4, LV_PART_MAIN);

    /* ---- 5 秒自动销毁 ---- */
    lv_timer_t *timer = lv_timer_create(Auto_Close_Tips_Cb,
                                        TIPS_BOX_AUTO_CLOSE, toast);
    lv_timer_set_repeat_count(timer, 1);

    /* timer 存到 toast 的 user_data，方便 DELETE 事件清理 */
    lv_obj_set_user_data(toast, timer);
    lv_obj_add_event_cb(toast, Toast_Delete_Cb, LV_EVENT_DELETE, NULL);
}
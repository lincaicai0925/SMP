#include "modules/include/button.h"

static lv_style_transition_dsc_t s_btn_trans;
static lv_style_prop_t s_btn_trans_props[] = {LV_STYLE_BG_COLOR, LV_STYLE_BG_OPA, 0};
static lv_style_t s_btn_style;

lv_obj_t* Create_Button(lv_obj_t* base_obj, uint32_t width, uint32_t height, const char* label_text)
{
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    // 初始化按钮样式和动画（只初始化一次）
    static bool s_btn_trans_inited = false;
    if (!s_btn_trans_inited) {
        lv_style_transition_dsc_init(&s_btn_trans, s_btn_trans_props, lv_anim_path_ease_in_out, 180, 0, NULL);
        lv_style_init(&s_btn_style);
        lv_style_set_transition(&s_btn_style, &s_btn_trans);
        s_btn_trans_inited = true;
    }

    lv_obj_t* btn = lv_button_create(base_obj);
    lv_obj_set_size(btn, width, height);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x216883), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x00C8FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(btn, &s_btn_style, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);

    return btn;
}

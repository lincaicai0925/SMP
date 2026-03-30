#include "modules/include/checkbox.h"


lv_obj_t* Create_Checkbox(lv_obj_t* base_obj, const char* name)
{
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    // 复选框
    lv_obj_t * checkbox = lv_checkbox_create(base_obj);
    lv_obj_remove_flag(checkbox, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_checkbox_set_text_static(checkbox, name);
    lv_obj_set_style_text_color(checkbox, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(checkbox, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_pad_column(checkbox, 8, LV_PART_MAIN);
    lv_obj_center(checkbox);
    // 复选框背景样式
    lv_obj_set_style_bg_color(checkbox, lv_color_hex(0x2E3A42), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(checkbox, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(checkbox, 2, LV_PART_INDICATOR);
    lv_obj_set_style_border_color(checkbox, lv_color_hex(0x3ABCE4), LV_PART_INDICATOR);
    lv_obj_set_style_border_opa(checkbox, LV_OPA_50, LV_PART_INDICATOR);

    // 复选框选中状态背景样式
    lv_obj_set_style_bg_color(checkbox, lv_color_hex(0x3b82f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(checkbox, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(checkbox, lv_color_hex(0x3b82f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(checkbox, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(checkbox, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_image_src(checkbox, LV_SYMBOL_OK, LV_PART_INDICATOR | LV_STATE_CHECKED);

    return checkbox;
}

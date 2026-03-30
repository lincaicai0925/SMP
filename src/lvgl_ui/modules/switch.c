#include "modules/include/switch.h"

lv_obj_t* Create_Switch(lv_obj_t* base_obj, uint32_t width, uint32_t height)
{
    lv_obj_t* sw = lv_switch_create(base_obj);
    lv_obj_set_size(sw, width, height);
    lv_obj_remove_flag(sw, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xD9D9D9), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sw, 2, LV_PART_MAIN);
    lv_obj_set_style_border_width(sw, 0, LV_PART_MAIN);

    lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(sw, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x15b1e0), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_pad_all(sw, 2, LV_PART_KNOB);

    lv_obj_set_style_anim_duration(sw, 120, LV_PART_MAIN);
    return sw;
}

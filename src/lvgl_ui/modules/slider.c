#include "modules/include/slider.h"

static void Slider_Event_Cb(lv_event_t * e);

lv_obj_t* Create_Slider(lv_obj_t* base_obj, const char* name)
{
    LV_FONT_DECLARE(lv_font_digital_7_24);
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

    lv_obj_t* container = lv_obj_create(base_obj);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN); 
    lv_obj_set_style_pad_column(container, 15, LV_PART_MAIN);  
    lv_obj_set_style_pad_row(container, 0, LV_PART_MAIN);
    lv_obj_center(container);
    
    lv_obj_t * name_label = lv_label_create(container);
    lv_obj_set_style_text_font(name_label, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_label_set_text(name_label, name);
    lv_obj_set_width(name_label, LV_SIZE_CONTENT);
    lv_obj_set_height(name_label, 24); 
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_pad_left(name_label, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_right(name_label, 5, LV_PART_MAIN);
    lv_obj_set_flex_grow(name_label, 0);
    lv_obj_set_style_pad_top(name_label, 3, LV_PART_MAIN);

    // 滑块
    lv_obj_t * slider = lv_slider_create(container);
    lv_obj_set_style_max_height(slider, 5, LV_PART_MAIN);
    lv_obj_set_flex_grow(slider, 1);
    lv_obj_remove_flag(slider, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    
    // 主轨道样式
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2E3A42), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(slider, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(slider, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(slider, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(slider, 2, LV_PART_MAIN);

    // 指示器样式
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x00C8FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_width(slider, 10, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_color(slider, lv_color_hex(0x00C8FF), LV_PART_INDICATOR);
    lv_obj_set_style_shadow_opa(slider, LV_OPA_50, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_spread(slider, 3, LV_PART_INDICATOR);

    // 滑块样式
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x00C8FF), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 6, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(slider, 10, LV_PART_KNOB);
    lv_obj_set_style_shadow_color(slider, lv_color_hex(0x00C8FF), LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(slider, LV_OPA_70, LV_PART_KNOB);
    lv_obj_set_style_shadow_spread(slider, 2, LV_PART_KNOB);

    lv_obj_set_style_anim_duration(slider, 2000, 0);

    // 数值标签
    lv_obj_t *slider_label = lv_label_create(container);
    lv_label_set_text(slider_label, "0");
    lv_obj_set_width(slider_label, LV_SIZE_CONTENT);
    lv_obj_set_height(slider_label, 24);  
    lv_obj_set_style_text_font(slider_label, &lv_font_digital_7_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(slider_label, lv_color_hex(0x3ABCE4), LV_PART_MAIN);
    lv_obj_set_style_min_width(slider_label, 30, LV_PART_MAIN);
    lv_obj_set_flex_grow(slider_label, 0);
    lv_obj_set_style_pad_top(slider_label, 3, LV_PART_MAIN);

    lv_obj_add_event_cb(slider, Slider_Event_Cb, LV_EVENT_VALUE_CHANGED, slider_label);
    return slider;
}

static void Slider_Event_Cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_obj_t * slider_label = lv_event_get_user_data(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", (int)lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf);
}
#include "modules/include/arc.h"

typedef struct {
    lv_obj_t* outer_arc;
    lv_obj_t* label;
} arc_user_data_t;

static void Arc_Value_Changed_Cb(lv_event_t * e);

lv_obj_t* Create_Arc(lv_obj_t* base_obj, uint32_t size)
{
    if(size < 80)   size = 80;
    LV_FONT_DECLARE(lv_font_digital_7_36);
    lv_obj_t* arc_container = lv_obj_create(base_obj);
    lv_obj_set_style_bg_opa(arc_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_size(arc_container, size, size); 
    lv_obj_set_style_pad_all(arc_container, 0, LV_PART_MAIN);
    
    lv_obj_t* arc_outside = lv_arc_create(arc_container);
    lv_obj_center(arc_outside);
    lv_obj_set_size(arc_outside, size-15, size-15);
    lv_arc_set_bg_angles(arc_outside, 0, 360);
    lv_arc_set_rotation(arc_outside, 135);
    lv_arc_set_bg_angles(arc_outside, 0, 270);
    lv_arc_set_angles(arc_outside, 0, 270);
    lv_obj_set_style_arc_width(arc_outside, 5, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_outside, 5, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc_outside, true, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arc_outside, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_outside, lv_color_hex(0x00C8FF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_outside, lv_color_hex(0x2E3A42), LV_PART_MAIN);

    lv_obj_set_style_bg_color(arc_outside, lv_color_hex(0x00C8FF), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(arc_outside, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_radius(arc_outside, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(arc_outside, 5, LV_PART_KNOB);
    lv_obj_set_style_shadow_color(arc_outside, lv_color_hex(0x7BD7FF), LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc_outside, LV_OPA_50, LV_PART_KNOB);
    lv_obj_set_style_shadow_spread(arc_outside, 3, LV_PART_KNOB);

    lv_obj_t* arc_label = lv_obj_create(arc_outside);
    lv_obj_set_size(arc_label, size-20, 30);
    lv_obj_align(arc_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(arc_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_remove_flag(arc_label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* label = lv_label_create(arc_label);
    lv_obj_set_style_text_font(label, &lv_font_digital_7_36, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0x3ABCE4), LV_PART_MAIN);
    lv_obj_center(label);
    lv_label_set_text(label, "72");

    lv_obj_add_event_cb(arc_outside, Arc_Value_Changed_Cb, LV_EVENT_VALUE_CHANGED, label);

    return arc_outside;
}

static void Arc_Value_Changed_Cb(lv_event_t * e)
{
    lv_obj_t * arc = lv_event_get_target(e);
    lv_obj_t * label = lv_event_get_user_data(e);

    int32_t value = lv_arc_get_value(arc);
    lv_label_set_text_fmt(label, "%d", (int)value);
}

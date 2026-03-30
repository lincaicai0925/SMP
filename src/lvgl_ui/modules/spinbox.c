#include "modules/include/spinbox.h"

static void Spinbox_Increment_Event_Cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_obj_t * spinbox = lv_event_get_user_data(e);
        if(spinbox) {
            lv_spinbox_increment(spinbox);
        }
    }
}

static void Spinbox_Decrement_Event_Cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_obj_t * spinbox = lv_event_get_user_data(e);
        if(spinbox) {
            lv_spinbox_decrement(spinbox);
        }
    }
}

void Create_Spinbox(lv_obj_t* base_obj, uint32_t width, uint32_t height)
{
    lv_obj_t * spinbox = lv_spinbox_create(base_obj);
    lv_spinbox_set_range(spinbox, -1000, 25000);
    lv_spinbox_set_digit_format(spinbox, 3, 2);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_width(spinbox, 100);
    lv_obj_center(spinbox);

    lv_obj_set_style_bg_color(spinbox, lv_color_hex(0x2E3A42), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(spinbox, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(spinbox, lv_color_hex(0x00C8FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(spinbox, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(spinbox, 6, LV_PART_MAIN);
    lv_obj_set_style_text_color(spinbox, lv_color_hex(0x3ABCE4), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(spinbox, 0, LV_PART_MAIN);

    int32_t h = lv_obj_get_height(spinbox);

    lv_obj_t * btn = lv_button_create(base_obj);
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_image_src(btn, LV_SYMBOL_PLUS, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E3A42), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x00C8FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_hex(0x3ABCE4), LV_PART_MAIN);
    lv_obj_add_event_cb(btn, Spinbox_Increment_Event_Cb, LV_EVENT_ALL, spinbox);

    btn = lv_button_create(base_obj);
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_image_src(btn, LV_SYMBOL_MINUS, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E3A42), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x00C8FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_hex(0x3ABCE4), LV_PART_MAIN);
    lv_obj_add_event_cb(btn, Spinbox_Decrement_Event_Cb, LV_EVENT_ALL, spinbox);
}

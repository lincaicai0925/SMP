#include "modules/include/textarea.h"
#include "clabez/include/cdpi.h"

static void Ta_Focused_Cb(lv_event_t * e)
{
    lv_obj_t * ta = lv_event_get_target(e);
    lv_display_t * disp = lv_obj_get_display(ta);
    if(disp == NULL) return;

    /* 获取 LVGL 逻辑坐标 */
    lv_area_t coords;
    lv_obj_get_coords(ta, &coords);

    uint8_t zoom = lv_sdl_window_get_zoom(disp);
    SDL_Rect rect;
    rect.x = (int)(coords.x1 * zoom);
    rect.y = (int)(coords.y2 * zoom);  
    rect.w = (int)((coords.x2 - coords.x1) * zoom);
    rect.h = (int)((coords.y2 - coords.y1) * zoom);

    SDL_SetTextInputRect(&rect);
    SDL_StartTextInput();
}

lv_obj_t* Create_Textarea(lv_obj_t* base_obj, uint32_t width, uint32_t height, const char* placeholder)
{
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    lv_obj_t* textarea = lv_textarea_create(base_obj);
    lv_obj_set_size(textarea, width, height);
    lv_textarea_set_placeholder_text(textarea, placeholder);
    lv_obj_set_style_text_font(textarea, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(textarea, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_pad_all(textarea, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0x1F2A33), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(textarea, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(textarea, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(textarea, lv_color_hex(0x3A4A55), LV_PART_MAIN);
    lv_obj_set_style_border_opa(textarea, LV_OPA_COVER, LV_PART_MAIN);
    lv_textarea_set_one_line(textarea, true);
    lv_obj_set_style_text_color(textarea, lv_color_hex(0x6C7A86), LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_border_color(textarea, lv_color_hex(0x00C8FF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(textarea, LV_OPA_50, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_translate_x(textarea, 5, LV_PART_CURSOR); 
    lv_obj_remove_flag(textarea, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0x1565C0), LV_PART_SELECTED);
    lv_textarea_set_text_selection(textarea, true);

    lv_obj_add_event_cb(textarea, Ta_Focused_Cb, LV_EVENT_FOCUSED, NULL); 
    return textarea;
}

#include "modules/include/spinner.h"

lv_obj_t* Create_Spinner(lv_obj_t* base_obj, uint16_t size, uint32_t anim_time)
{
    lv_obj_t* spinner = lv_spinner_create(base_obj);
    lv_obj_set_size(spinner, size, size);
    lv_spinner_set_anim_params(spinner, anim_time, 200);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x2E3A42), LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x00C8FF), LV_PART_INDICATOR);
    return spinner;
}

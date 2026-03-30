#include "modules/include/styles.h"

void Set_Card_Style(lv_obj_t* obj)
{
    lv_obj_set_style_radius(obj, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, CART_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(obj, SHADOW_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, GAP_BORDER_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(obj, LV_OPA_10, LV_STATE_DEFAULT);
    
    lv_obj_set_style_pad_all(obj, 10, LV_PART_MAIN);
}

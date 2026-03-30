#include "modules/include/dropdown.h"

#define DROPDOWN_LIST_ANIM_TIME_MS 140
#define DROPDOWN_LIST_ANIM_SCALE_START 180
#define DROPDOWN_LIST_ANIM_SCALE_END 256

static lv_style_t list_style;
static lv_style_t list_selected_style;

static const char * Dropdown_Get_Symbol(lv_dir_t dir, bool opened)
{
    switch(dir) {
        case LV_DIR_TOP:
            return opened ? LV_SYMBOL_DOWN : LV_SYMBOL_UP;
        case LV_DIR_LEFT:
            return opened ? LV_SYMBOL_RIGHT : LV_SYMBOL_LEFT;
        case LV_DIR_RIGHT:
            return opened ? LV_SYMBOL_LEFT : LV_SYMBOL_RIGHT;
        case LV_DIR_BOTTOM:
        default:
            return opened ? LV_SYMBOL_UP : LV_SYMBOL_DOWN;
    }
}

static void Dropdown_List_Set_Opa(void * obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, LV_PART_MAIN);
}

static void Dropdown_List_Set_Scale_y(void * obj, int32_t v)
{
    lv_obj_set_style_transform_scale_y((lv_obj_t *)obj, v, LV_PART_MAIN);
}

static void Dropdown_List_Set_Pivot(lv_obj_t * list, lv_obj_t * dropdown)
{
    if(lv_obj_get_y(list) >= lv_obj_get_y(dropdown)) {
        lv_obj_set_style_transform_pivot_y(list, 0, LV_PART_MAIN);
    }
    else {
        lv_obj_set_style_transform_pivot_y(list, lv_obj_get_height(list), LV_PART_MAIN);
    }
    lv_obj_set_style_transform_pivot_x(list, 0, LV_PART_MAIN);
}

static void Dropdown_Start_List_Anim(lv_obj_t * dropdown)
{
    lv_obj_t * list = lv_dropdown_get_list(dropdown);
    if(list == NULL) return;

    Dropdown_List_Set_Pivot(list, dropdown);

    lv_anim_delete(list, Dropdown_List_Set_Opa);
    lv_anim_delete(list, Dropdown_List_Set_Scale_y);

    lv_obj_set_style_opa(list, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_transform_scale_y(list, DROPDOWN_LIST_ANIM_SCALE_START, LV_PART_MAIN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, list);
    lv_anim_set_time(&a, DROPDOWN_LIST_ANIM_TIME_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    lv_anim_set_exec_cb(&a, Dropdown_List_Set_Opa);
    lv_anim_set_values(&a, LV_OPA_0, LV_OPA_COVER);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, Dropdown_List_Set_Scale_y);
    lv_anim_set_values(&a, DROPDOWN_LIST_ANIM_SCALE_START, DROPDOWN_LIST_ANIM_SCALE_END);
    lv_anim_start(&a);
}

static void Dropdown_Event_Cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_READY) {
        Dropdown_Start_List_Anim(obj);
        lv_dropdown_set_symbol(obj, Dropdown_Get_Symbol(lv_dropdown_get_dir(obj), true));
    }
    else if(code == LV_EVENT_CANCEL) {
        lv_obj_t * list = lv_dropdown_get_list(obj);
        if(list) {
            lv_anim_delete(list, Dropdown_List_Set_Opa);
            lv_anim_delete(list, Dropdown_List_Set_Scale_y);
        }
        lv_dropdown_set_symbol(obj, Dropdown_Get_Symbol(lv_dropdown_get_dir(obj), false));
    }
}

lv_obj_t* Create_Dropdown(lv_obj_t *base_obj, const char *options)
{
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

    static bool styles_inited = false;
    if(!styles_inited) {
        lv_style_init(&list_style);
        lv_style_set_bg_color(&list_style, lv_color_hex(0x162027));
        lv_style_set_bg_opa(&list_style, LV_OPA_COVER);
        lv_style_set_text_color(&list_style, lv_color_hex(0xE8F0F5));
        lv_style_set_text_font(&list_style, &lv_font_founder_kaiti_simplified_16);
        lv_style_set_pad_left(&list_style, 5);
        lv_style_set_pad_right(&list_style, 5);
        lv_style_set_max_height(&list_style, 100);

        lv_style_init(&list_selected_style);
        lv_style_set_bg_color(&list_selected_style, lv_color_hex(0x00c8ff));
        lv_style_set_text_color(&list_selected_style, lv_color_hex(0x1A1A1A));   
        styles_inited = true;
    }

    // 创建下拉框
    lv_obj_t * dd = lv_dropdown_create(base_obj);
    lv_dropdown_set_options(dd, options);
    lv_dropdown_set_symbol(dd, Dropdown_Get_Symbol(lv_dropdown_get_dir(dd), false));
    lv_obj_remove_flag(dd, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    // 下拉框主体样式
    lv_obj_set_style_text_color(dd, lv_color_hex(0xE8F0F5), LV_PART_MAIN);
    lv_obj_set_style_text_font(dd, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(dd, lv_color_hex(0x1F2A33), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dd, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_left(dd, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_right(dd, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(dd, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(dd, lv_color_hex(0x4FB3C8), LV_PART_MAIN);
    lv_obj_set_style_border_opa(dd, LV_OPA_70, LV_PART_MAIN);

    lv_obj_add_event_cb(dd, Dropdown_Event_Cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(dd, Dropdown_Event_Cb, LV_EVENT_CANCEL, NULL);

    lv_obj_t * list = lv_dropdown_get_list(dd); 
    lv_obj_add_style(list, &list_style, LV_PART_MAIN);
    lv_obj_add_style(list, &list_selected_style, LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_add_style(list, &list_selected_style, LV_PART_SELECTED | LV_STATE_CHECKED);
        

    return dd;
}

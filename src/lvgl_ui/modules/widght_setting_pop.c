#include "modules/include/widght_setting_pop.h"
#include "modules/include/textarea.h"

static void Label_Name_Input_Cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);

    if(code == LV_EVENT_READY) {
        const char *txt = lv_textarea_get_text(ta);
        if(txt && strlen(txt) > 0) {
            lv_label_set_text(label, txt);
        }
        lv_obj_delete(ta);
    }
    else if(code == LV_EVENT_DEFOCUSED || code == LV_EVENT_CANCEL) {
        lv_obj_delete(ta);
    }
}

void Label_Set_Name(lv_event_t *e)
{
    lv_obj_t *label = lv_event_get_target(e);

    lv_area_t coords;
    lv_obj_get_coords(label, &coords);


    lv_obj_t *ta = Create_Textarea(lv_screen_active(), 100, 40, "按ESC关闭");
    lv_obj_set_pos(ta, coords.x2 + 4, coords.y1);  
    
    lv_obj_add_event_cb(ta, Label_Name_Input_Cb, LV_EVENT_READY, label);
    lv_obj_add_event_cb(ta, Label_Name_Input_Cb, LV_EVENT_DEFOCUSED, label);
    lv_obj_add_event_cb(ta, Label_Name_Input_Cb, LV_EVENT_CANCEL, label);

    lv_group_focus_obj(ta);
}
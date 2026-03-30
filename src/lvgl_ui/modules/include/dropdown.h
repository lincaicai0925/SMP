#ifndef DROPDOWN_H
#define DROPDOWN_H

#include "lvgl.h"

#define DROPDOWN_OPTIONS_MAX_LEN 256
lv_obj_t* Create_Dropdown(lv_obj_t *base_obj, const char *options);

#endif // DROPDOWN_H

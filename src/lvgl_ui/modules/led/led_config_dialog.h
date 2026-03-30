#ifndef LED_VADDR_POP_H
#define LED_VADDR_POP_H

#include "lvgl.h"
#include "modules/include/led.h"
#include "modules/pop/vaddr_dialog.h"

typedef void (*led_vaddr_cb_t)(led_dsc_t *dsc, void *user_data);

void Led_Vaddr_Dialog_Open(lv_obj_t *trigger,
                           led_dsc_t *dsc,
                           led_vaddr_cb_t on_confirm,
                           void *user_data);

#endif

#ifndef CHART_SETTING_POP_H
#define CHART_SETTING_POP_H

#include "lvgl.h"
#include "modules/include/chart.h"

typedef void (*series_dialog_cb_t)(chart_series_conf_t *conf, void *user_data);

void Series_Dialog_Open(lv_obj_t *trigger,  chart_series_conf_t *conf, series_dialog_cb_t on_confirm, void *user_data);
#endif /* CHART_SETTING_POP_H */



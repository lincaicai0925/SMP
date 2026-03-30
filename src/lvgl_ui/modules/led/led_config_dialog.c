#include "led_config_dialog.h"

typedef struct {
    led_dsc_t      *dsc;
    led_vaddr_cb_t  on_confirm;
    void           *cb_user_data;
} led_dlg_ctx_t;

static led_dlg_ctx_t s_ctx = {0};

static void led_vaddr_confirmed(uint32_t vaddr, const char *name, void *user_data)
{
    (void)user_data;
    (void)name;
    s_ctx.dsc->conf.vaddr = vaddr;
    if(s_ctx.on_confirm)
        s_ctx.on_confirm(s_ctx.dsc, s_ctx.cb_user_data);
}

void Led_Vaddr_Dialog_Open(lv_obj_t *trigger,
                           led_dsc_t *dsc,
                           led_vaddr_cb_t on_confirm,
                           void *user_data)
{
    if(!trigger || !dsc) return;

    s_ctx.dsc          = dsc;
    s_ctx.on_confirm   = on_confirm;
    s_ctx.cb_user_data = user_data;

    Vaddr_Dialog_Open(trigger, dsc->conf.vaddr, led_vaddr_confirmed, NULL);
}

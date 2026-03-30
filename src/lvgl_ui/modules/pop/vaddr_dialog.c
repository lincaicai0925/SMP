#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vaddr_dialog.h"
#include "modules/include/textarea.h"

typedef struct {
    lv_obj_t           *overlay;
    lv_obj_t           *dialog;
    lv_obj_t           *name_ta;       /* 名称输入框 (可选) */
    lv_obj_t           *addr_ta;
    lv_obj_t           *err_label;
    bool                open;
    vaddr_confirm_cb_t  on_confirm;
    void               *cb_user_data;
} vaddr_dlg_t;

static vaddr_dlg_t s_dlg = {0};

static void dlg_close(void)
{
    if(!s_dlg.open) return;
    if(s_dlg.overlay) {
        lv_obj_delete(s_dlg.overlay);
        s_dlg.overlay = NULL;
    }
    s_dlg.dialog    = NULL;
    s_dlg.name_ta   = NULL;
    s_dlg.addr_ta   = NULL;
    s_dlg.err_label = NULL;
    s_dlg.open      = false;
}

static void overlay_click_cb(lv_event_t *e)
{
    if(lv_event_get_target(e) == s_dlg.overlay)
        dlg_close();
}

static void btn_cancel_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    dlg_close();
}

static void btn_confirm_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    /* 校验名称 */
    const char *name_str = NULL;
    if(s_dlg.name_ta) {
        name_str = lv_textarea_get_text(s_dlg.name_ta);
        if(!name_str || strlen(name_str) == 0) {
            lv_label_set_text(s_dlg.err_label, "! 名称不能为空");
            lv_obj_remove_flag(s_dlg.err_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_color(s_dlg.name_ta, lv_color_hex(0xFF5252), 0);
            return;
        }
    }

    /* 校验地址 */
    const char *str = lv_textarea_get_text(s_dlg.addr_ta);
    if(!str || strlen(str) == 0) {
        lv_label_set_text(s_dlg.err_label, "! 地址不能为空");
        lv_obj_remove_flag(s_dlg.err_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_color(s_dlg.addr_ta, lv_color_hex(0xFF5252), 0);
        return;
    }

    char *endptr = NULL;
    long val = strtol(str, &endptr, 10);
    if(*endptr != '\0' || val < 0 || val > 65535) {
        lv_label_set_text(s_dlg.err_label, "! 地址无效 (0~65535)");
        lv_obj_remove_flag(s_dlg.err_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_color(s_dlg.addr_ta, lv_color_hex(0xFF5252), 0);
        return;
    }

    uint32_t vaddr = (uint32_t)val;
    vaddr_confirm_cb_t cb = s_dlg.on_confirm;
    void *ud = s_dlg.cb_user_data;

    /* 在关闭弹窗前拷贝名称，因为 dlg_close 会释放 textarea 内部缓冲区 */
    char name_buf[128];
    const char *name_out = NULL;
    if(name_str) {
        strncpy(name_buf, name_str, sizeof(name_buf) - 1);
        name_buf[sizeof(name_buf) - 1] = '\0';
        name_out = name_buf;
    }

    if(cb)
        cb(vaddr, name_out, ud);

    dlg_close();
}

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

/* 内部创建输入行的辅助函数 */
static lv_obj_t* create_input_row(lv_obj_t *parent, const char *label_text,
                                   int32_t ta_w, int32_t ta_h,
                                   const char *placeholder)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), 40);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 8, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, 0);

    lv_obj_t *ta = Create_Textarea(row, ta_w, ta_h, placeholder ? placeholder : "");

    return ta;
}

void Vaddr_Dialog_Open_Ex(lv_obj_t *trigger,
                          uint32_t  cur_vaddr,
                          const char *cur_name,
                          uint32_t  name_max_len,
                          vaddr_confirm_cb_t on_confirm,
                          void *user_data)
{
    if(!trigger) return;
    if(s_dlg.open) return;

    s_dlg.open         = true;
    s_dlg.on_confirm   = on_confirm;
    s_dlg.cb_user_data = user_data;
    s_dlg.name_ta      = NULL;

    bool has_name = (cur_name != NULL);

    /* ---- 1. 遮罩 ---- */
    s_dlg.overlay = lv_obj_create(lv_layer_top());
    lv_obj_set_size(s_dlg.overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(s_dlg.overlay, 0, 0);
    lv_obj_set_style_bg_color(s_dlg.overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_dlg.overlay, 120, 0);
    lv_obj_set_style_border_width(s_dlg.overlay, 0, 0);
    lv_obj_set_style_radius(s_dlg.overlay, 0, 0);
    lv_obj_remove_flag(s_dlg.overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(s_dlg.overlay, overlay_click_cb, LV_EVENT_CLICKED, NULL);

    /* ---- 2. 定位 ---- */
    lv_area_t trig_area;
    lv_obj_get_coords(trigger, &trig_area);

    int32_t dlg_w = 240;
    int32_t dlg_h = has_name ? 210 : 160;
    int32_t dlg_x = trig_area.x2 + 8;
    int32_t dlg_y = trig_area.y1;

    if(dlg_x + dlg_w > LV_HOR_RES - 8) {
        dlg_x = trig_area.x1 - dlg_w - 8;
        if(dlg_x < 8) dlg_x = (LV_HOR_RES - dlg_w) / 2;
    }
    if(dlg_y + dlg_h > LV_VER_RES - 8) {
        dlg_y = LV_VER_RES - dlg_h - 8;
        if(dlg_y < 8) dlg_y = 8;
    }

    /* ---- 3. 弹窗卡片 ---- */
    s_dlg.dialog = lv_obj_create(s_dlg.overlay);
    lv_obj_set_size(s_dlg.dialog, dlg_w, LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(s_dlg.dialog, dlg_h, 0);
    lv_obj_set_pos(s_dlg.dialog, dlg_x, dlg_y);
    lv_obj_set_style_bg_color(s_dlg.dialog, lv_color_hex(0x1e1e2e), 0);
    lv_obj_set_style_bg_opa(s_dlg.dialog, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_dlg.dialog, lv_color_hex(0x3a3a5c), 0);
    lv_obj_set_style_border_width(s_dlg.dialog, 1, 0);
    lv_obj_set_style_radius(s_dlg.dialog, 12, 0);
    lv_obj_set_style_shadow_width(s_dlg.dialog, 24, 0);
    lv_obj_set_style_shadow_color(s_dlg.dialog, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(s_dlg.dialog, 180, 0);
    lv_obj_set_style_pad_all(s_dlg.dialog, 16, 0);
    lv_obj_set_flex_flow(s_dlg.dialog, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(s_dlg.dialog, 10, 0);
    lv_obj_remove_flag(s_dlg.dialog, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 4. 标题 ---- */
    lv_obj_t *title = lv_label_create(s_dlg.dialog);
    lv_label_set_text(title, has_name ? "控件设置" : "虚拟地址设置");
    lv_obj_set_style_text_color(title, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(title, &lv_font_founder_kaiti_simplified_16, 0);

    /* ---- 5. 名称输入行 (可选) ---- */
    if(has_name) {
        s_dlg.name_ta = create_input_row(s_dlg.dialog, "名称:", 140, 36, "输入名称");
        if(name_max_len > 0)
            lv_textarea_set_max_length(s_dlg.name_ta, name_max_len);
        lv_textarea_set_text(s_dlg.name_ta, cur_name);
    }

    /* ---- 6. 地址输入行 ---- */
    s_dlg.addr_ta = create_input_row(s_dlg.dialog, "地址:", 120, 36, "0~65535");
    lv_textarea_set_accepted_chars(s_dlg.addr_ta, "0123456789");
    lv_textarea_set_max_length(s_dlg.addr_ta, 5);

    char buf[8];
    snprintf(buf, sizeof(buf), "%u", cur_vaddr);
    lv_textarea_set_text(s_dlg.addr_ta, buf);

    /* ---- 7. 错误提示（默认隐藏） ---- */
    s_dlg.err_label = lv_label_create(s_dlg.dialog);
    lv_label_set_text(s_dlg.err_label, "");
    lv_obj_set_style_text_color(s_dlg.err_label, lv_color_hex(0xFF5252), 0);
    lv_obj_set_style_text_font(s_dlg.err_label, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_add_flag(s_dlg.err_label, LV_OBJ_FLAG_HIDDEN);

    /* ---- 8. 按钮行 ---- */
    lv_obj_t *btn_row = lv_obj_create(s_dlg.dialog);
    lv_obj_set_size(btn_row, lv_pct(100), 40);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_END,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_row, 12, 0);
    lv_obj_remove_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *btn_cancel = lv_button_create(btn_row);
    lv_obj_set_size(btn_cancel, 72, 34);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x3a3a5c), 0);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_cancel, 6, 0);
    lv_obj_set_style_border_width(btn_cancel, 0, 0);
    lv_obj_add_event_cb(btn_cancel, btn_cancel_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lc = lv_label_create(btn_cancel);
    lv_label_set_text(lc, "取消");
    lv_obj_set_style_text_color(lc, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(lc, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_center(lc);

    lv_obj_t *btn_ok = lv_button_create(btn_row);
    lv_obj_set_size(btn_ok, 72, 34);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0x00B8D4), 0);
    lv_obj_set_style_bg_opa(btn_ok, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_ok, 6, 0);
    lv_obj_set_style_border_width(btn_ok, 0, 0);
    lv_obj_add_event_cb(btn_ok, btn_confirm_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lo = lv_label_create(btn_ok);
    lv_label_set_text(lo, "确认");
    lv_obj_set_style_text_color(lo, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lo, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_center(lo);
}

void Vaddr_Dialog_Open(lv_obj_t *trigger,
                       uint32_t  cur_vaddr,
                       vaddr_confirm_cb_t on_confirm,
                       void *user_data)
{
    Vaddr_Dialog_Open_Ex(trigger, cur_vaddr, NULL, 0, on_confirm, user_data);
}

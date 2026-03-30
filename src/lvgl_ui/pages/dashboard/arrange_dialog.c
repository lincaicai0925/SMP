#include <stdlib.h>
#include <stdio.h>
#include "pages/dashboard/arrange_dialog.h"
#include "modules/include/styles.h"
#include "modules/include/textarea.h"

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

static lv_obj_t *s_Arrange_Dialog = NULL;
static lv_obj_t *s_Ta_Rows = NULL;
static lv_obj_t *s_Ta_Cols = NULL;
static lv_obj_t *s_Ta_Gap  = NULL;

/* 记住上次参数 */
static char s_Last_Rows[4] = "2";
static char s_Last_Cols[4] = "3";
static char s_Last_Gap[8]  = "10";

static Arrange_Cb_t s_Arrange_Cb = NULL;

static void Arrange_Dialog_Close(void)
{
    if(s_Arrange_Dialog) {
        lv_obj_delete(s_Arrange_Dialog);
        s_Arrange_Dialog = NULL;
        s_Ta_Rows = NULL;
        s_Ta_Cols = NULL;
        s_Ta_Gap  = NULL;
    }
}

static void Arrange_Confirm_Cb(lv_event_t *e)
{
    (void)e;
    /* 保存本次参数 */
    snprintf(s_Last_Rows, sizeof(s_Last_Rows), "%s", lv_textarea_get_text(s_Ta_Rows));
    snprintf(s_Last_Cols, sizeof(s_Last_Cols), "%s", lv_textarea_get_text(s_Ta_Cols));
    snprintf(s_Last_Gap,  sizeof(s_Last_Gap),  "%s", lv_textarea_get_text(s_Ta_Gap));

    int rows = atoi(s_Last_Rows);
    int cols = atoi(s_Last_Cols);
    int gap  = atoi(s_Last_Gap);
    Arrange_Dialog_Close();
    if(s_Arrange_Cb) s_Arrange_Cb(rows, cols, gap);
}

static void Arrange_Cancel_Cb(lv_event_t *e)
{
    (void)e;
    Arrange_Dialog_Close();
}

/* 透明无边框行容器 */
static lv_obj_t* Create_Dialog_Row(lv_obj_t *parent)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 12, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    return row;
}

/* 标签+输入框的组合列 */
static lv_obj_t* Create_Input_Group(lv_obj_t *parent, const char *label_text, const char *init_val)
{
    lv_obj_t *group = lv_obj_create(parent);
    lv_obj_set_height(group, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(group, 1);
    lv_obj_set_flex_flow(group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(group, 0, 0);
    lv_obj_set_style_pad_row(group, 4, 0);
    lv_obj_set_style_bg_opa(group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(group, 0, 0);
    lv_obj_remove_flag(group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(group);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_color(lbl, STYLE_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, 0);

    lv_obj_t *ta = Create_Textarea(group, lv_pct(100), 36, "");
    lv_textarea_set_accepted_chars(ta, "0123456789");
    lv_textarea_set_max_length(ta, 3);
    lv_textarea_set_text(ta, init_val);
    return ta;
}

void Arrange_Dialog_Open(Arrange_Cb_t cb)
{
    if(s_Arrange_Dialog) return;
    s_Arrange_Cb = cb;

    /* 遮罩层 */
    s_Arrange_Dialog = lv_obj_create(lv_layer_top());
    lv_obj_set_size(s_Arrange_Dialog, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_Arrange_Dialog, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_Arrange_Dialog, LV_OPA_50, 0);
    lv_obj_remove_flag(s_Arrange_Dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(s_Arrange_Dialog, 0, 0);
    lv_obj_set_style_radius(s_Arrange_Dialog, 0, 0);

    /* 对话框面板 */
    lv_obj_t *panel = lv_obj_create(s_Arrange_Dialog);
    lv_obj_set_size(panel, 300, LV_SIZE_CONTENT);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, STYLE_BG_PANEL, 0);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_set_style_border_color(panel, STYLE_BORDER, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_pad_all(panel, 20, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel, 12, 0);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "自动排列");
    lv_obj_set_style_text_color(title, STYLE_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, &lv_font_founder_kaiti_simplified_16, 0);

    /* 行数 + 列数 同一行 */
    lv_obj_t *row_rc = Create_Dialog_Row(panel);
    s_Ta_Rows = Create_Input_Group(row_rc, "行数", s_Last_Rows);
    s_Ta_Cols = Create_Input_Group(row_rc, "列数", s_Last_Cols);

    /* 间隔 单独一行 */
    lv_obj_t *gap_group = lv_obj_create(panel);
    lv_obj_set_size(gap_group, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(gap_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(gap_group, 0, 0);
    lv_obj_set_style_pad_row(gap_group, 4, 0);
    lv_obj_set_style_bg_opa(gap_group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(gap_group, 0, 0);
    lv_obj_remove_flag(gap_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_gap = lv_label_create(gap_group);
    lv_label_set_text(lbl_gap, "间隔(像素)");
    lv_obj_set_style_text_color(lbl_gap, STYLE_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(lbl_gap, &lv_font_founder_kaiti_simplified_16, 0);
    s_Ta_Gap = Create_Textarea(gap_group, lv_pct(100), 36, "");
    lv_textarea_set_accepted_chars(s_Ta_Gap, "0123456789");
    lv_textarea_set_max_length(s_Ta_Gap, 3);
    lv_textarea_set_text(s_Ta_Gap, s_Last_Gap);

    /* 布局规则说明 */
    lv_obj_t *lbl_hint = lv_label_create(panel);
    lv_label_set_text(lbl_hint,
        "图表按行列网格排列并自适应尺寸，其余控件保持原始大小，按类型\n"
        "  从左到右、从上至下流式排列");
    lv_obj_set_width(lbl_hint, lv_pct(100));
    lv_obj_set_style_text_color(lbl_hint, lv_color_hex(0x9E9E9E), 0);
    lv_obj_set_style_text_font(lbl_hint, &lv_font_founder_kaiti_simplified_16, 0);

    /* 按钮行 */
    lv_obj_t *btn_row = Create_Dialog_Row(panel);
    lv_obj_set_style_pad_top(btn_row, 4, 0);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_END,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_row, 10, 0);

    lv_obj_t *btn_cancel = lv_button_create(btn_row);
    lv_obj_set_size(btn_cancel, 100, 36);
    lv_obj_set_style_bg_color(btn_cancel, STYLE_BG_HEADER, 0);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_cancel, 8, 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_border_color(btn_cancel, STYLE_BORDER, 0);
    lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, "取消");
    lv_obj_center(lbl_cancel);
    lv_obj_set_style_text_color(lbl_cancel, STYLE_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_cancel, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_add_event_cb(btn_cancel, Arrange_Cancel_Cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_ok = lv_button_create(btn_row);
    lv_obj_set_size(btn_ok, 100, 36);
    lv_obj_set_style_bg_color(btn_ok, STYLE_ACCENT, 0);
    lv_obj_set_style_bg_opa(btn_ok, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_ok, 8, 0);
    lv_obj_t *lbl_ok = lv_label_create(btn_ok);
    lv_label_set_text(lbl_ok, "确定");
    lv_obj_center(lbl_ok);
    lv_obj_set_style_text_color(lbl_ok, STYLE_TEXT_WHITE, 0);
    lv_obj_set_style_text_font(lbl_ok, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_add_event_cb(btn_ok, Arrange_Confirm_Cb, LV_EVENT_CLICKED, NULL);
}

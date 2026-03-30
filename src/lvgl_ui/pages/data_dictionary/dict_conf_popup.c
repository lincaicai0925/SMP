/**
 * @file dict_conf_popup.c
 * @brief 设备配置弹窗 
 */

#include "pages/include/dict_conf_popup.h"
#include "modules/include/dropdown.h"
#include "modules/include/textarea.h"
#include "modules/include/tips_box.h"
#include "cjson/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ═══════════════════════════════════════════════
 * 配色常量
 * ═══════════════════════════════════════════════ */
#define POP_BG          0x1A1D23
#define POP_PANEL       0x22262F
#define POP_CARD        0x2A2F3A
#define POP_INPUT_BG    0x1A1F28
#define POP_BORDER      0x3A3F4B
#define POP_SEPARATOR   0x353B48
#define POP_TEXT_PRI    0xC8CDD8
#define POP_TEXT_SEC    0x8B95A5
#define POP_TEXT_DIM    0x5B6370
#define POP_ACCENT      0x3B82F6
#define POP_ACCENT_DARK 0x2563EB
#define POP_BTN_DISCARD 0x374151
#define POP_BTN_DISC_PR 0x4B5563

#define POP_ALM_NONE    0x6B7280
#define POP_ALM_LOW     0x3B82F6
#define POP_ALM_MID     0xF59E0B
#define POP_ALM_HIGH    0xF97316
#define POP_ALM_CRIT    0xEF4444

/* 布局 */
#define POP_PANEL_W     700
#define POP_PANEL_H     550
#define CARD_RADIUS     14
#define CARD_PAD        16
#define CARD_GAP        14
#define INPUT_H         30
#define FIELD_GAP       10
#define LABEL_W         90
#define ANIM_TIME       180

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_24);
LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

/* 告警级别 */
static const char * const s_alarm_labels[] = { "无", "低级", "中级", "高级", "紧急" };
#define ALARM_LEVEL_CNT  5
static const uint32_t s_alarm_colors[] = {
    POP_ALM_NONE, POP_ALM_LOW, POP_ALM_MID, POP_ALM_HIGH, POP_ALM_CRIT
};

/* data_len */
static const uint32_t s_data_len_opts[] = { 1, 8, 16, 32, 64 };
#define DATA_LEN_CNT  5

/* rw */
static const char * const s_rw_labels[] = { "R", "W", "R/W" };
#define RW_CNT 3

#define DATA_TYPE_OPTIONS "float\nint16\nuint16\nint32\nuint32\nbool\nstring"

/* ═══════════════════════════════════════════════
 * 弹窗状态
 * ═══════════════════════════════════════════════ */
typedef struct {
    lv_obj_t *overlay;
    lv_obj_t *panel;
    smp_ctx_t *ctx;
    data_dict_param_cb arg_cb;

    lv_obj_t *ta_device_name;
    lv_obj_t *ta_desc;
    lv_obj_t *badge_device;
    lv_obj_t *rw_btns[RW_CNT];
    uint8_t   rw_sel;

    lv_obj_t *ta_bus_addr;
    lv_obj_t *ta_modbus_addr;
    lv_obj_t *ta_bit_offset;
    lv_obj_t *badge_addr;

    lv_obj_t *dd_data_type;
    lv_obj_t *ta_default_value;
    lv_obj_t *data_len_btns[DATA_LEN_CNT];
    uint8_t   data_len_sel;
    lv_obj_t *ta_decimal;
    lv_obj_t *ta_subtype;

    lv_obj_t *ta_thmin;
    lv_obj_t *ta_thmax;
    lv_obj_t *alarm_btns[ALARM_LEVEL_CNT];
    uint8_t   alarm_sel;
    lv_obj_t *alarm_badge;
    lv_obj_t *alarm_dot;
    lv_obj_t *bar_preview;
    lv_obj_t *lbl_bar_min;
    lv_obj_t *lbl_bar_max;
    lv_obj_t *lbl_bar_def;

    lv_obj_t *btn_save;
    lv_obj_t *btn_discard;
} popup_state_t;

static popup_state_t s_pop;

/* 前向声明 */
static void Create_Top_Bar(lv_obj_t *parent);
static lv_obj_t *Create_Card(lv_obj_t *parent, const char *title, const char *icon_sym);
static void Create_Card1_Device(lv_obj_t *card);
static void Create_Card2_Address(lv_obj_t *card);
static void Create_Card3_Format(lv_obj_t *card);
static void Create_Card4_Alarm(lv_obj_t *card);
static lv_obj_t *Create_Field_Row(lv_obj_t *parent, const char *label_text);
static lv_obj_t *Create_Badge(lv_obj_t *parent, const char *text, uint32_t bg_color);
static void Create_Separator(lv_obj_t *parent);
static lv_obj_t *Create_Toggle_Btn(lv_obj_t *parent, const char *text, uint32_t w);
static void Save_Cb(lv_event_t *e);
static void Discard_Cb(lv_event_t *e);
static void Close_Popup(void);
static void Reset_Fields(void);
static void RW_Btn_Cb(lv_event_t *e);
static void DataLen_Btn_Cb(lv_event_t *e);
static void Alarm_Btn_Cb(lv_event_t *e);
static void DeviceName_Changed_Cb(lv_event_t *e);
static void BusAddr_Changed_Cb(lv_event_t *e);
static void Threshold_Changed_Cb(lv_event_t *e);
static void Update_RW_Visuals(void);
static void Update_DataLen_Visuals(void);
static void Update_Alarm_Visuals(void);
static void Update_Alarm_Badge(void);
static void Update_Bar_Preview(void);
static void Overlay_Click_Cb(lv_event_t *e);
static int Save_To_JSON(data_dict_item_dsc_t *item);
static void Collect_Fields(data_dict_item_dsc_t *item);

/* 动画辅助：lv_anim_exec_xcb_t 签名是 (void*, int32_t)，
   但 lv_obj_set_style_opa 需要3个参数，所以需要包装函数 */
static void _anim_opa_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, LV_PART_MAIN);
}

static void _anim_scale_cb(void *obj, int32_t v)
{
    lv_obj_set_style_transform_scale((lv_obj_t *)obj, v, LV_PART_MAIN);
}

static void Popup_Auto_Cleanup_Cb(lv_event_t *e) {
    LV_UNUSED(e);
    memset(&s_pop, 0, sizeof(s_pop));
}

/* ═══════════════════════════════════════════════
 * 公共接口
 * ═══════════════════════════════════════════════ */
void Dict_Conf_Popup_Open(smp_ctx_t *ctx, data_dict_param_cb cb)
{
    if(s_pop.overlay) return;
    memset(&s_pop, 0, sizeof(s_pop));
    s_pop.ctx = ctx;
    s_pop.rw_sel = 0;
    s_pop.data_len_sel = 3;
    s_pop.alarm_sel = 0;
    s_pop.arg_cb = cb;

    /* 全屏半透明遮罩 */
    s_pop.overlay = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(s_pop.overlay);
    lv_obj_set_size(s_pop.overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(s_pop.overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pop.overlay, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_flag(s_pop.overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(s_pop.overlay, Overlay_Click_Cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(s_pop.overlay, Popup_Auto_Cleanup_Cb, LV_EVENT_DELETE, NULL);

    /* 主面板 */
    s_pop.panel = lv_obj_create(s_pop.overlay);
    lv_obj_set_size(s_pop.panel, POP_PANEL_W, POP_PANEL_H);
    lv_obj_center(s_pop.panel);
    lv_obj_set_style_bg_color(s_pop.panel, lv_color_hex(POP_PANEL), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pop.panel, LV_OPA_COVER, LV_PART_MAIN);
    // lv_obj_set_style_radius(s_pop.panel, 16, LV_PART_MAIN);
    // lv_obj_set_style_clip_corner(s_pop.panel, true, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(s_pop.panel, 40, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(s_pop.panel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_pop.panel, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_pop.panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_pop.panel, lv_color_hex(POP_BORDER), LV_PART_MAIN);
    lv_obj_set_flex_flow(s_pop.panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(s_pop.panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(s_pop.panel, 0, LV_PART_MAIN);
    lv_obj_remove_flag(s_pop.panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(s_pop.panel, LV_OBJ_FLAG_CLICKABLE);

    /* 打开动画 */
    lv_obj_set_style_opa(s_pop.panel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_transform_scale(s_pop.panel, 230, LV_PART_MAIN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_pop.panel);
    lv_anim_set_time(&a, ANIM_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    lv_anim_set_exec_cb(&a, _anim_opa_cb);
    lv_anim_set_values(&a, LV_OPA_0, LV_OPA_COVER);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, _anim_scale_cb);
    lv_anim_set_values(&a, 230, 256);
    lv_anim_start(&a);

    /* 顶部栏 */
    Create_Top_Bar(s_pop.panel);

    /* 卡片网格容器 */
    lv_obj_t *grid = lv_obj_create(s_pop.panel);
    lv_obj_remove_style_all(grid);
    lv_obj_set_flex_grow(grid, 1);
    lv_obj_set_width(grid, lv_pct(100));
    lv_obj_set_style_pad_all(grid, CARD_GAP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(grid, CARD_GAP, LV_PART_MAIN);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_scroll_dir(grid, LV_DIR_VER);

    lv_obj_t *c1 = Create_Card(grid, "设备标识", LV_SYMBOL_HOME);
    lv_obj_t *c2 = Create_Card(grid, "地址映射", LV_SYMBOL_LIST);
    lv_obj_t *c3 = Create_Card(grid, "数据格式", LV_SYMBOL_EDIT);
    lv_obj_t *c4 = Create_Card(grid, "阈值告警", LV_SYMBOL_WARNING);

    Create_Card1_Device(c1);
    Create_Card2_Address(c2);
    Create_Card3_Format(c3);
    Create_Card4_Alarm(c4);

    Update_RW_Visuals();
    Update_DataLen_Visuals();
    Update_Alarm_Visuals();
    Update_Alarm_Badge();
    Update_Bar_Preview();

    /* overlay 底部关闭提示 */
    lv_obj_t *hint = lv_label_create(s_pop.overlay);
    lv_label_set_text(hint, "点击空白区域关闭");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(hint, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_opa(hint, LV_OPA_50, LV_PART_MAIN);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);
}

/* ═══════════════════════════════════════════════
 * 顶部栏
 * ═══════════════════════════════════════════════ */
static void Create_Top_Bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, lv_pct(100), 60);
    lv_obj_set_style_bg_color(bar, lv_color_hex(POP_PANEL), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_left(bar, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_right(bar, 16, LV_PART_MAIN);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(bar, lv_color_hex(POP_BORDER), LV_PART_MAIN);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    /* 左侧标题 */
    lv_obj_t *title_col = lv_obj_create(bar);
    lv_obj_remove_style_all(title_col);
    lv_obj_set_flex_grow(title_col, 1);
    lv_obj_set_height(title_col, lv_pct(100));
    lv_obj_set_flex_flow(title_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(title_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(title_col, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(title_col);
    lv_label_set_text(title, "设备配置");
    lv_obj_set_style_text_color(title, lv_color_hex(POP_TEXT_PRI), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);

    lv_obj_t *subtitle = lv_label_create(title_col);
    lv_label_set_text(subtitle, "添加新的设备数据条目");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(POP_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(subtitle, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    /* 丢弃按钮 */
    s_pop.btn_discard = lv_btn_create(bar);
    lv_obj_remove_style_all(s_pop.btn_discard);
    lv_obj_set_size(s_pop.btn_discard, 80, 36);
    lv_obj_set_style_bg_color(s_pop.btn_discard, lv_color_hex(POP_BTN_DISCARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pop.btn_discard, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(s_pop.btn_discard, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_pop.btn_discard, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_pop.btn_discard, lv_color_hex(POP_BORDER), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_pop.btn_discard, lv_color_hex(POP_BTN_DISC_PR), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_translate_y(s_pop.btn_discard, 1, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *dl = lv_label_create(s_pop.btn_discard);
    lv_label_set_text(dl, "丢弃");
    lv_obj_set_style_text_color(dl, lv_color_hex(POP_TEXT_SEC), LV_PART_MAIN);
    lv_obj_set_style_text_font(dl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(dl);
    lv_obj_add_event_cb(s_pop.btn_discard, Discard_Cb, LV_EVENT_CLICKED, NULL);

    /* 间隔 */
    lv_obj_t *sp = lv_obj_create(bar);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 8, 1);

    /* 保存按钮 */
    s_pop.btn_save = lv_btn_create(bar);
    lv_obj_remove_style_all(s_pop.btn_save);
    lv_obj_set_size(s_pop.btn_save, 100, 36);
    lv_obj_set_style_bg_color(s_pop.btn_save, lv_color_hex(POP_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pop.btn_save, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(s_pop.btn_save, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(s_pop.btn_save, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(s_pop.btn_save, lv_color_hex(POP_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_pop.btn_save, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_pop.btn_save, lv_color_hex(POP_ACCENT_DARK), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_translate_y(s_pop.btn_save, 1, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(s_pop.btn_save, 4, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *sl = lv_label_create(s_pop.btn_save);
    lv_label_set_text(sl, "保存配置");
    lv_obj_set_style_text_color(sl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(sl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(sl);
    lv_obj_add_event_cb(s_pop.btn_save, Save_Cb, LV_EVENT_CLICKED, NULL);
}

/* ═══════════════════════════════════════════════
 * 卡片创建
 * ═══════════════════════════════════════════════ */
static lv_obj_t *Create_Card(lv_obj_t *parent, const char *title, const char *icon_sym)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, (POP_PANEL_W - 2 - CARD_GAP * 3) / 2, LV_SIZE_CONTENT);
    lv_obj_set_style_min_height(card, 220, LV_PART_MAIN);
    lv_obj_set_style_bg_color(card, lv_color_hex(POP_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card, CARD_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(POP_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_opa(card, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 16, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(card, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(card, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(card, 0, LV_PART_MAIN);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题栏 (child 0) */
    lv_obj_t *header = lv_obj_create(card);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, lv_pct(100), 42);
    lv_obj_set_style_pad_left(header, CARD_PAD, LV_PART_MAIN);
    lv_obj_set_style_pad_right(header, CARD_PAD, LV_PART_MAIN);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *icon = lv_label_create(header);
    lv_label_set_text(icon, icon_sym);
    lv_obj_set_style_text_color(icon, lv_color_hex(POP_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(header);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_color(lbl, lv_color_hex(POP_TEXT_PRI), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_style_pad_left(lbl, 6, LV_PART_MAIN);

    /* 弹性间隔 */
    lv_obj_t *spacer = lv_obj_create(header);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_height(spacer, 1);

    /* 分隔线 (child 1) */
    lv_obj_t *sep = lv_obj_create(card);
    lv_obj_remove_style_all(sep);
    lv_obj_set_size(sep, lv_pct(100), 1);
    lv_obj_set_style_bg_color(sep, lv_color_hex(POP_SEPARATOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, LV_PART_MAIN);

    /* 内容区 (child 2) */
    lv_obj_t *content = lv_obj_create(card);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_style_pad_all(content, CARD_PAD, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(content, FIELD_GAP, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    return card;
}

static lv_obj_t *Card_Get_Header(lv_obj_t *card)  { return lv_obj_get_child(card, 0); }
static lv_obj_t *Card_Get_Content(lv_obj_t *card)  { return lv_obj_get_child(card, 2); }

/* ═══════════════════════════════════════════════
 * 辅助组件
 * ═══════════════════════════════════════════════ */
static lv_obj_t *Create_Field_Row(lv_obj_t *parent, const char *label_text)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), INPUT_H+4);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(row, 8, LV_PART_MAIN);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(POP_TEXT_SEC), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_set_width(lbl, LABEL_W);
    lv_obj_set_style_min_width(lbl, LABEL_W, LV_PART_MAIN);

    return row;
}


static lv_obj_t *Create_Badge(lv_obj_t *parent, const char *text, uint32_t bg_color)
{
    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, LV_SIZE_CONTENT, 22);
    lv_obj_set_style_bg_color(badge, lv_color_hex(bg_color), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_radius(badge, 11, LV_PART_MAIN);
    lv_obj_set_style_pad_left(badge, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(badge, 10, LV_PART_MAIN);
    lv_obj_remove_flag(badge, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(badge);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(bg_color), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(lbl);

    return badge;
}

static void Create_Separator(lv_obj_t *parent)
{
    lv_obj_t *sep = lv_obj_create(parent);
    lv_obj_remove_style_all(sep);
    lv_obj_set_size(sep, lv_pct(100), 1);
    lv_obj_set_style_bg_color(sep, lv_color_hex(POP_SEPARATOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sep, LV_OPA_40, LV_PART_MAIN);
}

static lv_obj_t *Create_Toggle_Btn(lv_obj_t *parent, const char *text, uint32_t w)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, w, 30);
    lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(POP_BORDER), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(POP_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_30, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(POP_TEXT_SEC), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(lbl);

    return btn;
}

static void Set_Toggle_Selected(lv_obj_t *btn, bool selected, uint32_t accent_color)
{
    if(selected) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(accent_color), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(accent_color), LV_PART_MAIN);
        lv_obj_t *lbl = lv_obj_get_child(btn, 0);
        if(lbl) lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(POP_BORDER), LV_PART_MAIN);
        lv_obj_t *lbl = lv_obj_get_child(btn, 0);
        if(lbl) lv_obj_set_style_text_color(lbl, lv_color_hex(POP_TEXT_SEC), LV_PART_MAIN);
    }
}

/* ═══════════════════════════════════════════════
 * 卡片1 - 设备标识
 * ═══════════════════════════════════════════════ */
static void Create_Card1_Device(lv_obj_t *card)
{
    lv_obj_t *header = Card_Get_Header(card);
    lv_obj_t *content = Card_Get_Content(card);

    s_pop.badge_device = Create_Badge(header, "---", POP_ACCENT);

    lv_obj_t *row1 = Create_Field_Row(content, "设备名称");
    s_pop.ta_device_name = Create_Textarea(row1, 0, INPUT_H, "device_name");
    lv_obj_set_flex_grow(s_pop.ta_device_name, 1);
    lv_obj_add_event_cb(s_pop.ta_device_name, DeviceName_Changed_Cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *row2 = Create_Field_Row(content, "描述");
    s_pop.ta_desc = Create_Textarea(row2, 0, INPUT_H, "输入描述信息");
    lv_obj_set_flex_grow(s_pop.ta_desc, 1);

    lv_obj_t *row3 = Create_Field_Row(content, "访问模式");
    lv_obj_t *btn_row = lv_obj_create(row3);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_flex_grow(btn_row, 1);
    lv_obj_set_height(btn_row, 30);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(btn_row, 6, LV_PART_MAIN);
    lv_obj_remove_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < RW_CNT; i++) {
        s_pop.rw_btns[i] = Create_Toggle_Btn(btn_row, s_rw_labels[i], 60);
        lv_obj_add_event_cb(s_pop.rw_btns[i], RW_Btn_Cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }
}

/* ═══════════════════════════════════════════════
 * 卡片2 - 地址映射
 * ═══════════════════════════════════════════════ */
static void Create_Card2_Address(lv_obj_t *card)
{
    lv_obj_t *header = Card_Get_Header(card);
    lv_obj_t *content = Card_Get_Content(card);

    s_pop.badge_addr = Create_Badge(header, "0x0000", POP_ACCENT);

    lv_obj_t *row1 = Create_Field_Row(content, "BUS 地址");
    s_pop.ta_bus_addr = Create_Textarea(row1, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_bus_addr, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_bus_addr, "-0123456789.");
    lv_obj_add_event_cb(s_pop.ta_bus_addr, BusAddr_Changed_Cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *row2 = Create_Field_Row(content, "MODBUS 地址");
    s_pop.ta_modbus_addr = Create_Textarea(row2, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_modbus_addr, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_modbus_addr, "-0123456789.");

    lv_obj_t *row3 = Create_Field_Row(content, "BIT 偏移");
    s_pop.ta_bit_offset = Create_Textarea(row3, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_bit_offset, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_bit_offset, "-0123456789.");
}

/* ═══════════════════════════════════════════════
 * 卡片3 - 数据格式
 * ═══════════════════════════════════════════════ */
static void Create_Card3_Format(lv_obj_t *card)
{
    lv_obj_t *content = Card_Get_Content(card);

    lv_obj_t *row1 = Create_Field_Row(content, "数据类型");
    s_pop.dd_data_type = Create_Dropdown(row1, DATA_TYPE_OPTIONS);
    lv_obj_set_flex_grow(s_pop.dd_data_type, 1);
    lv_obj_set_height(s_pop.dd_data_type, INPUT_H);

    lv_obj_t *row2 = Create_Field_Row(content, "默认值");
    s_pop.ta_default_value = Create_Textarea(row2, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_default_value, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_default_value, "-0123456789.");

    lv_obj_t *row3 = Create_Field_Row(content, "数据长度");
    lv_obj_t *len_row = lv_obj_create(row3);
    lv_obj_remove_style_all(len_row);
    lv_obj_set_flex_grow(len_row, 1);
    lv_obj_set_height(len_row, 30);
    lv_obj_set_flex_flow(len_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(len_row, 4, LV_PART_MAIN);
    lv_obj_set_scroll_dir(len_row, LV_DIR_HOR);

    const char *len_labels[] = {"1", "8", "16", "32", "64"};
    for(int i = 0; i < DATA_LEN_CNT; i++) {
        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%s bit", len_labels[i]);
        s_pop.data_len_btns[i] = Create_Toggle_Btn(len_row, buf, 56);
        lv_obj_add_event_cb(s_pop.data_len_btns[i], DataLen_Btn_Cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }

    lv_obj_t *row4 = Create_Field_Row(content, "小数位");
    s_pop.ta_decimal = Create_Textarea(row4, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_decimal, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_decimal, "-0123456789.");

    lv_obj_t *row5 = Create_Field_Row(content, "子类型");
    s_pop.ta_subtype = Create_Textarea(row5, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_subtype, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_subtype, "-0123456789.");
}

/* ═══════════════════════════════════════════════
 * 卡片4 - 阈值告警
 * ═══════════════════════════════════════════════ */
static void Create_Card4_Alarm(lv_obj_t *card)
{
    lv_obj_t *header = Card_Get_Header(card);
    lv_obj_t *content = Card_Get_Content(card);

    /* 告警徽章容器 */
    s_pop.alarm_badge = lv_obj_create(header);
    lv_obj_remove_style_all(s_pop.alarm_badge);
    lv_obj_set_size(s_pop.alarm_badge, LV_SIZE_CONTENT, 22);
    lv_obj_set_flex_flow(s_pop.alarm_badge, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_pop.alarm_badge, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(s_pop.alarm_badge, 4, LV_PART_MAIN);
    lv_obj_remove_flag(s_pop.alarm_badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(s_pop.alarm_badge, LV_OBJ_FLAG_HIDDEN);

    s_pop.alarm_dot = lv_obj_create(s_pop.alarm_badge);
    lv_obj_remove_style_all(s_pop.alarm_dot);
    lv_obj_set_size(s_pop.alarm_dot, 10, 10);
    lv_obj_set_style_radius(s_pop.alarm_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_pop.alarm_dot, lv_color_hex(POP_ALM_NONE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pop.alarm_dot, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *alarm_lbl = lv_label_create(s_pop.alarm_badge);
    lv_label_set_text(alarm_lbl, "");
    lv_obj_set_style_text_color(alarm_lbl, lv_color_hex(POP_TEXT_SEC), LV_PART_MAIN);
    lv_obj_set_style_text_font(alarm_lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    lv_obj_t *row1 = Create_Field_Row(content, "阈值下限");
    s_pop.ta_thmin = Create_Textarea(row1, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_thmin, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_thmin, "-0123456789.");
    lv_obj_add_event_cb(s_pop.ta_thmin, Threshold_Changed_Cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *row2 = Create_Field_Row(content, "阈值上限");
    s_pop.ta_thmax = Create_Textarea(row2, 0, INPUT_H, "0");
    lv_obj_set_flex_grow(s_pop.ta_thmax, 1);
    lv_textarea_set_accepted_chars(s_pop.ta_thmax, "-0123456789.");
    lv_obj_add_event_cb(s_pop.ta_thmax, Threshold_Changed_Cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *row3 = Create_Field_Row(content, "告警级别");
    lv_obj_set_height(row3, 34);
    lv_obj_t *alm_row = lv_obj_create(row3);
    lv_obj_remove_style_all(alm_row);
    lv_obj_set_flex_grow(alm_row, 1);
    lv_obj_set_height(alm_row, 30);
    lv_obj_set_flex_flow(alm_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(alm_row, 4, LV_PART_MAIN);
    lv_obj_set_scroll_dir(alm_row, LV_DIR_HOR);

    for(int i = 0; i < ALARM_LEVEL_CNT; i++) {
        s_pop.alarm_btns[i] = Create_Toggle_Btn(alm_row, s_alarm_labels[i], 52);
        lv_obj_set_style_radius(s_pop.alarm_btns[i], 15, LV_PART_MAIN);
        lv_obj_add_event_cb(s_pop.alarm_btns[i], Alarm_Btn_Cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }

    Create_Separator(content);

    /* 阈值范围预览条 */
    lv_obj_t *bar_cont = lv_obj_create(content);
    lv_obj_remove_style_all(bar_cont);
    lv_obj_set_size(bar_cont, lv_pct(100), 40);
    lv_obj_set_flex_flow(bar_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(bar_cont, 4, LV_PART_MAIN);
    lv_obj_remove_flag(bar_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_row = lv_obj_create(bar_cont);
    lv_obj_remove_style_all(lbl_row);
    lv_obj_set_size(lbl_row, lv_pct(100), 16);
    lv_obj_set_flex_flow(lbl_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(lbl_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(lbl_row, LV_OBJ_FLAG_SCROLLABLE);

    s_pop.lbl_bar_min = lv_label_create(lbl_row);
    lv_label_set_text(s_pop.lbl_bar_min, "MIN: 0");
    lv_obj_set_style_text_color(s_pop.lbl_bar_min, lv_color_hex(POP_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_pop.lbl_bar_min, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    s_pop.lbl_bar_def = lv_label_create(lbl_row);
    lv_label_set_text(s_pop.lbl_bar_def, "DEF: 0");
    lv_obj_set_style_text_color(s_pop.lbl_bar_def, lv_color_hex(POP_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_pop.lbl_bar_def, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    s_pop.lbl_bar_max = lv_label_create(lbl_row);
    lv_label_set_text(s_pop.lbl_bar_max, "MAX: 0");
    lv_obj_set_style_text_color(s_pop.lbl_bar_max, lv_color_hex(POP_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_pop.lbl_bar_max, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    s_pop.bar_preview = lv_bar_create(bar_cont);
    lv_obj_set_size(s_pop.bar_preview, lv_pct(100), 12);
    lv_bar_set_range(s_pop.bar_preview, 0, 100);
    lv_bar_set_value(s_pop.bar_preview, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_pop.bar_preview, lv_color_hex(POP_INPUT_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pop.bar_preview, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(s_pop.bar_preview, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_pop.bar_preview, lv_color_hex(POP_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_pop.bar_preview, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_pop.bar_preview, 6, LV_PART_INDICATOR);
}

/* ═══════════════════════════════════════════════
 * 回调
 * ═══════════════════════════════════════════════ */
static void RW_Btn_Cb(lv_event_t *e)
{
    s_pop.rw_sel = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    Update_RW_Visuals();
}

static void DataLen_Btn_Cb(lv_event_t *e)
{
    s_pop.data_len_sel = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    Update_DataLen_Visuals();
}

static void Alarm_Btn_Cb(lv_event_t *e)
{
    s_pop.alarm_sel = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    Update_Alarm_Visuals();
    Update_Alarm_Badge();
}

static void DeviceName_Changed_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(!s_pop.badge_device) return;
    const char *txt = lv_textarea_get_text(s_pop.ta_device_name);
    lv_obj_t *lbl = lv_obj_get_child(s_pop.badge_device, 0);
    if(lbl) lv_label_set_text(lbl, (txt && txt[0]) ? txt : "---");
}

static void BusAddr_Changed_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(!s_pop.badge_addr) return;
    const char *txt = lv_textarea_get_text(s_pop.ta_bus_addr);
    uint32_t val = (txt && txt[0]) ? (uint32_t)atoi(txt) : 0;
    char hex_buf[16];
    lv_snprintf(hex_buf, sizeof(hex_buf), "0x%04X", (unsigned)val);
    lv_obj_t *lbl = lv_obj_get_child(s_pop.badge_addr, 0);
    if(lbl) lv_label_set_text(lbl, hex_buf);
}

static void Threshold_Changed_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    Update_Bar_Preview();
}

static void Overlay_Click_Cb(lv_event_t *e)
{
    if(lv_event_get_target(e) == s_pop.overlay)
        Close_Popup();
}

/* 视觉更新 */
static void Update_RW_Visuals(void)
{
    for(int i = 0; i < RW_CNT; i++)
        Set_Toggle_Selected(s_pop.rw_btns[i], i == s_pop.rw_sel, POP_ACCENT);
}

static void Update_DataLen_Visuals(void)
{
    for(int i = 0; i < DATA_LEN_CNT; i++)
        Set_Toggle_Selected(s_pop.data_len_btns[i], i == s_pop.data_len_sel, POP_ACCENT);
}

static void Update_Alarm_Visuals(void)
{
    for(int i = 0; i < ALARM_LEVEL_CNT; i++)
        Set_Toggle_Selected(s_pop.alarm_btns[i], i == s_pop.alarm_sel, s_alarm_colors[i]);
}

static void Update_Alarm_Badge(void)
{
    if(s_pop.alarm_sel == 0) {
        lv_obj_add_flag(s_pop.alarm_badge, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(s_pop.alarm_badge, LV_OBJ_FLAG_HIDDEN);
        uint32_t clr = s_alarm_colors[s_pop.alarm_sel];
        lv_obj_set_style_bg_color(s_pop.alarm_dot, lv_color_hex(clr), LV_PART_MAIN);
        lv_obj_t *lbl = lv_obj_get_child(s_pop.alarm_badge, 1);
        if(lbl) lv_label_set_text(lbl, s_alarm_labels[s_pop.alarm_sel]);
    }
}

static void Update_Bar_Preview(void)
{
    if(!s_pop.bar_preview) return;

    const char *min_txt = lv_textarea_get_text(s_pop.ta_thmin);
    const char *max_txt = lv_textarea_get_text(s_pop.ta_thmax);
    const char *def_txt = lv_textarea_get_text(s_pop.ta_default_value);

    double thmin = (min_txt && min_txt[0]) ? strtod(min_txt, NULL) : 0;
    double thmax = (max_txt && max_txt[0]) ? strtod(max_txt, NULL) : 0;
    double defv  = (def_txt && def_txt[0]) ? strtod(def_txt, NULL) : 0;

    char buf[32];
    lv_snprintf(buf, sizeof(buf), "MIN: %d", (int)thmin);
    lv_label_set_text(s_pop.lbl_bar_min, buf);
    lv_snprintf(buf, sizeof(buf), "DEF: %d", (int)defv);
    lv_label_set_text(s_pop.lbl_bar_def, buf);
    lv_snprintf(buf, sizeof(buf), "MAX: %d", (int)thmax);
    lv_label_set_text(s_pop.lbl_bar_max, buf);

    if(thmax > thmin) {
        int32_t pct = (int32_t)((thmax - thmin) / thmax * 100);
        if(pct < 0) pct = 0;
        if(pct > 100) pct = 100;
        lv_bar_set_value(s_pop.bar_preview, pct, LV_ANIM_ON);
        uint32_t bar_clr = POP_ACCENT;
        if(s_pop.alarm_sel > 0) bar_clr = s_alarm_colors[s_pop.alarm_sel];
        lv_obj_set_style_bg_color(s_pop.bar_preview, lv_color_hex(bar_clr), LV_PART_INDICATOR);
    } else {
        lv_bar_set_value(s_pop.bar_preview, 0, LV_ANIM_ON);
    }
}

/* ═══════════════════════════════════════════════
 * 字段收集与保存
 * ═══════════════════════════════════════════════ */
static inline void _pop_safe_strcpy(char *dst, size_t dst_size, const char *src)
{
    if(!src) { dst[0] = '\0'; return; }
    size_t len = strlen(src);
    if(len >= dst_size) len = dst_size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void Collect_Fields(data_dict_item_dsc_t *item)
{
    memset(item, 0, sizeof(*item));
    const char *txt;

    txt = lv_textarea_get_text(s_pop.ta_device_name);
    _pop_safe_strcpy(item->device_name, DEVICE_NAME_MAX_LEN, txt);

    txt = lv_textarea_get_text(s_pop.ta_desc);
    _pop_safe_strcpy(item->desc, DESC_MAX_LEN, txt);

    _pop_safe_strcpy(item->rw, RW_MAX_LEN, s_rw_labels[s_pop.rw_sel]);

    txt = lv_textarea_get_text(s_pop.ta_bus_addr);
    item->bus_addr = (txt && txt[0]) ? (uint32_t)atoi(txt) : 0;

    txt = lv_textarea_get_text(s_pop.ta_modbus_addr);
    _pop_safe_strcpy(item->modbus_addr, MODBUS_ADDR_MAX_LEN, txt);

    txt = lv_textarea_get_text(s_pop.ta_bit_offset);
    item->bit_offset = (txt && txt[0]) ? (uint32_t)atoi(txt) : 0;

    char dt_buf[DATA_TYPW_MAX_LEN];
    lv_dropdown_get_selected_str(s_pop.dd_data_type, dt_buf, sizeof(dt_buf));
    _pop_safe_strcpy(item->data_type, DATA_TYPW_MAX_LEN, dt_buf);

    txt = lv_textarea_get_text(s_pop.ta_default_value);
    item->default_value = (txt && txt[0]) ? strtod(txt, NULL) : 0;

    item->data_len = s_data_len_opts[s_pop.data_len_sel];

    txt = lv_textarea_get_text(s_pop.ta_decimal);
    item->decimal = (txt && txt[0]) ? (uint32_t)atoi(txt) : 0;

    txt = lv_textarea_get_text(s_pop.ta_subtype);
    item->subtype = (txt && txt[0]) ? (uint32_t)atoi(txt) : 0;

    txt = lv_textarea_get_text(s_pop.ta_thmin);
    item->thmin = (txt && txt[0]) ? strtod(txt, NULL) : 0;

    txt = lv_textarea_get_text(s_pop.ta_thmax);
    item->thmax = (txt && txt[0]) ? strtod(txt, NULL) : 0;

    item->alarm_level = s_pop.alarm_sel;
}

static int Save_To_JSON(data_dict_item_dsc_t *item)
{
   
    return 0;
}

static void Save_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    data_dict_item_dsc_t item;
    Collect_Fields(&item);
    int8_t rtn = 0;
    if(s_pop.arg_cb != NULL){
        rtn = s_pop.arg_cb(&item);
    }

    // /* 成功反馈 */
    // lv_obj_set_style_bg_color(s_pop.btn_save, lv_color_hex(0x22C55E), LV_PART_MAIN);
    // lv_obj_t *sl = lv_obj_get_child(s_pop.btn_save, 0);
    // if(sl) lv_label_set_text(sl, "已保存");

    // Create_Tips_Box("配置已保存", 0x22C55E);
    if(rtn == 0)
        Close_Popup();
}

static void Discard_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    Reset_Fields();
}

static void Reset_Fields(void)
{
    lv_textarea_set_text(s_pop.ta_device_name, "");
    lv_textarea_set_text(s_pop.ta_desc, "");
    s_pop.rw_sel = 0;
    Update_RW_Visuals();

    lv_textarea_set_text(s_pop.ta_bus_addr, "");
    lv_textarea_set_text(s_pop.ta_modbus_addr, "");
    lv_textarea_set_text(s_pop.ta_bit_offset, "");

    lv_dropdown_set_selected(s_pop.dd_data_type, 0);
    lv_textarea_set_text(s_pop.ta_default_value, "");
    s_pop.data_len_sel = 3;
    Update_DataLen_Visuals();
    lv_textarea_set_text(s_pop.ta_decimal, "");
    lv_textarea_set_text(s_pop.ta_subtype, "");

    lv_textarea_set_text(s_pop.ta_thmin, "");
    lv_textarea_set_text(s_pop.ta_thmax, "");
    s_pop.alarm_sel = 0;
    Update_Alarm_Visuals();
    Update_Alarm_Badge();
    Update_Bar_Preview();

    lv_obj_t *lbl = lv_obj_get_child(s_pop.badge_device, 0);
    if(lbl) lv_label_set_text(lbl, "---");
    lbl = lv_obj_get_child(s_pop.badge_addr, 0);
    if(lbl) lv_label_set_text(lbl, "0x0000");
}

static void Close_Popup(void)
{
    if(!s_pop.overlay) return;
    lv_obj_delete(s_pop.overlay);
    memset(&s_pop, 0, sizeof(s_pop));
}

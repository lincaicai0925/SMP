#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "chart_setting_pop.h"
#include "smp_private.h"
#include "modules/include/textarea.h"
#include "modules/include/checkbox.h"
#include "modules/include/styles.h"

extern smp_ctx_t * g_smp_ctx;

/* 默认颜色表 */
static const uint32_t s_default_colors[CHART_SERIES_NUM] = {
    0x00E5FF, 0xFFEB3B, 0x76FF03, 0xFF4081
};


typedef struct {
    lv_obj_t            *overlay;                       /* 半透明遮罩 */
    lv_obj_t            *dialog;                        /* 弹窗卡片 */
    lv_obj_t            *name_ta[CHART_SERIES_NUM];     /* 名称输入 */
    lv_obj_t            *addr_ta[CHART_SERIES_NUM];     /* 地址输入 */
    lv_obj_t            *eye_btn[CHART_SERIES_NUM];     /* 显隐按钮 */
    lv_obj_t            *enable_cb[CHART_SERIES_NUM];   /* 启用复选框 */
    lv_obj_t            *err_label[CHART_SERIES_NUM];   /* 每行的错误提示 */
    lv_obj_t            *err_input;                     /* 当前标红的输入框 */
    chart_series_conf_t *conf;                          /* 外部数据指针 */
    lv_obj_t            *trigger;                       /* 触发控件 */
    bool                 open;

    series_dialog_cb_t   on_confirm;                    /* 确认回调 */
    void                *cb_user_data;                  /* 回调透传参数 */
} series_dialog_t;

static series_dialog_t s_dlg = {0};

/* ---------- 前置声明 ---------- */
static void series_dialog_close(void);
static void overlay_click_cb(lv_event_t *e);
static void btn_confirm_cb(lv_event_t *e);
static void btn_cancel_cb(lv_event_t *e);
static void eye_btn_cb(lv_event_t *e);
static void enable_cb(lv_event_t *e);
static void show_error(int row, lv_obj_t *input, const char *msg);
static void clear_error(void);
static void set_row_disabled(int idx, bool disabled);

/* ========== 颜色常量 (集中管理，方便统一调整) ========== */
#define CLR_DLG_BG          0x1a1a2e   /* 弹窗背景 */
#define CLR_DLG_BORDER      0x2a2a44   /* 弹窗/分割线边框 */
#define CLR_CARD_BG         0x22223a   /* 卡片背景 */
#define CLR_INPUT_BG        0x26263e   /* 输入框背景 */
#define CLR_INPUT_BORDER    0x333350   /* 输入框默认边框 */
#define CLR_FOCUS_BORDER    0x00E5FF   /* 输入框焦点边框 */
#define CLR_TEXT_PRIMARY     0xeeeeee   /* 主文字 */
#define CLR_TEXT_SECONDARY   0x8888aa   /* 次级文字 */
#define CLR_TEXT_HINT        0x555577   /* 占位符/提示文字 */
#define CLR_ERR_TEXT         0xFF5252   /* 错误文字 */
#define CLR_ERR_BG           0x35202a   /* 错误输入框背景 */
#define CLR_BTN_CANCEL_BG    0x2a2a44   /* 取消按钮背景 */
#define CLR_BTN_CANCEL_PR    0x3a3a5c   /* 取消按钮按下 */
#define CLR_BTN_OK_BG        0x00B8D4   /* 确认按钮背景 */
#define CLR_BTN_OK_PR        0x009EAD   /* 确认按钮按下 */
#define CLR_EYE_BG           0x2a2a44   /* 眼睛按钮背景 */

static void apply_ta_style(lv_obj_t *ta)
{
    if(!ta) return;

    lv_obj_set_style_bg_color(ta, lv_color_hex(CLR_INPUT_BG), 0);
    lv_obj_set_style_bg_opa(ta, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(ta, lv_color_hex(0xffffff), 0);

    lv_obj_set_style_border_color(ta, lv_color_hex(CLR_INPUT_BORDER), 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_radius(ta, 6, 0);

    lv_obj_set_style_pad_top(ta, 6, 0);
    lv_obj_set_style_pad_bottom(ta, 6, 0);
    lv_obj_set_style_pad_left(ta, 10, 0);
    lv_obj_set_style_pad_right(ta, 10, 0);

    lv_obj_set_style_text_color(ta, lv_color_hex(CLR_TEXT_HINT),
                                LV_PART_TEXTAREA_PLACEHOLDER);

    lv_obj_set_style_border_color(ta, lv_color_hex(CLR_FOCUS_BORDER),
                                  LV_STATE_FOCUSED);

    lv_obj_set_style_width(ta, 2, LV_PART_CURSOR);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0xffffff), LV_PART_CURSOR);
    lv_obj_set_style_border_width(ta, 0, LV_PART_CURSOR);
}

/* 将指定输入框标记为错误状态：红色边框 + 微红背景 */
static void mark_input_error(lv_obj_t *ta)
{
    if(!ta) return;
    lv_obj_set_style_border_color(ta, lv_color_hex(CLR_ERR_TEXT), 0);
    lv_obj_set_style_border_width(ta, 2, 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(CLR_ERR_BG), 0);
    s_dlg.err_input = ta;
}

/* 恢复输入框的默认样式 */
static void reset_input_style(lv_obj_t *ta)
{
    if(!ta) return;
    lv_obj_set_style_border_color(ta, lv_color_hex(CLR_INPUT_BORDER), 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(CLR_INPUT_BG), 0);
}

/* ---------- 眼睛按钮：切换系列显隐（visible） ---------- */
static void eye_btn_cb(lv_event_t *e)
{
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if(idx < 0 || idx >= CHART_SERIES_NUM) return;
    if(!s_dlg.conf) return;

    /* 切换 visible 状态 */
    s_dlg.conf[idx].visible = !s_dlg.conf[idx].visible;

    /* 更新图标 */
    lv_obj_t *btn = lv_event_get_target(e);
    if(!btn) return;

    if(s_dlg.conf[idx].visible) {
        lv_obj_set_style_bg_image_src(btn, g_smp_ctx->imgs[IMG_EYES_OPEN], LV_PART_MAIN);
        lv_obj_set_style_bg_image_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_image_src(btn, g_smp_ctx->imgs[IMG_EYES_OFF], LV_PART_MAIN);
        lv_obj_set_style_bg_image_opa(btn, LV_OPA_40, LV_PART_MAIN);
    }
}

/* ---------- 复选框：切换系列启用/禁用 ---------- */
static void enable_cb(lv_event_t *e)
{
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if(idx < 0 || idx >= CHART_SERIES_NUM) return;
    if(!s_dlg.conf) return;

    bool checked = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    s_dlg.conf[idx].enabled = checked;

    set_row_disabled(idx, !checked);
}

/* ---------- 设置行内控件的禁用/启用态 ---------- */
static void set_row_disabled(int idx, bool disabled)
{
    lv_obj_t *widgets[] = {
        s_dlg.name_ta[idx],
        s_dlg.addr_ta[idx],
        s_dlg.eye_btn[idx]
    };
    for(int j = 0; j < 3; j++) {
        if(!widgets[j]) continue;
        if(disabled) {
            lv_obj_add_state(widgets[j], LV_STATE_DISABLED);
            lv_obj_remove_flag(widgets[j], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(widgets[j], LV_OPA_40, 0);
        } else {
            lv_obj_remove_state(widgets[j], LV_STATE_DISABLED);
            lv_obj_add_flag(widgets[j], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(widgets[j], LV_OPA_COVER, 0);
        }
    }
}

/*------------------------------------------------------------
 * 验证输入
 *   - 名称不能为空
 *   - 地址可以为空（表示不绑定），填了则必须合法
 *   - 已填地址之间不能重复
 *------------------------------------------------------------*/
static bool validate_inputs(void)
{
    clear_error();

    for(int i = 0; i < CHART_SERIES_NUM; i++) {
        if(!s_dlg.name_ta[i] || !s_dlg.addr_ta[i]) continue;

        /* 未启用的系列跳过验证 */
        if(!s_dlg.conf[i].enabled) continue;

        /* 名称不能为空 */
        const char *name = lv_textarea_get_text(s_dlg.name_ta[i]);
        if(!name || name[0] == '\0') {
            show_error(i, s_dlg.name_ta[i], "名称不能为空");
            return false;
        }
        if(strlen(name) >= CHART_SERIES_NAME_MAX_LEN) {
            show_error(i, s_dlg.name_ta[i], "名称过长");
            return false;
        }

        /* 启用的系列：地址必须填写且为 0~0xFFFFFFFE */
        const char *addr_str = lv_textarea_get_text(s_dlg.addr_ta[i]);
        if(!addr_str || addr_str[0] == '\0') {
            show_error(i, s_dlg.addr_ta[i], "启用的系列必须填写地址");
            return false;
        }

        char *endptr = NULL;
        unsigned long val = strtoul(addr_str, &endptr, 10);
        if(endptr == NULL || *endptr != '\0' || addr_str[0] == '-' || val > 0xFFFFFFFEUL) {
            show_error(i, s_dlg.addr_ta[i], "地址无效 (0~0xFFFFFFFE)");
            return false;
        }

        /* 启用系列之间地址不能重复 */
        for(int j = 0; j < i; j++) {
            if(!s_dlg.addr_ta[j] || !s_dlg.conf[j].enabled) continue;
            const char *other = lv_textarea_get_text(s_dlg.addr_ta[j]);
            if(other && other[0] != '\0'
               && strtol(other, NULL, 10) == val) {
                char buf[64];
                snprintf(buf, sizeof(buf),
                         "与系列 %d 地址重复", j + 1);
                show_error(i, s_dlg.addr_ta[i], buf);
                return false;
            }
        }
    }
    return true;
}

/*------------------------------------------------------------
 * 确认：验证 → 回写 → 关闭
 *------------------------------------------------------------*/
static void btn_confirm_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(!validate_inputs()) return;
    if(!s_dlg.conf) return;

    for(int i = 0; i < CHART_SERIES_NUM; i++) {
        if(!s_dlg.name_ta[i] || !s_dlg.addr_ta[i]) continue;

        const char *name = lv_textarea_get_text(s_dlg.name_ta[i]);
        if(name) {
            strncpy(s_dlg.conf[i].name, name, CHART_SERIES_NAME_MAX_LEN - 1);
            s_dlg.conf[i].name[CHART_SERIES_NAME_MAX_LEN - 1] = '\0';
        }

        if(!s_dlg.conf[i].enabled) {
            s_dlg.conf[i].vaddr = 0xFFFFFFFF;  /* 未启用：标记为无效地址 */
            continue;
        }

        const char *addr_str = lv_textarea_get_text(s_dlg.addr_ta[i]);
        if(addr_str && addr_str[0] != '\0') {
            s_dlg.conf[i].vaddr = (uint32_t)strtol(addr_str, NULL, 10);
        } else {
            s_dlg.conf[i].vaddr = 0xFFFFFFFF;
        }
    }

    if(s_dlg.on_confirm) {
        s_dlg.on_confirm(s_dlg.conf, s_dlg.cb_user_data);
    }
    series_dialog_close();

}

/*------------------------------------------------------------
 * 取消 / 点击遮罩
 *------------------------------------------------------------*/
static void btn_cancel_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    series_dialog_close();
}

static void overlay_click_cb(lv_event_t *e)
{
    /* 只响应直接点击遮罩本身，不响应子对象冒泡 */
    if(lv_event_get_target(e) == s_dlg.overlay) {
        series_dialog_close();
    }
}

/*------------------------------------------------------------
 * 错误提示 —— 卡片内模式
 *   row   : 出错的系列索引 (0-based)
 *   input : 出错的 textarea，会被标红
 *   msg   : 错误文本（卡片已标识系列号，无需再拼前缀）
 *------------------------------------------------------------*/
static void show_error(int row, lv_obj_t *input, const char *msg)
{
    if(row < 0 || row >= CHART_SERIES_NUM) return;
    if(!msg) return;

    /* 显示该行的错误标签 */
    if(s_dlg.err_label[row]) {
        lv_label_set_text(s_dlg.err_label[row], msg);
        lv_obj_remove_flag(s_dlg.err_label[row], LV_OBJ_FLAG_HIDDEN);
    }

    /* 标红出错的输入框 */
    mark_input_error(input);
}

static void clear_error(void)
{
    /* 隐藏所有行的错误标签 */
    for(int i = 0; i < CHART_SERIES_NUM; i++) {
        if(s_dlg.err_label[i]) {
            lv_obj_add_flag(s_dlg.err_label[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 恢复上一次标红的输入框 */
    if(s_dlg.err_input) {
        reset_input_style(s_dlg.err_input);
        s_dlg.err_input = NULL;
    }
}

/*------------------------------------------------------------
 * 关闭弹窗：统一清理
 *------------------------------------------------------------*/
static void series_dialog_close(void)
{
    if(!s_dlg.open) return;

    if(s_dlg.overlay) {
        lv_obj_delete(s_dlg.overlay);   /* 会连带删除 dialog 及所有子对象 */
        s_dlg.overlay = NULL;
    }
    s_dlg.dialog    = NULL;
    s_dlg.err_input = NULL;
    memset(s_dlg.name_ta,   0, sizeof(s_dlg.name_ta));
    memset(s_dlg.addr_ta,   0, sizeof(s_dlg.addr_ta));
    memset(s_dlg.eye_btn,   0, sizeof(s_dlg.eye_btn));
    memset(s_dlg.enable_cb, 0, sizeof(s_dlg.enable_cb));
    memset(s_dlg.err_label,  0, sizeof(s_dlg.err_label));
    s_dlg.open = false;
}

/*============================================================
 * 创建弹窗  (外部调用入口)
 *
 * @param trigger   触发控件，弹窗定位在其右侧
 * @param conf      系列配置数组，长度 CHART_SERIES_NUM
 *============================================================*/
void Series_Dialog_Open(lv_obj_t *trigger,
                        chart_series_conf_t *conf,
                        series_dialog_cb_t on_confirm,
                        void *user_data)
{
    if(!trigger || !conf) return;
    if(s_dlg.open) return;              /* 防止重复打开 */

    s_dlg.conf    = conf;
    s_dlg.trigger = trigger;
    s_dlg.open    = true;
    s_dlg.on_confirm   = on_confirm;
    s_dlg.cb_user_data = user_data;

    /* ---- 1. 半透明遮罩（全屏） ---- */
    s_dlg.overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(s_dlg.overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(s_dlg.overlay, 0, 0);
    lv_obj_set_style_bg_color(s_dlg.overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_dlg.overlay, 120, 0);
    lv_obj_set_style_border_width(s_dlg.overlay, 0, 0);
    lv_obj_set_style_radius(s_dlg.overlay, 0, 0);
    lv_obj_remove_flag(s_dlg.overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(s_dlg.overlay, overlay_click_cb,
                        LV_EVENT_CLICKED, NULL);

    /* 遮罩提示：点击空白区域关闭 */
    LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
    lv_obj_t *tip = lv_label_create(s_dlg.overlay);
    lv_label_set_text(tip, "点击空白区域关闭");
    lv_obj_set_style_text_color(tip, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(tip, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_align(tip, LV_ALIGN_BOTTOM_MID, 0, -12);

    /* ---- 2. 弹窗卡片定位 ---- */
    lv_area_t trig_area;
    lv_obj_get_coords(trigger, &trig_area);

    int32_t dlg_w = 400;
    int32_t dlg_h = CHART_SERIES_NUM * 75 + 130;
    int32_t dlg_x = trig_area.x2 + 8;
    int32_t dlg_y = trig_area.y1;

    /* 边界修正：右侧放不下就放左侧 */
    if(dlg_x + dlg_w > LV_HOR_RES - 8) {
        dlg_x = trig_area.x1 - dlg_w - 8;
        if(dlg_x < 8) dlg_x = (LV_HOR_RES - dlg_w) / 2;  /* 居中兜底 */
    }
    /* 下方溢出修正 */
    if(dlg_y + dlg_h > LV_VER_RES - 8) {
        dlg_y = LV_VER_RES - dlg_h - 8;
        if(dlg_y < 8) dlg_y = 8;
    }

    /* ---- 3. 弹窗卡片 ---- */
    s_dlg.dialog = lv_obj_create(s_dlg.overlay);
    lv_obj_set_size(s_dlg.dialog, dlg_w, LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(s_dlg.dialog, dlg_h, 0);
    lv_obj_set_pos(s_dlg.dialog, dlg_x, dlg_y);
    lv_obj_set_style_bg_color(s_dlg.dialog, lv_color_hex(CLR_DLG_BG), 0);
    lv_obj_set_style_bg_opa(s_dlg.dialog, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_dlg.dialog, lv_color_hex(CLR_DLG_BORDER), 0);
    lv_obj_set_style_border_width(s_dlg.dialog, 1, 0);
    lv_obj_set_style_radius(s_dlg.dialog, 14, 0);
    lv_obj_set_style_shadow_width(s_dlg.dialog, 32, 0);
    lv_obj_set_style_shadow_color(s_dlg.dialog, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(s_dlg.dialog, 180, 0);
    lv_obj_set_style_pad_top(s_dlg.dialog, 0, 0);
    lv_obj_set_style_pad_bottom(s_dlg.dialog, 16, 0);
    lv_obj_set_style_pad_left(s_dlg.dialog, 0, 0);
    lv_obj_set_style_pad_right(s_dlg.dialog, 0, 0);
    lv_obj_set_flex_flow(s_dlg.dialog, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(s_dlg.dialog, 0, 0);
    lv_obj_remove_flag(s_dlg.dialog, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 4. 标题栏（带底部分割线） ---- */
    lv_obj_t *title_row = lv_obj_create(s_dlg.dialog);
    lv_obj_set_size(title_row, lv_pct(100), 48);
    lv_obj_set_style_bg_opa(title_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_side(title_row, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(title_row, lv_color_hex(CLR_DLG_BORDER), 0);
    lv_obj_set_style_border_width(title_row, 1, 0);
    lv_obj_set_style_pad_left(title_row, 20, 0);
    lv_obj_set_style_pad_right(title_row, 20, 0);
    lv_obj_set_style_pad_top(title_row, 0, 0);
    lv_obj_set_style_pad_bottom(title_row, 0, 0);
    lv_obj_remove_flag(title_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(title_row);
    lv_label_set_text(title_label, "系列设置");
    lv_obj_set_style_text_color(title_label, lv_color_hex(CLR_TEXT_PRIMARY), 0);
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);

    /* ---- 5. 系列卡片容器（含左右内边距） ---- */
    lv_obj_t *cards_cont = lv_obj_create(s_dlg.dialog);
    lv_obj_set_size(cards_cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(cards_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cards_cont, 0, 0);
    lv_obj_set_style_pad_top(cards_cont, 12, 0);
    lv_obj_set_style_pad_bottom(cards_cont, 4, 0);
    lv_obj_set_style_pad_left(cards_cont, 16, 0);
    lv_obj_set_style_pad_right(cards_cont, 16, 0);
    lv_obj_set_style_pad_row(cards_cont, 8, 0);
    lv_obj_set_flex_flow(cards_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_flag(cards_cont, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 6. 每个系列一张卡片 ---- */
    for(int i = 0; i < CHART_SERIES_NUM; i++) {

        /* --- 卡片外壳：左侧色条标识系列颜色 --- */
        lv_obj_t *card = lv_obj_create(cards_cont);
        lv_obj_set_size(card, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(card, STYLE_BG_PANEL, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(card, 10, 0);
        lv_obj_set_style_border_side(card, LV_BORDER_SIDE_LEFT, 0);
        lv_obj_set_style_border_color(card, conf[i].color, 0);
        lv_obj_set_style_border_width(card, 10, 0);
        lv_obj_set_style_pad_top(card, 12, 0);
        lv_obj_set_style_pad_bottom(card, 12, 0);
        lv_obj_set_style_pad_left(card, 14, 0);
        lv_obj_set_style_pad_right(card, 14, 0);
        lv_obj_set_style_pad_row(card, 6, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        /* --- 内容行：复选框 + 名称 + 地址 + 眼睛 --- */
        lv_obj_t *content_row = lv_obj_create(card);
        lv_obj_set_size(content_row, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(content_row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(content_row, 0, 0);
        lv_obj_set_style_pad_all(content_row, 0, 0);
        lv_obj_set_flex_flow(content_row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(content_row, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(content_row, 8, 0);
        lv_obj_remove_flag(content_row, LV_OBJ_FLAG_SCROLLABLE);

        /* 启用复选框 */
        s_dlg.enable_cb[i] = Create_Checkbox(content_row, "");
        if(conf[i].enabled)
            lv_obj_add_state(s_dlg.enable_cb[i], LV_STATE_CHECKED);
        lv_obj_add_event_cb(s_dlg.enable_cb[i], enable_cb,
                            LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)i);

        /* 名称输入框 */
        s_dlg.name_ta[i] = Create_Textarea(content_row, 0, 32, "名称");
        lv_obj_set_flex_grow(s_dlg.name_ta[i], 14);
        lv_textarea_set_max_length(s_dlg.name_ta[i],
                                   CHART_SERIES_NAME_MAX_LEN - 1);
        if(conf[i].name[0] != '\0')
            lv_textarea_set_text(s_dlg.name_ta[i], conf[i].name);

        /* 地址输入框 */
        s_dlg.addr_ta[i] = Create_Textarea(content_row, 0, 32, "地址");
        lv_obj_set_flex_grow(s_dlg.addr_ta[i], 10);
        lv_textarea_set_accepted_chars(s_dlg.addr_ta[i], "0123456789");
        lv_textarea_set_max_length(s_dlg.addr_ta[i], 10);   /* 0~65535 */

        if(conf[i].vaddr > 0 && conf[i].vaddr != 0xFFFFFFFF) {
            char addr_buf[12];
            snprintf(addr_buf, sizeof(addr_buf), "%u", conf[i].vaddr);
            lv_textarea_set_text(s_dlg.addr_ta[i], addr_buf);
        }

        /* 显隐切换按钮（眼睛图标） */
        s_dlg.eye_btn[i] = lv_obj_create(content_row);
        lv_obj_set_size(s_dlg.eye_btn[i], 30, 30);
        lv_obj_set_style_bg_color(s_dlg.eye_btn[i],
                                  lv_color_hex(CLR_EYE_BG), 0);
        lv_obj_set_style_bg_opa(s_dlg.eye_btn[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(s_dlg.eye_btn[i], 0, 0);
        lv_obj_set_style_radius(s_dlg.eye_btn[i], 8, 0);
        lv_obj_set_style_pad_all(s_dlg.eye_btn[i], 0, 0);
        lv_obj_remove_flag(s_dlg.eye_btn[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(s_dlg.eye_btn[i], LV_OBJ_FLAG_CLICKABLE);

        /* 根据当前 visible 状态设置图标和透明度 */
        if(conf[i].visible) {
            lv_obj_set_style_bg_image_src(s_dlg.eye_btn[i],
                g_smp_ctx->imgs[IMG_EYES_OPEN], LV_PART_MAIN);
            lv_obj_set_style_bg_image_opa(s_dlg.eye_btn[i],
                LV_OPA_COVER, LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_image_src(s_dlg.eye_btn[i],
                g_smp_ctx->imgs[IMG_EYES_OFF], LV_PART_MAIN);
            lv_obj_set_style_bg_image_opa(s_dlg.eye_btn[i],
                LV_OPA_40, LV_PART_MAIN);
        }

        lv_obj_add_event_cb(s_dlg.eye_btn[i], eye_btn_cb,
                            LV_EVENT_CLICKED, (void*)(intptr_t)i);

        /* --- 行内错误提示标签（默认隐藏） --- */
        s_dlg.err_label[i] = lv_label_create(card);
        lv_label_set_text(s_dlg.err_label[i], "");
        lv_obj_set_width(s_dlg.err_label[i], lv_pct(100));
        lv_obj_set_style_text_color(s_dlg.err_label[i],
                                    lv_color_hex(CLR_ERR_TEXT), 0);
        lv_obj_set_style_pad_left(s_dlg.err_label[i], 30, 0);
        lv_obj_add_flag(s_dlg.err_label[i], LV_OBJ_FLAG_HIDDEN);

        /* 未启用的系列：初始化时禁用同行控件 */
        if(!conf[i].enabled) {
            set_row_disabled(i, true);
        }
    }

    /* ---- 7. 底部按钮栏 ---- */
    lv_obj_t *btn_row = lv_obj_create(s_dlg.dialog);
    lv_obj_set_size(btn_row, lv_pct(100), 44);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_top(btn_row, 4, 0);
    lv_obj_set_style_pad_bottom(btn_row, 0, 0);
    lv_obj_set_style_pad_left(btn_row, 16, 0);
    lv_obj_set_style_pad_right(btn_row, 16, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_row, 8, 0);
    lv_obj_remove_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    /* 左侧提示文字 */
    lv_obj_t *hint = lv_label_create(btn_row);
    lv_label_set_text(hint, "名称 / 地址");
    lv_obj_set_style_text_color(hint, lv_color_hex(CLR_TEXT_HINT), 0);
    lv_obj_set_flex_grow(hint, 1);          /* 撑开剩余空间，将按钮推到右侧 */

    /* 取消按钮 */
    lv_obj_t *btn_cancel = lv_button_create(btn_row);
    lv_obj_set_size(btn_cancel, 72, 34);
    lv_obj_set_style_bg_color(btn_cancel,
                lv_color_hex(CLR_BTN_CANCEL_BG), 0);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_cancel, 8, 0);
    lv_obj_set_style_border_color(btn_cancel,
                lv_color_hex(CLR_INPUT_BORDER), 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_bg_color(btn_cancel,
                lv_color_hex(CLR_BTN_CANCEL_PR), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_cancel, btn_cancel_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, "取消");
    lv_obj_set_style_text_color(lbl_cancel, lv_color_hex(CLR_TEXT_SECONDARY), 0);
    lv_obj_center(lbl_cancel);

    /* 确认按钮 */
    lv_obj_t *btn_ok = lv_button_create(btn_row);
    lv_obj_set_size(btn_ok, 72, 34);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(CLR_BTN_OK_BG), 0);
    lv_obj_set_style_bg_opa(btn_ok, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_ok, 8, 0);
    lv_obj_set_style_border_width(btn_ok, 0, 0);
    lv_obj_set_style_bg_color(btn_ok,
                lv_color_hex(CLR_BTN_OK_PR), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_ok, btn_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_ok = lv_label_create(btn_ok);
    lv_label_set_text(lbl_ok, "确认");
    lv_obj_set_style_text_color(lbl_ok, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl_ok);
}
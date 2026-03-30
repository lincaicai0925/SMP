#ifndef STYLES_H
#define STYLES_H
#include "lvgl.h"
#include "smp_private.h"

#define CART_BG_COLOR           lv_color_hex(0x353131)
#define SHADOW_BG_COLOR         lv_color_hex(0x837C7C)

/* ═══════════════════════════════════════════════
 * 全局统一配色方案
 * ═══════════════════════════════════════════════ */

/* 背景色 */
#define STYLE_BG_DARK           lv_color_hex(0x1A1D23)   /* 主背景 */
#define STYLE_BG_PANEL          lv_color_hex(0x252830)   /* 面板/卡片背景 */
#define STYLE_BG_HEADER         lv_color_hex(0x2D3142)   /* 头部/工具栏背景 */

/* 边框 & 分隔线 */
#define STYLE_BORDER            lv_color_hex(0x3A3F4B)   /* 通用边框 */

/* 文本色 */
#define STYLE_TEXT_PRIMARY      lv_color_hex(0xC8CDD8)   /* 主文本 */
#define STYLE_TEXT_SECONDARY    lv_color_hex(0x6B7280)   /* 次要/暗淡文本 */
#define STYLE_TEXT_WHITE        lv_color_hex(0xFFFFFF)   /* 纯白文本 */

/* 表格行交替色 */
#define STYLE_ROW_ODD           lv_color_hex(0x1C1F26)
#define STYLE_ROW_EVEN          lv_color_hex(0x1A1D23)

/* 强调色 & 状态色 */
#define STYLE_ACCENT            lv_color_hex(0x3B82F6)   /* 蓝色强调 */
#define STYLE_SUCCESS           lv_color_hex(0x22C55E)   /* 绿色/成功 */

/* 标签背景 & 前景 */
#define STYLE_TAG_ERROR_BG      lv_color_hex(0x7F1D1D)   /* 故障标签背景 */
#define STYLE_TAG_ERROR_FG      lv_color_hex(0xFCA5A5)   /* 故障标签文字 */
#define STYLE_TAG_SUCCESS_BG    lv_color_hex(0x14532D)   /* 运行标签背景 */
#define STYLE_TAG_SUCCESS_FG    lv_color_hex(0x86EFAC)   /* 运行标签文字 */
#define STYLE_TAG_INFO_BG       lv_color_hex(0x1E3A5F)   /* 调试标签背景 */
#define STYLE_TAG_INFO_FG       lv_color_hex(0x93C5FD)   /* 调试标签文字 */
#define STYLE_TAG_WARNING_BG    lv_color_hex(0x7C2D12)   /* 通信标签背景 */
#define STYLE_TAG_WARNING_FG    lv_color_hex(0xFDBA74)   /* 通信标签文字 */

/* 操作按钮色 */
#define STYLE_BTN_DANGER        lv_color_hex(0xDC2626)   /* 危险/警告按钮 */
#define STYLE_BTN_PRIMARY       lv_color_hex(0x2563EB)   /* 主要按钮 */
#define STYLE_BTN_SUCCESS       lv_color_hex(0x16A34A)   /* 成功按钮 */
#define STYLE_BTN_WARNING       lv_color_hex(0xD97706)   /* 提醒按钮 */

void Set_Card_Style(lv_obj_t* obj);
#endif  // STYLES_H


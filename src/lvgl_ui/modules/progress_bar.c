#include "lvgl.h"
#include "modules/include/progress_bar.h"

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_24);

progress_bar_t Progress_Bar_Create(const char *title)
{
    progress_bar_t pb;

    /* ---- 半透明遮罩 ---- */
    pb.mask = lv_obj_create(lv_screen_active());
    lv_obj_set_size(pb.mask, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(pb.mask, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pb.mask, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(pb.mask, 0, LV_PART_MAIN);
    lv_obj_remove_flag(pb.mask, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 卡片 ---- */
    pb.card = lv_obj_create(lv_screen_active());
    lv_obj_set_size(pb.card, 420, 180);
    lv_obj_center(pb.card);
    lv_obj_remove_flag(pb.card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(pb.card, lv_color_hex(0x2B2B2B), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pb.card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(pb.card, 16, LV_PART_MAIN);
    lv_obj_set_style_border_width(pb.card, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(pb.card, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_border_opa(pb.card, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(pb.card, 40, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(pb.card, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(pb.card, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(pb.card, 8, LV_PART_MAIN);

    lv_obj_set_flex_flow(pb.card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pb.card,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(pb.card, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_row(pb.card, 16, LV_PART_MAIN);

    /* ---- 标题 ---- */
    lv_obj_t *lbl_title = lv_label_create(pb.card);
    lv_label_set_text(lbl_title, title);
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);

    /* ---- 进度条 ---- */
    pb.bar = lv_bar_create(pb.card);
    lv_obj_set_size(pb.bar, LV_PCT(90), 14);
    lv_bar_set_range(pb.bar, 0, 100);
    lv_bar_set_value(pb.bar, 0, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(pb.bar, lv_color_hex(0x3A3A3A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pb.bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(pb.bar, 7, LV_PART_MAIN);

    lv_obj_set_style_bg_color(pb.bar, lv_color_hex(0x00C8FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(pb.bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(pb.bar, 7, LV_PART_INDICATOR);

    /* ---- 百分比 + 信息行 ---- */
    lv_obj_t *bottom_row = lv_obj_create(pb.card);
    lv_obj_set_size(bottom_row, LV_PCT(90), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(bottom_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bottom_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bottom_row, 0, LV_PART_MAIN);
    lv_obj_remove_flag(bottom_row, LV_OBJ_FLAG_SCROLLABLE);

    pb.pct_label = lv_label_create(bottom_row);
    lv_label_set_text(pb.pct_label, "0%");
    lv_obj_set_style_text_color(pb.pct_label, lv_color_hex(0x00C8FF), LV_PART_MAIN);
    lv_obj_align(pb.pct_label, LV_ALIGN_LEFT_MID, 0, 0);

    pb.info_label = lv_label_create(bottom_row);
    lv_label_set_text(pb.info_label, "");
    lv_obj_set_style_text_color(pb.info_label, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align(pb.info_label, LV_ALIGN_RIGHT_MID, 0, 0);

    return pb;
}

void Progress_Bar_Update(progress_bar_t *pb, int32_t pct, const char *info)
{
    if(!pb || !pb->bar) return;
    if(pct < 0) pct = 0;
    if(pct > 100) pct = 100;

    lv_bar_set_value(pb->bar, pct, LV_ANIM_ON);

    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)pct);
    lv_label_set_text(pb->pct_label, buf);

    if(info) {
        lv_label_set_text(pb->info_label, info);
    }
}

void Progress_Bar_Destroy(progress_bar_t *pb)
{
    if(!pb) return;
    if(pb->card) { lv_obj_delete(pb->card); pb->card = NULL; }
    if(pb->mask) { lv_obj_delete(pb->mask); pb->mask = NULL; }
    pb->bar = NULL;
    pb->pct_label = NULL;
    pb->info_label = NULL;
}

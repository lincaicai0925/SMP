#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H
#include "lvgl.h"

typedef struct {
    lv_obj_t *mask;         // 半透明遮罩
    lv_obj_t *card;         // 弹窗卡片
    lv_obj_t *bar;          // 进度条
    lv_obj_t *pct_label;    // 百分比文字
    lv_obj_t *info_label;   // 附加信息（如 "已加载 1234 条"）
} progress_bar_t;

/**
 * @brief 创建一个模态进度条弹窗
 * @param title 标题文字（如 "正在加载数据字典..."）
 * @return progress_bar_t 句柄，用于后续更新和销毁
 */
progress_bar_t Progress_Bar_Create(const char *title);

/**
 * @brief 更新进度
 * @param pb   句柄
 * @param pct  百分比 0~100
 * @param info 附加信息，NULL 则不更新
 */
void Progress_Bar_Update(progress_bar_t *pb, int32_t pct, const char *info);

/**
 * @brief 销毁进度条弹窗
 */
void Progress_Bar_Destroy(progress_bar_t *pb);

#endif /* PROGRESS_BAR_H */

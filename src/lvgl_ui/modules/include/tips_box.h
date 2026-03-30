#ifndef TIPS_BOX_H
#define TIPS_BOX_H

#include "lvgl.h"

/**
 * 在屏幕顶部弹出一条 toast 通知。
 * 多条通知会自动向下堆叠，每条 5 秒后自动销毁。
 *
 * @param msg   要显示的消息文本
 * @param color 文本颜色
 */
void Create_Tips_Box(const char *msg, uint32_t color);

#endif /* TIPS_BOX_H */
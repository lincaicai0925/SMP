#ifndef ARRANGE_DIALOG_H
#define ARRANGE_DIALOG_H

#include "lvgl.h"

/* 自动排列回调类型：由主模块提供排列实现 */
typedef void (*Arrange_Cb_t)(int rows, int cols, int gap);

/* 打开自动排列对话框（按钮回调中调用） */
void Arrange_Dialog_Open(Arrange_Cb_t cb);

#endif /* ARRANGE_DIALOG_H */

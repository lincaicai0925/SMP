#ifndef WINDOW_MODULE_H
#define WINDOW_MODULE_H

#include "smp_private.h"

void Window_Module_Init(void);
void Window_Module_Cleanup(void);
void Window_Module_Weight(smp_ctx_t *ctx);
void Window_Module_SetTitle(const char *title);
void Window_Module_SetContent(lv_obj_t *content);
void Window_Module_SetStyle(lv_style_t *style);
void Window_Module_SetTitleBarStyle(lv_style_t *style);
void Window_Module_SetContentStyle(lv_style_t *style);

#endif // WINDOW_MODULE_H

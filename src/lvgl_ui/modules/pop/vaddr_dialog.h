#ifndef VADDR_DIALOG_H
#define VADDR_DIALOG_H

#include "lvgl.h"

/* 确认回调: vaddr=地址, name=名称(若弹窗未启用名称则为NULL) */
typedef void (*vaddr_confirm_cb_t)(uint32_t vaddr, const char *name, void *user_data);

/* 基础版：仅设置虚拟地址 */
void Vaddr_Dialog_Open(lv_obj_t *trigger,
                       uint32_t  cur_vaddr,
                       vaddr_confirm_cb_t on_confirm,
                       void *user_data);

/* 扩展版：同时设置名称和虚拟地址, cur_name 非 NULL 时显示名称输入框 */
void Vaddr_Dialog_Open_Ex(lv_obj_t *trigger,
                          uint32_t  cur_vaddr,
                          const char *cur_name,
                          uint32_t  name_max_len,
                          vaddr_confirm_cb_t on_confirm,
                          void *user_data);

#endif /* VADDR_DIALOG_H */

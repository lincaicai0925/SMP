#ifndef EXPERIMENTAL_DATA_H
#define EXPERIMENTAL_DATA_H

/*
 * --ATTENTION--
 * 注意请不要在没有menu的页面访问menu， 因为它将是一个空指针!!! 
 * 若你的确需要一个二级菜单，请更改 main_window.c 的 
 * SMP_Navigation_Create(lv_obj_t* base_obj)函数里的
 * static void Nav_Obj_Create(lv_obj_t * cur_obj,  Nav_Page_t page, uint8_t is_need_menu, 
                                const lv_image_dsc_t * icon_img_dsc, nav_obj_cb_t app_cb)
 * 第3个参数改为 true
 * 
 * 若您使用了lv_malloc_XX等系列函数分配了内存，为了提高程序的健壮性请调用lv_free释放分配的内存
 * 注册一个删除事件的回调函数，在回调函数中释放内存， 可以像下面这样添加删除回调：
 * lv_obj_add_event_cb(base_obj, Chart_Delete, LV_EVENT_DELETE, m_chart);
 * 
 * 它的原理是当程序被关闭时，会向各个页面的activity_content以及menu（若有的话）发送删除事件，通知各个页面做好删除工作
 * 这个事件会递归传递到各个子对象。
 */


#include "smp_private.h"

void Experimental_Data_Weight(smp_ctx_t *ctx);

#endif /* EXPERIMENTAL_DATA_H */

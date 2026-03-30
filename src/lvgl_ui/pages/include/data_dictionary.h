#ifndef DATA_DICTIONARY_H
#define DATA_DICTIONARY_H

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
#include "clabez/include/cmap.h"

// bus_addr, device_name, desc, data_type, data_len, rw, alarm_level, 操作
#define TABLE_COLUMN_CNT        8
#define TABLE_ROW_HIGHT         40
#define LAZY_LOAD_BATCH_SIZE    100                                 // 每页渲染的条数

#define TABLE_BG_COLOR          lv_color_hex(0x252830)

#define JSON_BUFFER_SIZE        2048


#define DESC_MAX_LEN                1024
#define DEVICE_NAME_MAX_LEN         128
#define RW_MAX_LEN                  10
#define DATA_TYPW_MAX_LEN           10
#define MODBUS_ADDR_MAX_LEN         16

#define PROGRESS_BAR_DELAY          5

#define CLR_PANEL       0x252830
#define CLR_BORDER_PG   0x3A3F4B
#define CLR_TEXT_DIM    0x6B7280
#define CLR_ACCENT      0x15b1e0
#define CLR_BG_PG       0x1A1D23
#define CLR_ROW_ODD     0x252830
#define CLR_ROW_EVEN    0x1A1D23
#define CLR_HEADER_BG   0x252830
#define CLR_ACCENT_DARK 0x4415E0
#define PAGER_H         44
#define FILTERBAR_H     40
void Experimental_Data_Weight(smp_ctx_t *ctx);
typedef struct {
    char desc[DESC_MAX_LEN];
    char device_name[DEVICE_NAME_MAX_LEN];
    char modbus_addr[MODBUS_ADDR_MAX_LEN];
    char data_type[DATA_TYPW_MAX_LEN];
    char rw[RW_MAX_LEN];

    uint32_t bus_addr;
    uint32_t data_len;
    double thmin;
    double thmax;
    uint32_t alarm_level;

    double cur_val;
    double default_value;

    uint32_t decimal;
    uint32_t subtype;
    uint32_t bit_offset;

} data_dict_item_dsc_t;

typedef int8_t (*data_dict_param_cb) (const data_dict_item_dsc_t*);

DEFINE_CMAP(uint32_t, data_dict_item_dsc_t);

// 全局数据字典 (bus_addr -> data_dict_item_dsc_t)
extern cmap_uint32_t_data_dict_item_dsc_t* g_data_dict;

void Data_Dictionary_Weight(smp_ctx_t *ctx);

#endif /* DATA_DICTIONARY_H */

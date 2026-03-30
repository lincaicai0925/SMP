#ifndef _DATA_DISTRIBUTION_H
#define _DATA_DISTRIBUTION_H

#include "clabez/include/cmap.h"
#include "clabez/include/clist.h"
#include "modules/include/mpmc_queue.h"
#define RING_BUFFER_SIZE            4096

enum value_type_t{
    VALUE_TYPE_QUEUE,
    VALUE_TYPE_OTHER
};

typedef struct{
    float       *data[RING_BUFFER_SIZE];            // 实际数据存储的位置
    MPMC_Queue  *queue;                             // 队列
    MPMC_Queue  *free_pool;                         // 空闲槽回收队列
}mpmc_queue_t;

typedef struct 
{       
    enum value_type_t type;                         // 控件类型   --- 用于使用那个数据
    float widget_data;
    mpmc_queue_t chart_queue;
}widght_data_dsc_t;

typedef widght_data_dsc_t widget_data;

DEFINE_CLIST_PTR(widget_data);
typedef struct
{
    uint32_t list_cnt;
    clist_widget_data_ptr *data;
}map_clist_dsc_t;

typedef uint32_t addr;
typedef map_clist_dsc_t map_clist_t;

DEFINE_CMAP(addr, map_clist_t);

extern cmap_addr_map_clist_t *addr_map;

void Data_Map_Init(void);
widget_data* Data_Map_Addr_Bind(uint32_t addr, widget_data *wd);
void Data_Map_Addr_Remove(uint32_t addr, widget_data *target);
void Data_Map_Destroy(void);

#endif /* _DATA_DISTRIBUTION_H */

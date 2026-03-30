//TODO 注意库的返回值有问题，clist_widget_data_ptr_push_back(new_entry.data, wd) != 1

#include "lvgl.h"
#include "data_distribution.h"
#include "clabez/include/cthread.h"
#include <stdio.h>
#include <math.h>

static cthread test_thread;
static volatile int test_running = 0;

cmap_addr_map_clist_t *addr_map = NULL;
void Data_Map_Test_Start(void);
void Data_Map_Test_Stop(void);

void Data_Map_Init(void)
{
    addr_map = cmap_addr_map_clist_t_create();
    LV_ASSERT_NULL(addr_map);
    Data_Map_Test_Start();
}

/**
 * @brief 根据地址生成不同波形的测试数据
 *   地址1: 正弦波    (平滑周期信号)
 *   地址2: 锯齿波    (线性上升后突降)
 *   地址3: 脉冲/方波 (高低电平交替)
 *   地址4: 三角波    (线性上升再线性下降)
 *   其他:  余弦波    (默认)
 */
static float generate_signal(uint32_t addr, float t)
{
    switch (addr) {
    case 1: /* 正弦波: 幅值 -1-1, 周期 ~628 步 */
        return sinf(t * 0.1f);
    case 6:
        return sinf(t * 0.1f);

    case 2: { /* 锯齿波: 幅值 -100~100, 周期 200 步 */
        float period = 200.0f;
        float phase = fmodf(t, period) / period;        /* 0.0 ~ 1.0 */
        return (phase * 2.0f - 1.0f);          /* -100 ~ 100 */
    }

    case 3: { /* 脉冲/方波: ±80, 周期 100 步, 占空比 50% */
        float period = 100.0f;
        float phase = fmodf(t, period);
        return (phase < period * 0.5f) ? 80.0f : -80.0f;
    }

    case 4: { /* 三角波: 幅值 -100~100, 周期 100 步 */
        float period = 100.0f;
        float phase = fmodf(t, period) / period;        /* 0.0 ~ 1.0 */
        /* 前半周期上升，后半周期下降 */
        float v = (phase < 0.5f)
                  ? (phase * 4.0f - 1.0f)               /* -1 → 1 */
                  : (3.0f - phase * 4.0f);               /* 1 → -1 */
        return v * 100.0f;
    }

    case 5: { /* 振幅渐增正弦波: 包络周期内幅值 0→2500, 正弦周期 300 步 */
        float sin_period = 300.0f;          /* 单个正弦周期(步), 比其他波形长 */
        float envelope_period = 4000.0f;    /* 包络总长(步), 走完后振幅归零重来 */
        float env_t = fmodf(t, envelope_period);
        float amplitude = (env_t / envelope_period) * 2500.0f;  /* 0 → 2500 线性增长 */
        return sinf(env_t * 6.2831853f / sin_period) * amplitude;
    }
    
    default: /* 余弦波 */
        return cosf(t * 0.08f) * 60.0f;
    }
}

// TODO [高风险-竞态] 此线程遍历 addr_map 时无同步机制，若主线程同时调用
//      Data_Map_Addr_Remove / Data_Map_Destroy 会导致 UAF。
//      需要为 addr_map 的读写加互斥锁，或在遍历时持锁。
static void* Test_Inject_Thread(void *arg)
{
    (void)arg;
    float counter = 0.0f;

    while (test_running) {
        if (!addr_map || cmap_addr_map_clist_t_size(addr_map) == 0) {
            lv_delay_ms(100);
            continue;
        }

        // 遍历 map 中每个地址
        cmap_foreach(addr_map, addr, map_clist_t, it) {
            if (!it->value_ptr->data) continue;

            float value = generate_signal(*it->key_ptr, counter);

            // 遍历该地址绑定的所有控件
            clist_as_foreach(it->value_ptr->data, widget_data *, wd, widget_data_ptr) {
                if (!wd) continue;

                if (wd->type == VALUE_TYPE_QUEUE) {
                    if(wd->chart_queue.free_pool == NULL || wd->chart_queue.queue == NULL )
                        continue;

                    void *slot = NULL;
                    if (MPMC_Queue_Dequeue(wd->chart_queue.free_pool, &slot)) {
                        *(float *)slot = value;
                        MPMC_Queue_Enqueue(wd->chart_queue.queue, slot);
                    }
                } else {
                    if(wd->widget_data)
                        wd->widget_data = value;
                }
            }
        }

        counter += 1.0f;

        cthread_timespec t = {.tv_sec = 0, .tv_nsec = 10000000};
        cthread_sleep_for(&t);
    }
    return NULL;
}

void Data_Map_Test_Start(void)
{
    if (test_running) return;
    test_running = 1;
    cthread_create(&test_thread, Test_Inject_Thread, NULL);
    printf("[TEST] inject thread started.\n");
}

void Data_Map_Test_Stop(void)
{
    if (!test_running) return;
    test_running = 0;
    cthread_join(&test_thread);
    printf("[TEST] inject thread stopped.\n");
}

widget_data* Data_Map_Addr_Bind(uint32_t addr, widget_data *wd)
{
    cmap_addr_map_clist_t_node *node = cmap_addr_map_clist_t_find(addr_map, addr);
    if (node != cmap_addr_map_clist_t_end(addr_map)) {
        // 键已存在，追加控件指针到链表
        map_clist_t *list = cmap_addr_map_clist_t_get_value_ptr(node);
        if (clist_widget_data_ptr_push_back(list->data, wd) == 0) return NULL;
        list->list_cnt++;
        return wd;
    } else {
        // 键不存在，创建新链表并插入
        map_clist_t new_entry = {0};
        new_entry.data = clist_widget_data_ptr_create();
        if (!new_entry.data) 
            return NULL;
        // TODO 这里库的返回值有问题，需要注意替换
        if (clist_widget_data_ptr_push_back(new_entry.data, wd) != 1) {
            clist_widget_data_ptr_destroy(&new_entry.data);
            return NULL;
        }
        new_entry.list_cnt = 1;
        if (cmap_addr_map_clist_t_insert(addr_map, addr, new_entry) != 1) {
            clist_widget_data_ptr_destroy(&new_entry.data);
            return NULL;
        }
        return wd;
    }
}


void Data_Map_Addr_Remove(uint32_t addr, widget_data *target)
{
    cmap_addr_map_clist_t_node *node = cmap_addr_map_clist_t_find(addr_map, addr);
    if (node == cmap_addr_map_clist_t_end(addr_map)) return;

    map_clist_t *list = cmap_addr_map_clist_t_get_value_ptr(node);
    if (list->list_cnt == 1) {
        // 仅一个元素，直接删除整个键值对
        clist_widget_data_ptr_destroy(&list->data);
        cmap_addr_map_clist_t_remove(addr_map, addr);
    } else {
        // 遍历链表，通过指针值匹配目标元素
        clist_as_foreach(list->data, widget_data *, item, widget_data_ptr)
        {
            if (item == target)
            {
                clist_widget_data_ptr_erase(list->data, item_iter);
                list->list_cnt--;
                break;
            }
        }
    }
}

void Data_Map_Destroy(void)
{
    cmap_foreach(addr_map, addr, map_clist_t, it) {
        clist_widget_data_ptr_destroy(&it->value_ptr->data);
    }
    cmap_addr_map_clist_t_destroy(&addr_map);
    addr_map = NULL;
}

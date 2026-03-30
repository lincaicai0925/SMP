#ifndef LOG_MANAGMENT_H
#define LOG_MANAGMENT_H

#include "smp_private.h"
#include <time.h>
#include <stdbool.h>

/* ── 日志数据结构体 ─────────────────────────── */
typedef struct {
    time_t timestamp;
    char date[16];
    char start_time[16];
    char end_time[16];
    char dev_name[16];
    char log_category[32];
    char event_level[16];
    char event_type[32];
    char event_content[128];
} log_data_t;

/* ── 全局数据────── */
extern log_data_t *log_datas;
extern int         total_log_count;

/* ── 公开接口 ───────────────────────────────── */
void Log_Managment_Weight(smp_ctx_t *ctx);
void Init_Test_Log_Data_Batch(int count);

#endif /* LOG_MANAGMENT_H */


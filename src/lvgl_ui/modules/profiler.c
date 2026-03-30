/**
 * @file profiler.c
 * @brief LVGL 性能分析器实现 - 跨平台（Windows / Linux）
 * @details 提供基于平台原生 API 的高精度时间戳和性能分析功能
 */

#ifdef _WIN32
#include <windows.h>
#else
#define _GNU_SOURCE
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include "lvgl.h"

/* 性能分析日志文件句柄 */
static FILE *s_profiler_log_file = NULL;

static uint32_t Get_tick_us_cb(void)
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (uint32_t)((counter.QuadPart * 1000000) / frequency.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
#endif
}

static int Get_tid_cb(void)
{
#ifdef _WIN32
    return (int)GetCurrentThreadId();
#else
    return (int)syscall(SYS_gettid);
#endif
}

static int Get_cpu_cb(void)
{
#ifdef _WIN32
    return (int)GetCurrentProcessorNumber();
#else
    return sched_getcpu();
#endif
}

static void Log_print_cb(const char * buf)
{
    if (s_profiler_log_file != NULL) {
        fprintf(s_profiler_log_file, "%s", buf);
        fflush(s_profiler_log_file);
    }
}

void My_Profiler_Init(void)
{
    s_profiler_log_file = fopen("profiler_log.txt", "w");
    if (s_profiler_log_file == NULL) {
        return;
    }

    lv_profiler_builtin_config_t config;
    lv_profiler_builtin_config_init(&config);

    config.tick_per_sec = 1000000; /* 一秒等于1000000微秒 */
    config.tick_get_cb  = Get_tick_us_cb;
    config.tid_get_cb   = Get_tid_cb;
    config.cpu_get_cb   = Get_cpu_cb;
    config.flush_cb     = Log_print_cb;

    lv_profiler_builtin_init(&config);
}
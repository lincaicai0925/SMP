#include "lvgl.h"

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* needed for usleep() */
#endif

#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include "SDL2/SDL.h"

#include "../lvgl_ui/hal/hal.h"
#include "main_window.h"
#include "modules/include/profiler.h"

int main(int argc, char **argv)
{
  (void)argc; 
  (void)argv;

  /* 初始化 LVGL */
  lv_init();

  /* 为LVGL初始化 HAL (display, input devices, tick) */
  sdl_hal_init(1920, 1080);
  lv_sdl_window_set_resizeable(lv_display_get_default(), true);
  SDL_StartTextInput();

  /* 创建主窗口 */
  Main_Window_Create(lv_screen_active());
  
  // My_Profiler_Init();
  while (1)
  {
    /* 定期调用 lv_task 处理程序
     * 也可以在定时器中断或操作系统任务中执行。*/
    uint32_t sleep_time_ms = lv_timer_handler();
    if (sleep_time_ms == LV_NO_TIMER_READY)
    {
      sleep_time_ms = LV_DEF_REFR_PERIOD;
    }
#ifdef _MSC_VER
    Sleep(sleep_time_ms);
#else
    usleep(sleep_time_ms * 1000);
#endif
  }

  return 0;
}


void My_Assert_Handler(const char *file, int line, const char *func)
{
  printf("LVGL Assert failed! File: %s, Line: %d, Function: %s\n", file, line, func);
  exit(2);
}

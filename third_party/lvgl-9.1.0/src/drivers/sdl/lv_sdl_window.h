/**
 * @file lv_sdl_window.h
 *
 */

#ifndef LV_SDL_DISP_H
#define LV_SDL_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../display/lv_display.h"
#include "../../indev/lv_indev.h"

#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include LV_SDL_INCLUDE_PATH

#if LV_USE_DRAW_SDL
    #include <SDL2/SDL_image.h>
#endif

#if LV_USE_SDL
#include <stdbool.h>
#include "../../core/lv_refr.h"
#include "../../stdlib/lv_string.h"
#include "../../core/lv_global.h"
#include "../../lv_init.h"

/*********************
 *      DEFINES
 *********************/

/* Possible values of LV_SDL_MOUSEWHEEL_MODE */
#define LV_SDL_MOUSEWHEEL_MODE_ENCODER  0  /* The mousewheel emulates an encoder input device*/
#define LV_SDL_MOUSEWHEEL_MODE_CROWN    1  /* The mousewheel emulates a smart watch crown*/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
#if LV_USE_DRAW_SDL == 0
    SDL_Texture * texture;
    uint8_t * fb1;
    uint8_t * fb2;
    uint8_t * fb_act;
    uint8_t * buf1;
    uint8_t * buf2;
#endif
    uint8_t zoom;
    uint8_t ignore_size_chg;
    
    bool resizing;           // 是否正在调整大小
    int resize_edge;         // 调整大小的边缘
    int resize_start_x;      // 调整开始时的鼠标X坐标
    int resize_start_y;      // 调整开始时的鼠标Y坐标
    int resize_start_width;  // 调整开始时的窗口宽度
    int resize_start_height; // 调整开始时的窗口高度
} lv_sdl_window_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_display_t * lv_sdl_window_create(int32_t hor_res, int32_t ver_res);

void lv_sdl_window_set_resizeable(lv_display_t * disp, bool value);

void lv_sdl_window_set_zoom(lv_display_t * disp, uint8_t zoom);

uint8_t lv_sdl_window_get_zoom(lv_display_t * disp);

lv_display_t * _lv_sdl_get_disp_from_win_id(uint32_t win_id);

void lv_sdl_window_set_title(lv_display_t * disp, const char * title);

void * lv_sdl_window_get_renderer(lv_display_t * disp);

void lv_sdl_quit();

/**********************
 *      MACROS
 **********************/

#endif /* LV_DRV_SDL */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_SDL_DISP_H */

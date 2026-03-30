/**
 * @file lv_sdl_keyboard.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_sdl_keyboard.h"
#if LV_USE_SDL

#include "../../indev/lv_indev.h"
#include "../../core/lv_group.h"
#include "../../stdlib/lv_string.h"
#include "../../widgets/textarea/lv_textarea.h"
#include LV_SDL_INCLUDE_PATH

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    char buf[KEYBOARD_BUFFER_SIZE];
    bool dummy_read;
    uint32_t comp_chars; 
} lv_sdl_keyboard_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void sdl_keyboard_read(lv_indev_t * indev, lv_indev_data_t * data);
static uint32_t keycode_to_ctrl_key(SDL_Keycode sdl_key);
static void release_indev_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_indev_t * lv_sdl_keyboard_create(void)
{
    lv_sdl_keyboard_t * dsc = lv_malloc_zeroed(sizeof(lv_sdl_keyboard_t));
    LV_ASSERT_MALLOC(dsc);
    if(dsc == NULL) return NULL;

    lv_indev_t * indev = lv_indev_create();
    LV_ASSERT_MALLOC(indev);
    if(indev == NULL) {
        lv_free(dsc);
        return NULL;
    }

    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, sdl_keyboard_read);
    lv_indev_set_driver_data(indev, dsc);
    lv_indev_set_mode(indev, LV_INDEV_MODE_EVENT);
    lv_indev_add_event_cb(indev, release_indev_cb, LV_EVENT_DELETE, indev);

    return indev;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void sdl_keyboard_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    lv_sdl_keyboard_t * dev = lv_indev_get_driver_data(indev);

    const size_t len = lv_strlen(dev->buf);

    /*Send a release manually*/
    if(dev->dummy_read) {
        dev->dummy_read = false;
        data->state = LV_INDEV_STATE_RELEASED;
    }
    /*Send the pressed character*/
    else if(len > 0) {
        dev->dummy_read = true;
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = dev->buf[0];
        memmove(dev->buf, dev->buf + 1, len);
    }
}

static void release_indev_cb(lv_event_t * e)
{
    lv_indev_t * indev = (lv_indev_t *) lv_event_get_user_data(e);
    lv_sdl_keyboard_t * dev = lv_indev_get_driver_data(indev);
    if(dev) {
        lv_indev_set_driver_data(indev, NULL);
        lv_indev_set_read_cb(indev, NULL);
        lv_free(dev);
        LV_LOG_INFO("done");
    }
}
// 计算 UTF-8 字符串中的字符个数（非字节数）
static uint32_t utf8_char_count(const char * str)
{
    uint32_t count = 0;
    while(*str) {
        uint8_t b = (uint8_t)*str;
        if((b & 0x80) == 0)         str += 1;
        else if((b & 0xE0) == 0xC0) str += 2;
        else if((b & 0xF0) == 0xE0) str += 3;
        else if((b & 0xF8) == 0xF0) str += 4;
        else                         str += 1;
        count++;
    }
    return count;
}

// 将 UTF-8 字符索引（码点数）转换为字节偏移
static uint32_t utf8_char_idx_to_byte_offset(const char * str, uint32_t char_idx)
{
    const char * p = str;
    while(*p && char_idx > 0) {
        uint8_t b = (uint8_t)*p;
        if((b & 0x80) == 0)         p += 1;
        else if((b & 0xE0) == 0xC0) p += 2;
        else if((b & 0xF0) == 0xE0) p += 3;
        else if((b & 0xF8) == 0xF0) p += 4;
        else                         p += 1;
        char_idx--;
    }
    return (uint32_t)(p - str);
}

void _lv_sdl_keyboard_handler(SDL_Event * event)
{
    uint32_t win_id = UINT32_MAX;
    switch(event->type) {
        case SDL_KEYDOWN:      win_id = event->key.windowID;  break;
        case SDL_TEXTINPUT:    win_id = event->text.windowID; break;
        case SDL_TEXTEDITING:  win_id = event->edit.windowID; break;
        default: return;
    }

    lv_display_t * disp = _lv_sdl_get_disp_from_win_id(win_id);

    lv_indev_t * indev = lv_indev_get_next(NULL);
    while(indev) {
        if(lv_indev_get_display(indev) == disp &&
           lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
            break;
        }
        indev = lv_indev_get_next(indev);
    }
    if(indev == NULL) return;

    lv_sdl_keyboard_t * dsc = lv_indev_get_driver_data(indev);

    /* 拼音组合中，临时显示 */
    if(event->type == SDL_TEXTEDITING) {
        lv_group_t * g = lv_indev_get_group(indev);
        if(g == NULL) return;
        lv_obj_t * focused = lv_group_get_focused(g);
        if(focused == NULL || !lv_obj_check_type(focused, &lv_textarea_class)) return;

        /* 先删掉上一次的组合文本 */
        for(uint32_t i = 0; i < dsc->comp_chars; i++) {
            lv_textarea_delete_char(focused);
        }
        dsc->comp_chars = 0;

        if(event->edit.text[0] != '\0') {
            lv_textarea_add_text(focused, event->edit.text);
            dsc->comp_chars = utf8_char_count(event->edit.text);
        }
        return;
    }

    /*最终确认的文字 */
    if(event->type == SDL_TEXTINPUT) {
        lv_group_t * g = lv_indev_get_group(indev);
        if(g == NULL) return;
        lv_obj_t * focused = lv_group_get_focused(g);
        if(focused == NULL || !lv_obj_check_type(focused, &lv_textarea_class)) return;

        /* 删掉组合文本 */
        for(uint32_t i = 0; i < dsc->comp_chars; i++) {
            lv_textarea_delete_char(focused);
        }
        dsc->comp_chars = 0;

        /* 插入最终文字 */
        lv_textarea_add_text(focused, event->text.text);
        return;
    }

    /* Backspace / Delete：有选区时删除整个选中范围 */
    if(event->key.keysym.sym == SDLK_BACKSPACE || event->key.keysym.sym == SDLK_DELETE) {
        lv_group_t * g = lv_indev_get_group(indev);
        lv_obj_t * focused = g ? lv_group_get_focused(g) : NULL;
        if(focused && lv_obj_check_type(focused, &lv_textarea_class)) {
            lv_obj_t * label = lv_textarea_get_label(focused);
            uint32_t sel_s = lv_label_get_text_selection_start(label);
            uint32_t sel_e = lv_label_get_text_selection_end(label);
            if(sel_s != LV_DRAW_LABEL_NO_TXT_SEL && sel_e != LV_DRAW_LABEL_NO_TXT_SEL && sel_s != sel_e) {
                if(sel_s > sel_e) { uint32_t tmp = sel_s; sel_s = sel_e; sel_e = tmp; }
                const char * full = lv_textarea_get_text(focused);
                uint32_t byte_s = utf8_char_idx_to_byte_offset(full, sel_s);
                uint32_t byte_e = utf8_char_idx_to_byte_offset(full, sel_e);
                uint32_t full_len = lv_strlen(full);
                /* 拼出删除选区后的新字符串 */
                char * buf = lv_malloc(full_len - (byte_e - byte_s) + 1);
                if(buf) {
                    lv_memcpy(buf, full, byte_s);
                    lv_memcpy(buf + byte_s, full + byte_e, full_len - byte_e);
                    buf[full_len - (byte_e - byte_s)] = '\0';
                    lv_textarea_set_text(focused, buf);
                    lv_free(buf);
                }
                lv_textarea_set_cursor_pos(focused, sel_s);
                lv_textarea_clear_selection(focused);
                return;  /* 拦截，不再走逐字删逻辑 */
            }
        }
    }

    /* Ctrl+C / Ctrl+V */
    if(event->key.keysym.mod & KMOD_CTRL) {
        lv_group_t * g = lv_indev_get_group(indev);
        lv_obj_t * focused = g ? lv_group_get_focused(g) : NULL;
        bool is_ta = focused && lv_obj_check_type(focused, &lv_textarea_class);

        if(event->key.keysym.sym == SDLK_c && is_ta) {
            const char * full = lv_textarea_get_text(focused);
            if(full && full[0] != '\0') {
                /* 优先复制选中区域，无选区则复制全文 */
                lv_obj_t * label = lv_textarea_get_label(focused);
                uint32_t sel_s = lv_label_get_text_selection_start(label);
                uint32_t sel_e = lv_label_get_text_selection_end(label);
                if(sel_s != LV_DRAW_LABEL_NO_TXT_SEL && sel_e != LV_DRAW_LABEL_NO_TXT_SEL && sel_s != sel_e) {
                    if(sel_s > sel_e) { uint32_t tmp = sel_s; sel_s = sel_e; sel_e = tmp; }
                    /* sel_s/sel_e 是字符索引，转换为字节偏移再截取 */
                    uint32_t byte_s = utf8_char_idx_to_byte_offset(full, sel_s);
                    uint32_t byte_e = utf8_char_idx_to_byte_offset(full, sel_e);
                    uint32_t len = byte_e - byte_s;
                    char * buf = SDL_malloc(len + 1);
                    if(buf) {
                        lv_memcpy(buf, full + byte_s, len);
                        buf[len] = '\0';
                        SDL_SetClipboardText(buf);
                        SDL_free(buf);
                    }
                } else {
                    SDL_SetClipboardText(full);
                }
            }
            return;
        }
        if(event->key.keysym.sym == SDLK_v && is_ta) {
            char * txt = SDL_GetClipboardText();
            if(txt) {
                /* 删掉拼音组合中的临时文本 */
                for(uint32_t i = 0; i < dsc->comp_chars; i++) lv_textarea_delete_char(focused);
                dsc->comp_chars = 0;
                lv_textarea_add_text(focused, txt);
                SDL_free(txt);
            }
            return;
        }
    }

    const uint32_t ctrl_key = keycode_to_ctrl_key(event->key.keysym.sym);
    if(ctrl_key == '\0') return;

    const size_t len = lv_strlen(dsc->buf);
    if(len < KEYBOARD_BUFFER_SIZE - 1) {
        dsc->buf[len] = ctrl_key;
        dsc->buf[len + 1] = '\0';
    }

    size_t remain = lv_strlen(dsc->buf);
    while(remain) {
        lv_indev_read(indev);
        lv_indev_read(indev);
        remain--;
    }
}

/**
 * Convert a SDL key code to it's LV_KEY_* counterpart or return '\0' if it's not a control character.
 * @param sdl_key the key code
 * @return LV_KEY_* control character or '\0'
 */
static uint32_t keycode_to_ctrl_key(SDL_Keycode sdl_key)
{
    /*Remap some key to LV_KEY_... to manage groups*/
    switch(sdl_key) {
        case SDLK_RIGHT:
        case SDLK_KP_PLUS:
            return LV_KEY_RIGHT;

        case SDLK_LEFT:
        case SDLK_KP_MINUS:
            return LV_KEY_LEFT;

        case SDLK_UP:
            return LV_KEY_UP;

        case SDLK_DOWN:
            return LV_KEY_DOWN;

        case SDLK_ESCAPE:
            return LV_KEY_ESC;

        case SDLK_BACKSPACE:
            return LV_KEY_BACKSPACE;

        case SDLK_DELETE:
            return LV_KEY_DEL;

        case SDLK_KP_ENTER:
        case '\r':
            return LV_KEY_ENTER;

        case SDLK_TAB:
        case SDLK_PAGEDOWN:
            return LV_KEY_NEXT;

        case SDLK_PAGEUP:
            return LV_KEY_PREV;

        case SDLK_HOME:
            return LV_KEY_HOME;

        case SDLK_END:
            return LV_KEY_END;

        default:
            return '\0';
    }
}

#endif /*LV_USE_SDL*/

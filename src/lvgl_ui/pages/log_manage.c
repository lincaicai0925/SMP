#include "pages/include/log_manage.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "log_export.h"

//中文字体申明
LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_12);
LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);

/* ═══════════════════════════════════════════════
 * 颜色与尺寸常量
 * ═══════════════════════════════════════════════ */
#define CLR_BG          0x1A1D23
#define CLR_PANEL       0x252830
#define CLR_HEADER_BG   0x2D3142
#define CLR_BORDER      0x3A3F4B
#define CLR_TEXT        0xC8CDD8
#define CLR_TEXT_DIM    0x6B7280
#define CLR_TEXT_WHITE  0xFFFFFF
#define CLR_ROW_ODD     0x1C1F26
#define CLR_ROW_EVEN    0x1A1D23
#define CLR_ACCENT      0x3B82F6
#define CLR_GREEN       0x22C55E

/* tag 颜色 */
#define CLR_FAULT_BG    0x7F1D1D
#define CLR_FAULT_FG    0xFCA5A5
#define CLR_RUN_BG      0x14532D
#define CLR_RUN_FG      0x86EFAC
#define CLR_DEBUG_BG    0x1E3A5F
#define CLR_DEBUG_FG    0x93C5FD
#define CLR_COMM_BG     0x7C2D12
#define CLR_COMM_FG     0xFDBA74

/* 操作按钮颜色 */
#define CLR_BTN_WARN    0xDC2626
#define CLR_BTN_COPY    0x2563EB
#define CLR_BTN_GREEN   0x16A34A
#define CLR_BTN_ORANGE  0xD97706

/* 行高 / 列宽 */
#define ROW_H         38
#define COL_CB_W      44
#define COL_NO_W      55
#define COL_TIME_W    180   
#define COL_DEV_W     110
#define COL_TYPE_W    130
#define COL_INFO_W    130
#define COL_ACT_W     78
#define TOPBAR_H      52
#define FILTERBAR_H   40
#define PAGER_H       44

/* 最大支持的日志条数 */
#define MAX_LOG_ENTRIES  1024

/* 查询条件结构体 */
typedef struct {
    int  cat;
    int  name;
    int  level;
    char start_str[32];
    char end_str[32];
    int  page;
    int  page_size;
} log_query_t;

/* 函数指针类型 */
typedef bool (*log_fetch_fn_t)(const log_query_t *query,
                                log_data_t        *out_buf,
                                int               *out_count,
                                int               *out_total);

static log_fetch_fn_t g_fetch_cb = NULL;

void Log_Register_Fetch(log_fetch_fn_t fn)
{
    g_fetch_cb = fn;
}

/* 当前页数据缓冲 */
#define PAGE_BUF_MAX  64
static log_data_t g_page_buf[PAGE_BUF_MAX];
static int        g_page_buf_count = 0;   
static int        g_query_total    = 0;
static int        g_page_global_idx[PAGE_BUF_MAX];  

/* 全局数据 */
log_data_t  *log_datas        = NULL;
int          total_log_count  = 0;
static bool  log_selected[MAX_LOG_ENTRIES];
static void populate_table(void);
static lv_obj_t  *g_start_date_ta = NULL;  
static lv_obj_t  *g_start_time_ta = NULL; 
static lv_obj_t  *g_end_date_ta   = NULL; 
static lv_obj_t  *g_end_time_ta   = NULL;  
static smp_ctx_t *g_ctx       = NULL;
static lv_obj_t  *g_scroll    = NULL;  
static lv_obj_t  *g_page_lbl  = NULL;  
static lv_obj_t  *g_num_lbl   = NULL;
static lv_obj_t  *g_total_page_lbl = NULL;  
static lv_obj_t  *g_allcb_obj  = NULL;  
static lv_obj_t  *g_allcb_icon = NULL; 
static lv_obj_t  *g_empty_lbl  = NULL;    

/* 分类 / 名称 / 级别筛选 */
static lv_obj_t  *g_dd_cat    = NULL;
static lv_obj_t  *g_dd_name   = NULL;
static lv_obj_t  *g_dd_level  = NULL;
static lv_obj_t  *g_dd_perpage = NULL;

static int   g_current_page   = 0; 
static int   g_items_per_page = 14;
static bool  g_time_filter    = false;
static char  g_start_str[32]  = {0};
static char  g_end_str[32]    = {0};
static int   g_sel_cat        = 0; 
static int   g_sel_name       = 0;
static int   g_sel_level      = 0;
static bool  g_all_selected   = false;

/* 筛选后的索引缓存 */
static int   g_filtered[MAX_LOG_ENTRIES];
static int   g_filtered_count = 0;
static int   g_total_pages    = 1;

/* matches() 用到的字符串数组提到全局，避免每次调用都在栈上创建 */
static const char * const g_cat_names[]   = {"","测试流程日志","设备运行日志","异常与报警日志","通信交互日志"};
static const char * const g_event_names[] = {"","CamOsc_Calres","Sys Running","Temp Warning","Data Update"};
static const char * const g_level_names[] = {"","Fatal","Error","Warning","Info","Debug","Trace"};

/* 工具函数 */
static inline lv_color_t C(uint32_t c) { return lv_color_hex(c); }

static void set_bg(lv_obj_t *o, uint32_t c)
{
    lv_obj_set_style_bg_color(o, C(c), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
}

static lv_obj_t *clean_cont(lv_obj_t *parent)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
    return o;
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *txt,
                              uint32_t color, const lv_font_t *font)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, C(color), 0);
    if (font) lv_obj_set_style_text_font(l, font, 0);
    return l;
}

static lv_obj_t *make_btn(lv_obj_t *parent, const char *txt,uint32_t bg, uint32_t fg, int w, int h)
{
    lv_obj_t *b = lv_btn_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_size(b, w, h);
    set_bg(b, bg);
    lv_obj_set_style_radius(b, 4, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, C(fg), 0);
    lv_obj_set_style_text_font(l, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_center(l);
    return b;
}

static void make_sep(lv_obj_t *parent)
{
    lv_obj_t *s = clean_cont(parent);
    lv_obj_set_size(s, LV_PCT(100), 1);
    set_bg(s, CLR_BORDER);
}

/* 弹窗关闭/删除时重置列表滚动位置 */
static void msgbox_delete_cb(lv_event_t *e)
{
    (void)e;
    if (g_scroll) lv_obj_scroll_to_y(g_scroll, 0, LV_ANIM_OFF);
}

static void show_notice_msgbox(const char *title, const char *text, uint32_t auto_close_ms)
{
    lv_obj_t *mbox = lv_msgbox_create(lv_scr_act());
    if (!mbox) return;

    /* 弹窗被关闭时重置滚动 */
    lv_obj_add_event_cb(mbox, msgbox_delete_cb, LV_EVENT_DELETE, NULL);

    lv_obj_set_width(mbox, 520);
    lv_obj_set_style_radius(mbox, 8, 0);
    lv_obj_set_style_border_width(mbox, 1, 0);
    lv_obj_set_style_border_color(mbox, C(CLR_BORDER), 0);
    lv_obj_set_style_shadow_width(mbox, 20, 0);
    lv_obj_set_style_shadow_color(mbox, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(mbox, LV_OPA_40, 0);
    lv_obj_set_style_shadow_offset_x(mbox, 0, 0);
    lv_obj_set_style_shadow_offset_y(mbox, 8, 0);
    set_bg(mbox, CLR_PANEL);

    lv_obj_t *title_lbl = lv_msgbox_add_title(mbox, title);
    if (title_lbl) {
        lv_obj_set_style_text_font(title_lbl, &lv_font_founder_kaiti_simplified_16, 0);
        lv_obj_set_style_text_color(title_lbl, C(CLR_TEXT), 0);
    }

    lv_obj_t *header = lv_msgbox_get_header(mbox);
    if (header) {
        set_bg(header, CLR_HEADER_BG);
        lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(header, 1, 0);
        lv_obj_set_style_border_color(header, C(CLR_BORDER), 0);
        lv_obj_set_style_pad_left(header, 14, 0);
        lv_obj_set_style_pad_right(header, 8, 0);
    }

    lv_obj_t *content = lv_msgbox_get_content(mbox);
    if (content) {
        set_bg(content, CLR_PANEL);
        lv_obj_set_style_pad_left(content, 14, 0);
        lv_obj_set_style_pad_right(content, 14, 0);
        lv_obj_set_style_pad_top(content, 10, 0);
        lv_obj_set_style_pad_bottom(content, 12, 0);
    }

    lv_obj_t *msg_lbl = lv_msgbox_add_text(mbox, text);
    if (msg_lbl) {
        lv_obj_set_style_text_font(msg_lbl, &lv_font_founder_kaiti_simplified_16, 0);
        lv_obj_set_style_text_color(msg_lbl, C(CLR_TEXT), 0);
        lv_obj_set_style_text_line_space(msg_lbl, 4, 0);
        lv_label_set_long_mode(msg_lbl, LV_LABEL_LONG_WRAP);
    }

    lv_obj_t *close_btn = lv_msgbox_add_close_button(mbox);
    if (close_btn) {
        lv_obj_set_size(close_btn, 28, 28);
        lv_obj_set_style_radius(close_btn, 4, 0);
        lv_obj_set_style_border_width(close_btn, 0, 0);
        lv_obj_set_style_bg_opa(close_btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(close_btn, C(CLR_BORDER), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(close_btn, LV_OPA_60, LV_STATE_PRESSED);
        lv_obj_t *close_icon = lv_obj_get_child(close_btn, 0);
        if (close_icon) {
            lv_obj_set_style_img_recolor(close_icon, C(CLR_TEXT_DIM), 0);
            lv_obj_set_style_img_recolor_opa(close_icon, LV_OPA_COVER, 0);
        }
    }

    lv_obj_center(mbox);
    if (auto_close_ms > 0U) {
        lv_obj_delete_delayed(mbox, auto_close_ms);
    }
}

/* ═══════════════════════════════════════════════
 * 时间格式校验
 * ═══════════════════════════════════════════════ */
static bool validate_date_str(const char *s)
{
    if (!s || !s[0]) return true;  /* 空串视为"不限"，合法 */
    int Y, M, D;
    if (sscanf(s, "%d.%d.%d", &Y, &M, &D) != 3) return false;
    if (Y < 2000 || Y > 2099) return false;
    if (M < 1 || M > 12) return false;
    if (D < 1 || D > 31) return false;
    return true;
}

static bool validate_time_str(const char *s)
{
    if (!s || !s[0]) return true;
    int h, m, sec;
    if (sscanf(s, "%d:%d:%d", &h, &m, &sec) != 3) return false;
    if (h < 0 || h > 23) return false;
    if (m < 0 || m > 59) return false;
    if (sec < 0 || sec > 59) return false;
    return true;
}

/* ═══════════════════════════════════════════════
 * 数据初始化
 * ═══════════════════════════════════════════════ */
void Init_Test_Log_Data_Batch(int count)
{
    if (count > MAX_LOG_ENTRIES) count = MAX_LOG_ENTRIES;
    if (log_datas != NULL) { free(log_datas); log_datas = NULL; }
    log_datas = (log_data_t *)malloc(sizeof(log_data_t) * count);
    if (!log_datas) return;
    total_log_count = count;

    const char *levels[]     = {"Info","Warning","Error","Debug","Fatal","Trace"};
    const char *categories[] = {"设备运行日志","测试流程日志","异常与报警日志","通信交互日志"};
    const char *types[]      = {"Sys Running","Temp Warning","CamOsc_Calres","Data Update"};
    const char *contents[]   = {
        "系统启动完成,所有模块初始化正常",
        "温度传感器超过阈值,当前温度85°C",
        "相机标定参数初始化完成,ID:2000",
        "数据同步完成,共传输1024条记录"
    };

    struct tm base_tm = {0};
    base_tm.tm_year = 2024 - 1900;
    base_tm.tm_mon  = 8 - 1;
    base_tm.tm_mday = 1;
    time_t base_time = mktime(&base_tm);

    for (int i = 0; i < count; i++) {
        /* 间隔改为 600 秒（10分钟），500 条跨约 3.5 天，方便测试时间筛选 */
        time_t t = base_time + (time_t)i * 600;
        log_datas[i].timestamp = t;
        struct tm *tm_s = localtime(&t);
        strftime(log_datas[i].date,       sizeof(log_datas[i].date),       "%Y.%m.%d", tm_s);
        strftime(log_datas[i].start_time, sizeof(log_datas[i].start_time), "%H:%M:%S", tm_s);
        time_t te = t + 5;
        struct tm *tm_e = localtime(&te);
        strftime(log_datas[i].end_time,   sizeof(log_datas[i].end_time),   "%H:%M:%S", tm_e);
        snprintf(log_datas[i].dev_name, sizeof(log_datas[i].dev_name), "Device_%02d", i%5+1);
        strcpy(log_datas[i].log_category,  categories[i % 4]);
        strcpy(log_datas[i].event_level,   levels[i % 6]);
        strcpy(log_datas[i].event_type,    types[i % 4]);
        strcpy(log_datas[i].event_content, contents[i % 4]);
    }
}

/* ═══════════════════════════════════════════════
 * 筛选逻辑
 * ═══════════════════════════════════════════════ */
static time_t parse_time(const char *s)
{
    if (!s || !*s) return 0;
    int Y,M,D,h,m,sec;
    if (sscanf(s, "%d.%d.%d %d:%d:%d", &Y,&M,&D,&h,&m,&sec) != 6) return 0;
    struct tm tm = {0};
    tm.tm_year = Y-1900; tm.tm_mon = M-1; tm.tm_mday = D;
    tm.tm_hour = h; tm.tm_min = m; tm.tm_sec = sec;
    tm.tm_isdst = -1;
    return mktime(&tm);
}


static bool matches(int idx)
{
    log_data_t *d = &log_datas[idx];

    if (g_time_filter) {
        char ts[64];
        snprintf(ts, sizeof(ts), "%s %s", d->date, d->start_time);
        time_t lt = parse_time(ts);
        time_t st = parse_time(g_start_str);
        time_t et = parse_time(g_end_str);
        if (st && lt < st) return false;
        if (et && lt > et) return false;
    }

    if (g_sel_cat > 0) {
        if (g_sel_cat < 5 && strstr(d->log_category, g_cat_names[g_sel_cat]) == NULL) return false;
    }

    if (g_sel_name > 0) {
        if (g_sel_name < 5 && strstr(d->event_type, g_event_names[g_sel_name]) == NULL) return false;
    }

    if (g_sel_level > 0) {
        if (g_sel_level < 7 && strcmp(d->event_level, g_level_names[g_sel_level]) != 0) return false;
    }

    return true;
}

static void rebuild_filter(void)
{
    g_filtered_count = 0;
    for (int i = 0; i < total_log_count && g_filtered_count < MAX_LOG_ENTRIES; i++) {
        if (matches(i)) g_filtered[g_filtered_count++] = i;
    }
    g_total_pages = (g_filtered_count + g_items_per_page - 1) / g_items_per_page;
    if (g_total_pages < 1) g_total_pages = 1;
    if (g_current_page >= g_total_pages) g_current_page = g_total_pages - 1;
}

static void do_fetch(void)
{
    if (g_fetch_cb != NULL) {
        log_query_t q;
        memset(&q, 0, sizeof(q));
        q.cat       = g_sel_cat;
        q.name      = g_sel_name;
        q.level     = g_sel_level;
        q.page      = g_current_page;
        q.page_size = g_items_per_page;
        strncpy(q.start_str, g_start_str, sizeof(q.start_str) - 1);
        strncpy(q.end_str,   g_end_str,   sizeof(q.end_str)   - 1);

        int count = 0, total = 0;
        if (g_fetch_cb(&q, g_page_buf, &count, &total)) {
            g_page_buf_count = count;
            g_query_total    = total;
            for (int i = 0; i < count; i++)
                g_page_global_idx[i] = q.page * q.page_size + i;
        }
    } else {
        rebuild_filter();
        g_query_total = g_filtered_count;

        int start = g_current_page * g_items_per_page;
        int end   = start + g_items_per_page;
        if (end > g_filtered_count) end = g_filtered_count;

        g_page_buf_count = end - start;
        for (int i = start; i < end; i++) {
            g_page_buf[i - start] = log_datas[g_filtered[i]];
            g_page_global_idx[i - start] = g_filtered[i];
        }
    }

    g_total_pages = (g_query_total + g_items_per_page - 1) / g_items_per_page;
    if (g_total_pages < 1) g_total_pages = 1;
    if (g_current_page >= g_total_pages) g_current_page = g_total_pages - 1;
}

/* ═══════════════════════════════════════════════
 * 清除所有勾选状态的辅助函数
 * ═══════════════════════════════════════════════ */
static void clear_all_selections(void)
{
    memset(log_selected, 0, sizeof(log_selected));
    g_all_selected = false;
    if (g_allcb_obj)  set_bg(g_allcb_obj, 0xFFFFFF);
    if (g_allcb_icon) lv_obj_add_flag(g_allcb_icon, LV_OBJ_FLAG_HIDDEN);
}

/* ═══════════════════════════════════════════════
 * 彩色分类标签
 * ═══════════════════════════════════════════════ */
static void make_category_tag(lv_obj_t *parent, const char *cat)
{
    uint32_t bg, fg;
    const char *txt = cat;

    if (strstr(cat, "异常") || strstr(cat, "报警")) {
        bg = CLR_FAULT_BG; fg = CLR_FAULT_FG;
    } else if (strstr(cat, "测试")) {
        bg = CLR_DEBUG_BG; fg = CLR_DEBUG_FG;
    } else if (strstr(cat, "通信")) {
        bg = CLR_COMM_BG;  fg = CLR_COMM_FG;
    } else {
        bg = CLR_RUN_BG;   fg = CLR_RUN_FG;
    }

    lv_obj_t *tag = clean_cont(parent);
    lv_obj_set_size(tag, COL_TYPE_W - 12, 24);
    set_bg(tag, bg);
    lv_obj_set_style_radius(tag, 4, 0);
    lv_obj_set_style_border_width(tag, 1, 0);
    lv_obj_set_style_border_color(tag, C(fg), 0);
    lv_obj_set_style_border_opa(tag, LV_OPA_40, 0);

    lv_obj_t *l = lv_label_create(tag);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, C(fg), 0);
    lv_obj_set_style_text_font(l, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_center(l);
}

/* ═══════════════════════════════════════════════
 * 单元格辅助
 * ═══════════════════════════════════════════════ */
static lv_obj_t *make_cell(lv_obj_t *row, int w)
{
    lv_obj_t *c = clean_cont(row);
    lv_obj_set_size(c, w, ROW_H);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return c;
}

/* ═══════════════════════════════════════════════
 * 勾选框回调
 * ═══════════════════════════════════════════════ */
typedef struct { 
    int data_idx; 
    lv_obj_t *check_icon; 
    lv_obj_t *checkbox;
} cb_ud_t;
static cb_ud_t g_cb_ud[64];

static void checkbox_cb(lv_event_t *e)
{
    cb_ud_t *ud = (cb_ud_t *)lv_event_get_user_data(e);
    if (!ud) return;

    log_selected[ud->data_idx] = !log_selected[ud->data_idx];

    if (log_selected[ud->data_idx]) 
    {
        set_bg(ud->checkbox, CLR_ACCENT);
        lv_obj_clear_flag(ud->check_icon, LV_OBJ_FLAG_HIDDEN);
    } 
    else 
    {
        set_bg(ud->checkbox, 0xFFFFFF);
        lv_obj_add_flag(ud->check_icon, LV_OBJ_FLAG_HIDDEN);

        /* 取消任意一行时，同步取消表头全选框 */
        if (g_all_selected) {
            g_all_selected = false;
            if (g_allcb_obj)  set_bg(g_allcb_obj, 0xFFFFFF);
            if (g_allcb_icon) lv_obj_add_flag(g_allcb_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* ═══════════════════════════════════════════════
 * 全选回调
 * ═══════════════════════════════════════════════ */
static void select_all_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    g_all_selected = !g_all_selected;

    for (int i = 0; i < g_filtered_count; i++)
        log_selected[g_filtered[i]] = g_all_selected;

    lv_obj_t *icon = lv_obj_get_child(btn, 0);
    if (g_all_selected) {
        set_bg(btn, CLR_ACCENT);
        if (icon) lv_obj_clear_flag(icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        set_bg(btn, 0xFFFFFF);
        if (icon) lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
    }
    populate_table();
}

/* ═══════════════════════════════════════════════
 * 填充表格内容
 * ═══════════════════════════════════════════════ */
static void populate_table(void)
{
    if (!g_scroll) return;
    lv_obj_clean(g_scroll);

    if (g_page_lbl) {
        int start_no = (g_query_total > 0) ? g_current_page * g_items_per_page + 1 : 0;
        int end_no   = g_current_page * g_items_per_page + g_page_buf_count;
        char buf[64];
        lv_snprintf(buf, sizeof(buf), "显示 %d - %d / 共 %d 条",start_no, end_no, g_query_total);
        lv_label_set_text(g_page_lbl, buf);
    }
    if (g_num_lbl) {
        char buf[16]; 
        lv_snprintf(buf, sizeof(buf), "%d", g_current_page + 1);
        lv_label_set_text(g_num_lbl, buf);
    }
    /*  更新总页数标签 */
    if (g_total_page_lbl) {
        char buf[16];
        lv_snprintf(buf, sizeof(buf), "/ %d", g_total_pages);
        lv_label_set_text(g_total_page_lbl, buf);
    }

    /* 筛选结果为空时显示提示 */
    if (g_page_buf_count == 0) {
        g_empty_lbl = make_label(g_scroll, "未找到匹配的日志记录",
                                 CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
        lv_obj_set_width(g_empty_lbl, LV_PCT(100));
        lv_obj_set_style_text_align(g_empty_lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_top(g_empty_lbl, 40, 0);
        return;
    }
    g_empty_lbl = NULL;

    for (int row_idx = 0; row_idx < g_page_buf_count; row_idx++)
    {
        log_data_t *d = &g_page_buf[row_idx];

        int global_idx = g_page_global_idx[row_idx];
        bool selected = log_selected[global_idx];

        uint32_t row_bg = (row_idx % 2 == 0) ? CLR_ROW_ODD : CLR_ROW_EVEN;
        lv_obj_t *row = lv_obj_create(g_scroll);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), ROW_H);
        set_bg(row, row_bg);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, C(CLR_BORDER), 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    

        /* ── 勾选框 ── */
        lv_obj_t *cb_cell = make_cell(row, COL_CB_W);
        lv_obj_t *checkbox = clean_cont(cb_cell);
        lv_obj_set_size(checkbox, 20, 20);
        set_bg(checkbox, selected ? CLR_ACCENT : 0xFFFFFF);
        lv_obj_set_style_radius(checkbox, 3, 0);
        lv_obj_set_style_border_width(checkbox, 2, 0);
        lv_obj_set_style_border_color(checkbox, C(CLR_ACCENT), 0);
        lv_obj_add_flag(checkbox, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *check_icon = lv_label_create(checkbox);
        lv_label_set_text(check_icon, LV_SYMBOL_OK);
        lv_obj_set_style_text_color(check_icon, C(0xFFFFFF), 0);
        lv_obj_set_style_text_font(check_icon, &lv_font_montserrat_16, 0);
        lv_obj_center(check_icon);
        if (!selected) lv_obj_add_flag(check_icon, LV_OBJ_FLAG_HIDDEN);

        if (row_idx < 64) 
        {
            g_cb_ud[row_idx].data_idx   = global_idx;
            g_cb_ud[row_idx].check_icon = check_icon;
            g_cb_ud[row_idx].checkbox   = checkbox;
            lv_obj_add_event_cb(checkbox, checkbox_cb, LV_EVENT_CLICKED, &g_cb_ud[row_idx]);
        }

        /* ── 序号 ── */
        lv_obj_t *c;
        c = make_cell(row, COL_NO_W);
        char no_buf[16];  /* 8→16 */
        lv_snprintf(no_buf, sizeof(no_buf), "%d",g_current_page * g_items_per_page + row_idx + 1);
        make_label(c, no_buf, CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);

        /* ── 时间 ── */
        c = make_cell(row, COL_TIME_W);
        char full_time[32];
        snprintf(full_time, sizeof(full_time), "%s %s", d->date, d->start_time);
        make_label(c, full_time, CLR_TEXT, &lv_font_founder_kaiti_simplified_16);

        /* ── 设备名称 ── */
        c = make_cell(row, COL_DEV_W);
        make_label(c, d->dev_name, CLR_TEXT, &lv_font_founder_kaiti_simplified_16);

        /* ── 日志分类 tag ── */
        c = make_cell(row, COL_TYPE_W);
        make_category_tag(c, d->log_category);

        /* ── 事件信息 ── */
        c = make_cell(row, COL_INFO_W);
        make_label(c, d->event_type, CLR_TEXT, &lv_font_founder_kaiti_simplified_16);

        /* ── 事件内容 ── */
        lv_obj_t *content_cell = clean_cont(row);
        lv_obj_set_flex_grow(content_cell, 1);
        lv_obj_set_height(content_cell, ROW_H);
        lv_obj_set_flex_flow(content_cell, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(content_cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_t *content_lbl = lv_label_create(content_cell);
        lv_label_set_text(content_lbl, d->event_content);
        lv_label_set_long_mode(content_lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_color(content_lbl, C(CLR_TEXT), 0);
        lv_obj_set_style_text_font(content_lbl, &lv_font_founder_kaiti_simplified_16, 0);

        /* ── 操作按钮 ── */
        c = make_cell(row, COL_ACT_W);
        bool is_warn = (strstr(d->log_category, "异常") || strstr(d->log_category, "报警"));
        make_btn(c, is_warn ? "警告" : "正常",
                 is_warn ? CLR_BTN_WARN : CLR_BTN_COPY, 0xFFFFFF, 56, 26);
    }
}

/* ═══════════════════════════════════════════════
 * 分页回调
 * ═══════════════════════════════════════════════ */
static void prev_cb(lv_event_t *e)
{
    (void)e;
    if (g_current_page > 0) { g_current_page--; do_fetch(); populate_table(); }
}

static void next_cb(lv_event_t *e)
{
    (void)e;
    if (g_current_page < g_total_pages - 1) { g_current_page++; do_fetch(); populate_table(); }
}

static void go_cb(lv_event_t *e)
{
    lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
    if (!ta) return;
    int pg = atoi(lv_textarea_get_text(ta));
    if (pg >= 1 && pg <= g_total_pages) {
        g_current_page = pg - 1;
        do_fetch();
        populate_table();
    }
    lv_textarea_set_text(ta, "");
}

/* ═══════════════════════════════════════════════
 * 时间重置回调 — 清空时间输入框并刷新
 * ═══════════════════════════════════════════════ */
static void time_reset_cb(lv_event_t *e)
{
    (void)e;
    if (g_start_date_ta) lv_textarea_set_text(g_start_date_ta, "");
    if (g_start_time_ta) lv_textarea_set_text(g_start_time_ta, "");
    if (g_end_date_ta)   lv_textarea_set_text(g_end_date_ta,   "");
    if (g_end_time_ta)   lv_textarea_set_text(g_end_time_ta,   "");
    g_time_filter = false;
    memset(g_start_str, 0, sizeof(g_start_str));
    memset(g_end_str,   0, sizeof(g_end_str));
    g_current_page = 0;
    do_fetch();
    populate_table();
}

/* ═══════════════════════════════════════════════
 *  搜索回调 — 增加格式校验
 * ═══════════════════════════════════════════════ */
static void search_cb(lv_event_t *e)
{
    (void)e;

    const char *sd = g_start_date_ta ? lv_textarea_get_text(g_start_date_ta) : "";
    const char *st = g_start_time_ta ? lv_textarea_get_text(g_start_time_ta) : "";
    const char *ed = g_end_date_ta   ? lv_textarea_get_text(g_end_date_ta)   : "";
    const char *et = g_end_time_ta   ? lv_textarea_get_text(g_end_time_ta)   : "";

    /* 格式校验 */
    if (!validate_date_str(sd) || !validate_date_str(ed)) {
        show_notice_msgbox("格式错误", "日期格式应为 YYYY.MM.DD\n例如: 2024.08.01", 3000);
        return;
    }
    if (!validate_time_str(st) || !validate_time_str(et)) {
        show_notice_msgbox("格式错误", "时间格式应为 HH:MM:SS\n例如: 08:30:00", 3000);
        return;
    }

    if (sd[0] && st[0])
        snprintf(g_start_str, sizeof(g_start_str), "%s %s", sd, st);
    else if (sd[0])
        snprintf(g_start_str, sizeof(g_start_str), "%s 00:00:00", sd);
    else
        g_start_str[0] = '\0';

    if (ed[0] && et[0])
        snprintf(g_end_str, sizeof(g_end_str), "%s %s", ed, et);
    else if (ed[0])
        snprintf(g_end_str, sizeof(g_end_str), "%s 23:59:59", ed);
    else
        g_end_str[0] = '\0';

    g_time_filter = (g_start_str[0] || g_end_str[0]);
    g_current_page = 0;
    do_fetch();      
    populate_table();
}

static void dropdown_cb(lv_event_t *e)
{
    lv_obj_t *dd = lv_event_get_target(e);
    int *target = (int *)lv_event_get_user_data(e);
    if (target) *target = (int)lv_dropdown_get_selected(dd);
    g_current_page = 0;
    do_fetch();     
    populate_table();
}

static void perpage_cb(lv_event_t *e)
{
    if (!g_dd_perpage) return;
    static const int sizes[] = {10, 14, 20, 25, 30};
    int sel = (int)lv_dropdown_get_selected(g_dd_perpage);
    if (sel >= 0 && sel < 5) g_items_per_page = sizes[sel];
    g_current_page = 0;
    do_fetch();      
    populate_table();
}

/* ═══════════════════════════════════════════════
 *  删除确认弹窗 + 实际删除逻辑
 * ═══════════════════════════════════════════════ */
static void do_delete_confirmed(void)
{
    for (int i = total_log_count - 1; i >= 0; i--) 
    {
        if (log_selected[i]) {
            memmove(&log_datas[i], &log_datas[i+1],
                    sizeof(log_data_t) * (total_log_count - i - 1));
            memmove(&log_selected[i], &log_selected[i+1],
                    sizeof(bool) * (total_log_count - i - 1));
            total_log_count--;
        }
    }
    clear_all_selections();
    g_current_page = 0;
    do_fetch();  
    populate_table();
}

static void delete_confirm_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *mbox = (lv_obj_t *)lv_event_get_user_data(e);  /* 从 user_data 拿弹窗指针 */

    /* 判断点击的是哪个按钮 */
    const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));
    if (txt && strstr(txt, "确定")) {
        do_delete_confirmed();
    }
    /* 关闭弹窗（DELETE 事件会自动重置滚动位置）*/
    if (mbox) lv_msgbox_close(mbox);
}

static void delete_cb(lv_event_t *e)
{
    (void)e;

    /* 先统计选中条数 */
    int sel_count = 0;
    for (int i = 0; i < g_filtered_count; i++) {
        if (log_selected[g_filtered[i]]) sel_count++;
    }

    if (sel_count == 0) {
        show_notice_msgbox("提示", "请先勾选要删除的日志条目", 2200);
        return;
    }

    /* 弹出确认弹窗 */
    lv_obj_t *mbox = lv_msgbox_create(lv_scr_act());
    if (!mbox) return;

    /* 弹窗被关闭时重置滚动 */
    lv_obj_add_event_cb(mbox, msgbox_delete_cb, LV_EVENT_DELETE, NULL);

    lv_obj_set_width(mbox, 420);
    lv_obj_set_style_radius(mbox, 8, 0);
    lv_obj_set_style_border_width(mbox, 1, 0);
    lv_obj_set_style_border_color(mbox, C(CLR_BORDER), 0);
    lv_obj_set_style_shadow_width(mbox, 20, 0);
    lv_obj_set_style_shadow_color(mbox, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(mbox, LV_OPA_40, 0);
    set_bg(mbox, CLR_PANEL);

    lv_obj_t *title_lbl = lv_msgbox_add_title(mbox, "确认删除");
    if (title_lbl) {
        lv_obj_set_style_text_font(title_lbl, &lv_font_founder_kaiti_simplified_16, 0);
        lv_obj_set_style_text_color(title_lbl, C(CLR_TEXT), 0);
    }

    lv_obj_t *header = lv_msgbox_get_header(mbox);
    if (header) {
        set_bg(header, CLR_HEADER_BG);
        lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(header, 1, 0);
        lv_obj_set_style_border_color(header, C(CLR_BORDER), 0);
    }

    lv_obj_t *content = lv_msgbox_get_content(mbox);
    if (content) {
        set_bg(content, CLR_PANEL);
        lv_obj_set_style_pad_all(content, 14, 0);
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "确定要删除选中的 %d 条日志吗?\n此操作不可撤销!", sel_count);
    lv_obj_t *msg_lbl = lv_msgbox_add_text(mbox, msg);
    if (msg_lbl) {
        lv_obj_set_style_text_font(msg_lbl, &lv_font_founder_kaiti_simplified_16, 0);
        lv_obj_set_style_text_color(msg_lbl, C(CLR_TEXT), 0);
    }

    lv_obj_t *btn_ok = lv_msgbox_add_footer_button(mbox, "确定删除");
    if (btn_ok) {
        lv_obj_set_size(btn_ok, 100, 32);
        set_bg(btn_ok, CLR_BTN_WARN);
        lv_obj_set_style_text_color(btn_ok, C(0xFFFFFF), 0);
        lv_obj_set_style_text_font(btn_ok, &lv_font_founder_kaiti_simplified_16, 0);
        lv_obj_add_event_cb(btn_ok, delete_confirm_cb, LV_EVENT_CLICKED, mbox);
    }

    lv_obj_t *btn_cancel = lv_msgbox_add_footer_button(mbox, "取消");
    if (btn_cancel) {
        lv_obj_set_size(btn_cancel, 80, 32);
        set_bg(btn_cancel, CLR_BORDER);
        lv_obj_set_style_text_color(btn_cancel, C(CLR_TEXT), 0);
        lv_obj_set_style_text_font(btn_cancel, &lv_font_founder_kaiti_simplified_16, 0);
        lv_obj_add_event_cb(btn_cancel, delete_confirm_cb, LV_EVENT_CLICKED, mbox);
    }

    lv_obj_t *footer = lv_msgbox_get_footer(mbox);
    if (footer) {
        set_bg(footer, CLR_PANEL);
        lv_obj_set_style_pad_all(footer, 10, 0);
        lv_obj_set_style_pad_gap(footer, 12, 0);
    }

    lv_obj_t *close_btn = lv_msgbox_add_close_button(mbox);
    if (close_btn) {
        lv_obj_set_size(close_btn, 28, 28);
        lv_obj_set_style_bg_opa(close_btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    }

    lv_obj_center(mbox);
}

static void reset_filter_cb(lv_event_t *e)
{
    (void)e;
    g_sel_cat = g_sel_name = g_sel_level = 0;
    g_time_filter = false;
    memset(g_start_str, 0, sizeof(g_start_str));
    memset(g_end_str,   0, sizeof(g_end_str));
    if (g_start_date_ta) lv_textarea_set_text(g_start_date_ta, "");
    if (g_start_time_ta) lv_textarea_set_text(g_start_time_ta, "");
    if (g_end_date_ta)   lv_textarea_set_text(g_end_date_ta,   "");
    if (g_end_time_ta)   lv_textarea_set_text(g_end_time_ta,   "");
    if (g_dd_cat)   lv_dropdown_set_selected(g_dd_cat,   0);
    if (g_dd_name)  lv_dropdown_set_selected(g_dd_name,  0);
    if (g_dd_level) lv_dropdown_set_selected(g_dd_level, 0);
    g_current_page = 0;
    do_fetch();     
    populate_table();
}

/* ═══════════════════════════════════════════════
 * 导出回调 — 导出成功后清除勾选
 * ═══════════════════════════════════════════════ */
static void export_cb_all(lv_event_t *e)
{
    (void)e;

    int sel_count = 0;
    for (int i = 0; i < g_filtered_count; i++) {
        if (log_selected[g_filtered[i]]) sel_count++;
    }

    if (sel_count == 0) {
        show_notice_msgbox("提示", "请先勾选要导出的日志条目", 2200);
        return;
    }
    
    log_data_t *sel_buf = (log_data_t *)malloc(sizeof(log_data_t) * sel_count);
    if (!sel_buf) return;

    int idx = 0;
    for (int i = 0; i < g_filtered_count; i++) {
        int gi = g_filtered[i];
        if (log_selected[gi]) {
            sel_buf[idx++] = log_datas[gi];
        }
    }

    time_t now = time(NULL);
    struct tm *tm_s = localtime(&now);
    char filename[64];
    strftime(filename, sizeof(filename), "log_%Y%m%d_%H%M%S", tm_s);

    bool ok = Log_Export_To_Word(sel_buf, idx, filename);
    free(sel_buf);

    char msg[256];
    if (ok) {
        snprintf(msg, sizeof(msg),"导出成功!共 %d 条\n""保存至程序目录下的 log 文件夹\n%s.rtf",idx, filename);
        /*  导出成功后清除所有勾选 */
        clear_all_selections();
        populate_table();
    } else {
        snprintf(msg, sizeof(msg),"导出失败\n请确认目录存在:\n" "D:\\smp\\log\\");
    }

    show_notice_msgbox(ok ? "导出结果" : "导出失败", msg, 5000);
}

/* ═══════════════════════════════════════════════
 * 表头行
 * ═══════════════════════════════════════════════ */
static void create_table_header(lv_obj_t *parent)
{
    lv_obj_t *hdr = clean_cont(parent);
    lv_obj_set_size(hdr, LV_PCT(100), ROW_H);
    set_bg(hdr, CLR_HEADER_BG);
    lv_obj_set_style_border_side(hdr, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(hdr, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(hdr, 2, 0);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);  /* 禁用表头拖动 */
    lv_obj_t *cb_cell = make_cell(hdr, COL_CB_W);
    lv_obj_t *all_cb = clean_cont(cb_cell);
    lv_obj_set_size(all_cb, 20, 20);
    set_bg(all_cb, g_all_selected ? CLR_ACCENT : 0xFFFFFF);
    lv_obj_set_style_radius(all_cb, 3, 0);
    lv_obj_set_style_border_width(all_cb, 2, 0);
    lv_obj_set_style_border_color(all_cb, C(CLR_ACCENT), 0);
    lv_obj_add_flag(all_cb, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *all_icon = lv_label_create(all_cb);
    lv_label_set_text(all_icon, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(all_icon, C(0xFFFFFF), 0);
    lv_obj_set_style_text_font(all_icon, &lv_font_founder_kaiti_simplified_12, 0);
    lv_obj_center(all_icon);
    if (!g_all_selected) lv_obj_add_flag(all_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(all_cb, select_all_cb, LV_EVENT_CLICKED, NULL);
    g_allcb_obj  = all_cb;
    g_allcb_icon = all_icon;

    static const char * const col_txts[] = {
        "序号", "时间", "设备名称", "日志分类", "事件信息", "操作"
    };
    static const int col_ws[] = {
        COL_NO_W, COL_TIME_W, COL_DEV_W, COL_TYPE_W, COL_INFO_W, COL_ACT_W
    };

    int i;
    for (i = 0; i < 5; i++) {
        lv_obj_t *c = make_cell(hdr, col_ws[i]);
        make_label(c, col_txts[i], CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
    }

    lv_obj_t *content_hdr = clean_cont(hdr);
    lv_obj_set_flex_grow(content_hdr, 1);
    lv_obj_set_height(content_hdr, ROW_H);
    lv_obj_set_flex_flow(content_hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(content_hdr, LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    make_label(content_hdr, "事件内容", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);

    lv_obj_t *act_hdr = make_cell(hdr, col_ws[5]);
    make_label(act_hdr, col_txts[5], CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
}

/* ═══════════════════════════════════════════════
 * 顶部工具栏
 * ═══════════════════════════════════════════════ */
static void create_topbar(lv_obj_t *parent)
{
    lv_obj_t *tb = clean_cont(parent);
    lv_obj_set_size(tb, LV_PCT(100), TOPBAR_H);
    set_bg(tb, CLR_PANEL);
    lv_obj_set_style_border_side(tb, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(tb, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(tb, 1, 0);
    lv_obj_set_flex_flow(tb, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tb, LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(tb, 12, 0);
    lv_obj_set_style_pad_right(tb, 12, 0);
    lv_obj_set_style_pad_gap(tb, 8, 0);

    make_label(tb, "时间:", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);

    /* 限制输入字符 +  动态 placeholder */
    char ph_start_date[16] = "YYYY.MM.DD";
    char ph_end_date[16]   = "YYYY.MM.DD";
    if (log_datas && total_log_count > 0) {
        snprintf(ph_start_date, sizeof(ph_start_date), "%s", log_datas[0].date);
        snprintf(ph_end_date,   sizeof(ph_end_date),   "%s", log_datas[total_log_count - 1].date);
    }

    g_start_date_ta = lv_textarea_create(tb);
    lv_textarea_set_one_line(g_start_date_ta, true);
    lv_textarea_set_placeholder_text(g_start_date_ta, ph_start_date);
    lv_textarea_set_accepted_chars(g_start_date_ta, "0123456789.");  
    lv_obj_set_size(g_start_date_ta, 108, 30);
    lv_obj_set_style_bg_color(g_start_date_ta, C(CLR_BG), 0);
    lv_obj_set_style_bg_opa(g_start_date_ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(g_start_date_ta, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(g_start_date_ta, 1, 0);
    lv_obj_set_style_text_color(g_start_date_ta, C(CLR_TEXT), 0);
    lv_obj_set_style_text_font(g_start_date_ta, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_set_style_radius(g_start_date_ta, 4, 0);

    g_start_time_ta = lv_textarea_create(tb);
    lv_textarea_set_one_line(g_start_time_ta, true);
    lv_textarea_set_placeholder_text(g_start_time_ta, "00:00:00");
    lv_textarea_set_accepted_chars(g_start_time_ta, "0123456789:");  
    lv_obj_set_size(g_start_time_ta, 90, 30);
    lv_obj_set_style_bg_color(g_start_time_ta, C(CLR_BG), 0);
    lv_obj_set_style_bg_opa(g_start_time_ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(g_start_time_ta, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(g_start_time_ta, 1, 0);
    lv_obj_set_style_text_color(g_start_time_ta, C(CLR_TEXT), 0);
    lv_obj_set_style_text_font(g_start_time_ta, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_set_style_radius(g_start_time_ta, 4, 0);

    make_label(tb, "至", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);

    g_end_date_ta = lv_textarea_create(tb);
    lv_textarea_set_one_line(g_end_date_ta, true);
    lv_textarea_set_placeholder_text(g_end_date_ta, ph_end_date);
    lv_textarea_set_accepted_chars(g_end_date_ta, "0123456789."); 
    lv_obj_set_size(g_end_date_ta, 108, 30);
    lv_obj_set_style_bg_color(g_end_date_ta, C(CLR_BG), 0);
    lv_obj_set_style_bg_opa(g_end_date_ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(g_end_date_ta, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(g_end_date_ta, 1, 0);
    lv_obj_set_style_text_color(g_end_date_ta, C(CLR_TEXT), 0);
    lv_obj_set_style_text_font(g_end_date_ta, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_set_style_radius(g_end_date_ta, 4, 0);

    g_end_time_ta = lv_textarea_create(tb);
    lv_textarea_set_one_line(g_end_time_ta, true);
    lv_textarea_set_placeholder_text(g_end_time_ta, "23:59:59");
    lv_textarea_set_accepted_chars(g_end_time_ta, "0123456789:");  
    lv_obj_set_size(g_end_time_ta, 90, 30);
    lv_obj_set_style_bg_color(g_end_time_ta, C(CLR_BG), 0);
    lv_obj_set_style_bg_opa(g_end_time_ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(g_end_time_ta, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(g_end_time_ta, 1, 0);
    lv_obj_set_style_text_color(g_end_time_ta, C(CLR_TEXT), 0);
    lv_obj_set_style_text_font(g_end_time_ta, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_set_style_radius(g_end_time_ta, 4, 0);

    lv_obj_t *sb = make_btn(tb, "筛选" ,CLR_ACCENT,CLR_TEXT_WHITE,72,32);
    lv_obj_add_event_cb(sb, search_cb, LV_EVENT_CLICKED, NULL); 

    lv_obj_t *rst_btn = make_btn(tb, "重置", CLR_BORDER, CLR_TEXT_DIM, 60, 32);
    lv_obj_add_event_cb(rst_btn, time_reset_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *sp = clean_cont(tb);
    lv_obj_set_flex_grow(sp, 1);
    lv_obj_set_height(sp, 1);

    lv_obj_t *db = lv_btn_create(tb);
    lv_obj_remove_style_all(db);
    lv_obj_set_size(db, 110, 32);
    set_bg(db, 0xDC2626);
    lv_obj_set_style_radius(db, 4, 0);
    lv_obj_set_flex_flow(db, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(db, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(db, 4, 0);
    lv_obj_t *di = lv_label_create(db);
    lv_label_set_text(di, LV_SYMBOL_TRASH);
    lv_obj_set_style_text_color(di, C(0xFFFFFF), 0);
    lv_obj_set_style_text_font(di, &lv_font_founder_kaiti_simplified_16, 0);
    make_label(db, "删除日志", 0xFFFFFF, &lv_font_founder_kaiti_simplified_16);
    lv_obj_add_event_cb(db, delete_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *eb = lv_btn_create(tb);
    lv_obj_remove_style_all(eb);
    lv_obj_set_size(eb, 96, 32);
    set_bg(eb, 0x3B82F6);
    lv_obj_set_style_radius(eb, 4, 0);
    lv_obj_set_flex_flow(eb, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(eb, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(eb, 4, 0);
    lv_obj_t *ei = lv_label_create(eb);
    lv_label_set_text(ei, LV_SYMBOL_UPLOAD);
    lv_obj_set_style_text_color(ei, C(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ei,&lv_font_founder_kaiti_simplified_16, 0);
    make_label(eb, "导出日志", 0xFFFFFF, &lv_font_founder_kaiti_simplified_16);
    lv_obj_add_event_cb(eb, export_cb_all, LV_EVENT_CLICKED, NULL);
}

/* ═══════════════════════════════════════════════
 * 筛选条件栏
 * ═══════════════════════════════════════════════ */
static lv_obj_t *make_dd(lv_obj_t *parent, const char *opts, int w)
{
    lv_obj_t *dd = lv_dropdown_create(parent);
    lv_dropdown_set_options(dd, opts);
    lv_obj_set_size(dd, w, 28);
    set_bg(dd, CLR_BG);
    lv_obj_set_style_border_color(dd, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(dd, 1, 0);
    lv_obj_set_style_text_color(dd, C(CLR_TEXT), 0);
    lv_obj_set_style_text_font(dd, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_set_style_radius(dd, 4, 0);
    lv_obj_set_style_pad_all(dd, 4, 0);
    lv_obj_t *lst = lv_dropdown_get_list(dd);
    set_bg(lst, CLR_PANEL);
    lv_obj_set_style_border_color(lst, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(lst, 1, 0);
    lv_obj_set_style_text_color(lst, C(CLR_TEXT), 0);
    lv_obj_set_style_text_font(lst, &lv_font_founder_kaiti_simplified_16, 0);
    return dd;
}

static void create_filterbar(lv_obj_t *parent)
{
    lv_obj_t *fb = clean_cont(parent);
    lv_obj_set_size(fb, LV_PCT(100), FILTERBAR_H);
    set_bg(fb, CLR_PANEL);
    lv_obj_set_style_border_side(fb, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(fb, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(fb, 1, 0);
    lv_obj_set_flex_flow(fb, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fb, LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(fb, 12, 0);
    lv_obj_set_style_pad_gap(fb, 8, 0);

    make_label(fb, "分类:", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
    g_dd_cat = make_dd(fb, "全部\n测试流程日志\n设备运行日志\n异常与报警日志\n通信交互日志", 140);
    lv_obj_add_event_cb(g_dd_cat, dropdown_cb, LV_EVENT_VALUE_CHANGED, &g_sel_cat);

    g_dd_name = make_dd(fb, "全部\nCamOsc_Calres\nSys Running\nTemp Warning\nData Update", 140);
    lv_obj_add_event_cb(g_dd_name, dropdown_cb, LV_EVENT_VALUE_CHANGED, &g_sel_name);

    g_dd_level = make_dd(fb, "全部\nFatal\nError\nWarning\nInfo\nDebug\nTrace", 100);
    lv_obj_add_event_cb(g_dd_level, dropdown_cb, LV_EVENT_VALUE_CHANGED, &g_sel_level);

    lv_obj_t *sp = clean_cont(fb);
    lv_obj_set_flex_grow(sp, 1);
    lv_obj_set_height(sp, 1);

    lv_obj_t *rb = make_btn(fb, "恢复筛选",CLR_BORDER,CLR_TEXT_DIM,80, 28);
    lv_obj_add_event_cb(rb, reset_filter_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_font(rb, &lv_font_founder_kaiti_simplified_16, 0);
}

/* ═══════════════════════════════════════════════
 * 分页栏  增加总页数显示
 * ═══════════════════════════════════════════════ */
static void create_pagination(lv_obj_t *parent)
{
    lv_obj_t *pg = clean_cont(parent);
    lv_obj_set_size(pg, LV_PCT(100), PAGER_H);
    set_bg(pg, CLR_PANEL);
    lv_obj_set_style_border_side(pg, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(pg, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(pg, 1, 0);
    lv_obj_set_flex_flow(pg, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pg, LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(pg, 16, 0);
    lv_obj_set_style_pad_right(pg, 16, 0);
    lv_obj_set_style_pad_gap(pg, 8, 0);

    g_page_lbl = make_label(pg, "显示 0 - 0 / 共 0 条",CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);

    lv_obj_t *sp = clean_cont(pg);
    lv_obj_set_flex_grow(sp, 1);
    lv_obj_set_height(sp, 1);

    lv_obj_t *prev = lv_btn_create(pg);
    lv_obj_remove_style_all(prev);
    lv_obj_set_size(prev, 28, 28);
    set_bg(prev, CLR_BG);
    lv_obj_set_style_border_color(prev, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(prev, 1, 0);
    lv_obj_set_style_radius(prev, 3, 0);
    lv_obj_t *pl = make_label(prev, "<", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
    lv_obj_center(pl);
    lv_obj_add_event_cb(prev, prev_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *pbox = clean_cont(pg);
    lv_obj_set_size(pbox, 28, 28);
    set_bg(pbox, CLR_ACCENT);
    lv_obj_set_style_radius(pbox, 3, 0);
    g_num_lbl = lv_label_create(pbox);
    lv_label_set_text(g_num_lbl, "1");
    lv_obj_set_style_text_color(g_num_lbl, C(0xFFFFFF), 0);
    lv_obj_set_style_text_font(g_num_lbl, &lv_font_founder_kaiti_simplified_16, 0);
    lv_obj_center(g_num_lbl);

    /* 总页数标签 */
    g_total_page_lbl = make_label(pg, "/ 1", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);

    lv_obj_t *nxt = lv_btn_create(pg);
    lv_obj_remove_style_all(nxt);
    lv_obj_set_size(nxt, 28, 28);
    set_bg(nxt, CLR_BG);
    lv_obj_set_style_border_color(nxt, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(nxt, 1, 0);
    lv_obj_set_style_radius(nxt, 3, 0);
    lv_obj_t *nl = make_label(nxt, ">", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
    lv_obj_center(nl);
    lv_obj_add_event_cb(nxt, next_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *go_ta = lv_textarea_create(pg);
    lv_textarea_set_one_line(go_ta, true);
    lv_textarea_set_accepted_chars(go_ta, "0123456789");
    lv_textarea_set_placeholder_text(go_ta, "页码");
    lv_obj_set_size(go_ta, 60, 28);
    lv_obj_set_style_bg_color(go_ta, C(CLR_BG), 0);
    lv_obj_set_style_bg_opa(go_ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(go_ta, C(CLR_BORDER), 0);
    lv_obj_set_style_border_width(go_ta, 1, 0);
    lv_obj_set_style_text_color(go_ta, C(CLR_TEXT), 0);
    lv_obj_set_style_radius(go_ta, 4, 0);
    lv_obj_set_style_text_font(go_ta, &lv_font_founder_kaiti_simplified_16, 0);

    lv_obj_t *go_btn = lv_btn_create(pg);
    lv_obj_remove_style_all(go_btn);
    lv_obj_set_size(go_btn, 40, 28);
    set_bg(go_btn, 0x22C55E);
    lv_obj_set_style_radius(go_btn, 3, 0);
    lv_obj_t *go_l = make_label(go_btn, "Go", 0xFFFFFF, &lv_font_founder_kaiti_simplified_16);
    lv_obj_center(go_l);
    lv_obj_add_event_cb(go_btn, go_cb, LV_EVENT_CLICKED, go_ta);

    make_label(pg, "每页", CLR_TEXT_DIM, &lv_font_founder_kaiti_simplified_16);
    g_dd_perpage = make_dd(pg, "10\n14\n20\n25\n30", 65);
    lv_dropdown_set_selected(g_dd_perpage, 1);
    lv_obj_add_event_cb(g_dd_perpage, perpage_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

/* ═══════════════════════════════════════════════
 * 主入口
 * ═══════════════════════════════════════════════ */
void Log_Managment_Weight(smp_ctx_t *ctx)
{
    Init_Test_Log_Data_Batch(500);
    lv_obj_t *menu = NULL;
    lv_obj_t *activity_content = NULL;
    g_ctx = ctx;

    if (ctx && ctx->pages) {
        smp_page_t *page_obj = ctx->pages[NAV_PAGE_LOG];
        if (page_obj) {
            menu             = page_obj->menu;
            activity_content = page_obj->activity_content;
        }
    }
    if (!menu || !activity_content) return;
    if (menu != NULL) {
        lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN); 
    }

    lv_obj_set_style_pad_all(activity_content, 0, 0);

    /* 重置状态 */
    g_current_page = 0;
    g_all_selected = false;
    memset(log_selected, 0, sizeof(log_selected));
    g_sel_cat = g_sel_name = g_sel_level = 0;
    g_time_filter = false;
    memset(g_start_str, 0, sizeof(g_start_str));
    memset(g_end_str,   0, sizeof(g_end_str));

    lv_obj_t *content = clean_cont(activity_content);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    set_bg(content, CLR_BG);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    create_topbar(content);
    create_filterbar(content);
    create_table_header(content);
    
    lv_obj_t *scroll = lv_obj_create(content);
    lv_obj_remove_style_all(scroll);
    lv_obj_set_flex_grow(scroll, 1);
    lv_obj_set_width(scroll, LV_PCT(100));
    set_bg(scroll, CLR_BG);
    lv_obj_set_scroll_dir(scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(scroll, C(CLR_BORDER), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(scroll, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_style_width(scroll, 5, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll, LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    g_scroll = scroll;
    
    create_pagination(content);

    do_fetch();
    populate_table();
}

/* ═══════════════════════════════════════════════
 * 清理函数
 * ═══════════════════════════════════════════════ */
void Log_Managment_Cleanup(void)
{
    if (log_datas) { free(log_datas); log_datas = NULL; }
    total_log_count = 0;
    g_scroll     = NULL;
    g_page_lbl   = NULL;
    g_num_lbl    = NULL;
    g_total_page_lbl = NULL;
    g_allcb_obj  = NULL;
    g_allcb_icon = NULL;
    g_empty_lbl  = NULL;
    g_dd_cat     = NULL;
    g_dd_name    = NULL;
    g_dd_level   = NULL;
    g_dd_perpage = NULL;
    g_start_date_ta = NULL;
    g_start_time_ta = NULL;
    g_end_date_ta   = NULL;
    g_end_time_ta   = NULL;
    memset(log_selected, 0, sizeof(log_selected));
    g_all_selected = false;
}
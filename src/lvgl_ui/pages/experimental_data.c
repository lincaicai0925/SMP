#include "lvgl.h"
#include "pages/include/experimental_data.h"
#include "pages/include/data_dictionary.h"      
#include "pages/include/docx_export.h"
#include "pages/include/dashboard.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ================================================================== */
/*  宏定义 & 常量                                                       */
/* ================================================================== */

#define EXPORT_BTN_COUNT    5
#define VAR_MAP_COUNT       20
#define CHART_COUNT         4
#define TOOLBAR_HEIGHT      50
#define BTN_WIDTH           100
#define BTN_HEIGHT          36
#define BTN_GAP             12

/*
 * data_dictionary.h 中已有的裸整数颜色宏:
 *   CLR_PANEL      0x252830
 *   CLR_HEADER_BG  0x252830
 *   CLR_BORDER_PG  0x3A3F4B
 *   CLR_TEXT_DIM   0x6B7280
 *   CLR_ROW_ODD    0x252830
 *   CLR_ROW_EVEN   0x1A1D23
 *
 * 本文件需要额外定义的:
 */
#define CLR_EXP_TEXT        0xC8CDD8    /* 文字色 (data_dictionary.h 中无 CLR_TEXT) */
#define CLR_EXP_ROW_ALT    0x2A2D35    /* 交替行色 */

/* 按钮专属颜色 */
#define CLR_BTN_WORD        0x2B5797
#define CLR_BTN_PDF         0xC0392B
#define CLR_BTN_EXCEL       0x217346
#define CLR_BTN_XML         0xE67E22
#define CLR_BTN_ALL         0x8E44AD

/* ================================================================== */
/*  数据映射表                                                          */
/* ================================================================== */

typedef struct {
    const char *bookmark;
    uint32_t    bus_addr;
} var_mapping_t;

static const var_mapping_t VAR_MAP[VAR_MAP_COUNT] = {
    { "Var01",  0 }, { "Var02",  4 }, { "Var03",  8 }, { "Var04", 12 },
    { "Var05", 16 }, { "Var06", 20 }, { "Var07", 24 }, { "Var08", 28 },
    { "Var09", 32 }, { "Var10", 36 }, { "Var11", 40 }, { "Var12", 44 },
    { "Var13", 48 }, { "Var14", 52 }, { "Var15", 56 }, { "Var16", 60 },
    { "Var17", 64 }, { "Var18", 68 }, { "Var19", 72 }, { "Var20", 76 },
};

static const char *VAR_DESCS[VAR_MAP_COUNT] = {
    "第一路直流负载功率值",
    "第二路直流负载功率值",
    "第三路交流负载A相电阻有功功率",
    "第三路交流负载A相电感无功功率",
    "第三路交流负载B相电阻有功功率",
    "第三路交流负载B相电感无功功率",
    "第三路交流负载C相电阻有功功率",
    "第三路交流负载C相电感无功功率",
    "第四路交流负载A相电阻有功功率",
    "第四路交流负载A相电感无功功率",
    "第四路交流负载B相电阻有功功率",
    "第四路交流负载B相电感无功功率",
    "第四路交流负载C相电阻有功功率",
    "第四路交流负载C相电感无功功率",
    "第五路交流负载A相电阻有功功率",
    "第五路交流负载A相电感无功功率",
    "第五路交流负载B相电阻有功功率",
    "第五路交流负载B相电感无功功率",
    "第五路交流负载C相电阻有功功率",
    "第五路交流负载C相电感无功功率",
};

static const char *VAR_UNITS[VAR_MAP_COUNT] = {
    "W", "W", "W", "Var", "W", "Var", "W", "Var",
    "W", "Var", "W", "Var", "W", "Var",
    "W", "Var", "W", "Var", "W", "Var",
};

static const char *CHART_PATHS[CHART_COUNT] = {
    "word/media/chart1.png",
    "word/media/chart2.png",
    "word/media/chart3.png",
    "word/media/chart4.png",
};

/* ================================================================== */
/*  外部依赖                                                            */
/* ================================================================== */

extern cmap_uint32_t_data_dict_item_dsc_t *g_data_dict;

/* ================================================================== */
/*  页面状态                                                            */
/* ================================================================== */

static lv_obj_t *page_container = NULL;
static lv_obj_t *toolbar        = NULL;
static lv_obj_t *data_table     = NULL;

/* ================================================================== */
/*  路径辅助                                                            */
/* ================================================================== */

static void get_exe_dir(char *buf, size_t size)
{
#ifdef _WIN32
    GetModuleFileNameA(NULL, buf, (DWORD)size);
    char *last_sep = strrchr(buf, '\\');
    if (last_sep) *(last_sep + 1) = '\0';
#else
    strncpy(buf, "./", size);
#endif
}

static void build_path(char *out, size_t out_size,
                       const char *subdir, const char *filename)
{
    char exe_dir[512];
    get_exe_dir(exe_dir, sizeof(exe_dir));
    snprintf(out, out_size, "%s%s%s", exe_dir, subdir ? subdir : "", filename);
}

/* ================================================================== */
/*  Toast 提示                                                          */
/* ================================================================== */

static void toast_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *toast = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (toast && lv_obj_is_valid(toast)) {
        lv_obj_delete(toast);
    }
    lv_timer_delete(timer);
}

static void show_toast(const char *msg, uint32_t bg_hex)
{
    lv_obj_t *scr = lv_screen_active();

    lv_obj_t *toast = lv_obj_create(scr);
    lv_obj_set_size(toast, 400, 50);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(toast, lv_color_hex(bg_hex), 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_90, 0);
    lv_obj_set_style_radius(toast, 8, 0);
    lv_obj_set_style_border_width(toast, 0, 0);
    lv_obj_set_style_pad_all(toast, 10, 0);
    lv_obj_remove_flag(toast, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(toast);
    lv_label_set_text(label, msg);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);

    lv_timer_create(toast_timer_cb, 3000, toast);
}

/* ================================================================== */
/*  数据查询                                                            */
/* ================================================================== */

/**
 * 从 g_data_dict 查询 bus_addr 对应的当前值，格式化为字符串
 * 使用 cmap_..._get_value_ptr() 获取值指针（不是 node->val）
 */
static void query_cur_val(uint32_t bus_addr, char *out, size_t out_size)
{
    if (!g_data_dict) {
        snprintf(out, out_size, "N/A");
        return;
    }

    cmap_uint32_t_data_dict_item_dsc_t_node *node =
        cmap_uint32_t_data_dict_item_dsc_t_find(g_data_dict, bus_addr);

    if (node != cmap_uint32_t_data_dict_item_dsc_t_end(g_data_dict)) {
        data_dict_item_dsc_t *entry =
            cmap_uint32_t_data_dict_item_dsc_t_get_value_ptr(node);
        char fmt[16];
        snprintf(fmt, sizeof(fmt), "%%.%uf", entry->decimal);
        snprintf(out, out_size, fmt, entry->cur_val);
    } else {
        snprintf(out, out_size, "N/A");
    }
}

/* ================================================================== */
/*  构建书签数组                                                         */
/* ================================================================== */

static int build_bookmark_array(bookmark_replace_t *bookmarks)
{
    int idx = 0;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    /* report_no */
    snprintf(bookmarks[idx].name, sizeof(bookmarks[idx].name), "report_no");
    snprintf(bookmarks[idx].text, sizeof(bookmarks[idx].text),
             "RPT-%04d%02d%02d-%02d%02d%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    idx++;

    /* test_date */
    snprintf(bookmarks[idx].name, sizeof(bookmarks[idx].name), "test_date");
    snprintf(bookmarks[idx].text, sizeof(bookmarks[idx].text),
             "%04d-%02d-%02d %02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min);
    idx++;

    /* operator_name */
    snprintf(bookmarks[idx].name, sizeof(bookmarks[idx].name), "operator_name");
    snprintf(bookmarks[idx].text, sizeof(bookmarks[idx].text), "---");
    idx++;

    /* device_id */
    snprintf(bookmarks[idx].name, sizeof(bookmarks[idx].name), "device_id");
    snprintf(bookmarks[idx].text, sizeof(bookmarks[idx].text), "DEV_TYPE_LOAD_TESTER");
    idx++;

    /* Var01 ~ Var20 */
    int i;
    for (i = 0; i < VAR_MAP_COUNT; i++) {
        snprintf(bookmarks[idx].name, sizeof(bookmarks[idx].name),
                 "%s", VAR_MAP[i].bookmark);
        query_cur_val(VAR_MAP[i].bus_addr,
                      bookmarks[idx].text, sizeof(bookmarks[idx].text));
        idx++;
    }

    /* remarks */
    snprintf(bookmarks[idx].name, sizeof(bookmarks[idx].name), "remarks");
    snprintf(bookmarks[idx].text, sizeof(bookmarks[idx].text),
             "Auto-generated report, data collected: %04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    idx++;

    return idx;
}

/* ================================================================== */
/*  图表截图 (占位)                                                      */
/* ================================================================== */

// static void snapshot_dashboard_charts(image_replace_t *images, uint32_t *image_count)
// {
//     /*
//      * TODO: 接入实际 LVGL snapshot:
//      *   1. 获取 Dashboard 上的 4 个 chart 对象
//      *   2. lv_snapshot_take() -> RGB565 buffer
//      *   3. RGB565 -> RGB888
//      *   4. stbi_write_png_to_mem() -> PNG data
//      *   5. 填入 images[i].inner_path / data / data_size
//      *   6. *image_count = CHART_COUNT;
//      */
//     (void)images;
//     (void)CHART_PATHS;
//     *image_count = 0;
// }
/**
 * RGB565 -> RGB888 转换并编码为 PNG
 */
static void rgb565_to_rgb888(const uint16_t *src, uint8_t *dst, uint32_t pixel_count)
{
    uint32_t i;
    for (i = 0; i < pixel_count; i++) {
        uint16_t px = src[i];
        dst[i * 3 + 0] = (uint8_t)(((px >> 11) & 0x1F) * 255 / 31);  /* R */
        dst[i * 3 + 1] = (uint8_t)(((px >> 5)  & 0x3F) * 255 / 63);  /* G */
        dst[i * 3 + 2] = (uint8_t)(( px        & 0x1F) * 255 / 31);  /* B */
    }
}

/* stbi_write_png_to_mem 的回调 */
static void stbi_write_cb(void *context, void *data, int size)
{
    /* context 指向一个简单的动态缓冲区结构 */
    typedef struct { uint8_t *buf; uint32_t len; uint32_t cap; } png_buf_t;
    png_buf_t *pb = (png_buf_t *)context;

    if (pb->len + (uint32_t)size > pb->cap) {
        uint32_t new_cap = (pb->cap == 0) ? 65536 : pb->cap;
        while (new_cap < pb->len + (uint32_t)size) new_cap *= 2;
        uint8_t *tmp = (uint8_t *)realloc(pb->buf, new_cap);
        if (!tmp) return;
        pb->buf = tmp;
        pb->cap = new_cap;
    }
    memcpy(pb->buf + pb->len, data, (size_t)size);
    pb->len += (uint32_t)size;
}

typedef struct { uint8_t *buf; uint32_t len; uint32_t cap; } png_buf_t;

static void snapshot_dashboard_charts(image_replace_t *images, uint32_t *image_count)
{
    chart_snapshot_t snapshots[CHART_COUNT];
    int chart_count = Dashboard_Get_Chart_Snapshots(snapshots, CHART_COUNT);

    /* 静态缓冲区，保存 PNG 数据直到 docx_export 完成 */
    static png_buf_t png_bufs[CHART_COUNT];
    static uint8_t *rgb_buf = NULL;
    static uint32_t rgb_buf_size = 0;

    *image_count = 0;

    int i;
    for (i = 0; i < chart_count && i < (int)CHART_COUNT; i++) {
        uint32_t w = snapshots[i].width;
        uint32_t h = snapshots[i].height;
        uint32_t pixel_count = w * h;

        if (!snapshots[i].buffer || pixel_count == 0) continue;

        /* 确保 RGB888 转换缓冲区够大 */
        uint32_t needed = pixel_count * 3;
        if (needed > rgb_buf_size) {
            uint8_t *tmp = (uint8_t *)realloc(rgb_buf, needed);
            if (!tmp) continue;
            rgb_buf = tmp;
            rgb_buf_size = needed;
        }

        /* RGB565 -> RGB888 */
        rgb565_to_rgb888(snapshots[i].buffer, rgb_buf, pixel_count);

        /* 编码为 PNG */
        if (png_bufs[i].buf) { free(png_bufs[i].buf); }
        memset(&png_bufs[i], 0, sizeof(png_buf_t));

        stbi_write_png_to_func(stbi_write_cb, &png_bufs[i],
                               (int)w, (int)h, 3, rgb_buf, (int)(w * 3));

        if (png_bufs[i].len > 0) {
            images[*image_count].inner_path = CHART_PATHS[i];
            images[*image_count].data       = png_bufs[i].buf;
            images[*image_count].data_size  = png_bufs[i].len;
            (*image_count)++;
        }
    }
}

/* ================================================================== */
/*  导出回调                                                            */
/* ================================================================== */

static void export_word_cb(lv_event_t *e)
{
    (void)e;

    bookmark_replace_t bookmarks[25];
    memset(bookmarks, 0, sizeof(bookmarks));
    int bm_count = build_bookmark_array(bookmarks);
  

    image_replace_t images[CHART_COUNT];
    memset(images, 0, sizeof(images));
    uint32_t img_count = 0;
    snapshot_dashboard_charts(images, &img_count);

    char template_path[512];
    char output_path[512];
    build_path(template_path, sizeof(template_path),
               "template\\", "report_template_v2.docx");

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[128];
    snprintf(filename, sizeof(filename),
             "report_%04d%02d%02d_%02d%02d%02d.docx",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    build_path(output_path, sizeof(output_path), "export\\", filename);

#ifdef _WIN32
    {
        char export_dir[512];
        build_path(export_dir, sizeof(export_dir), "export\\", "");
        CreateDirectoryA(export_dir, NULL);
    }
#endif

    docx_export_config_t config;
    memset(&config, 0, sizeof(config));
    config.template_path  = template_path;
    config.output_path    = output_path;
    config.bookmarks      = bookmarks;
    config.bookmark_count = (uint32_t)bm_count;
    config.images         = (img_count > 0) ? images : NULL;
    config.image_count    = img_count;

    docx_export_result_t result;
    int ret = docx_export(&config, &result);

    if (ret == 0 && result.success) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Word OK! %d/%d bookmarks  %s",
                 result.bookmarks_replaced, result.bookmarks_total, filename);
        show_toast(msg, 0x27AE60);
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg), "Word FAIL: %s", result.error_msg);
        show_toast(msg, 0xE74C3C);
    }
}

static void export_pdf_cb(lv_event_t *e)
{
    (void)e;
    show_toast("PDF export - coming soon...", 0x7F8C8D);
}

static void export_excel_cb(lv_event_t *e)
{
    (void)e;
    show_toast("Excel export - coming soon...", 0x7F8C8D);
}

static void export_xml_cb(lv_event_t *e)
{
    (void)e;
    show_toast("XML export - coming soon...", 0x7F8C8D);
}

static void export_all_cb(lv_event_t *e)
{
    (void)e;
    show_toast("Batch export - coming soon...", 0x7F8C8D);
}

/* ================================================================== */
/*  UI: 工具栏                                                          */
/* ================================================================== */

typedef struct {
    const char    *label;
    uint32_t       color_hex;
    lv_event_cb_t  cb;
    int            width;
} export_btn_cfg_t;

static void create_toolbar(lv_obj_t *parent)
{
    toolbar = lv_obj_create(parent);
    lv_obj_set_size(toolbar, LV_PCT(100), TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_color(toolbar, lv_color_hex(CLR_PANEL), 0);
    lv_obj_set_style_bg_opa(toolbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(toolbar, 0, 0);
    lv_obj_set_style_radius(toolbar, 0, 0);
    lv_obj_set_style_pad_hor(toolbar, 16, 0);
    lv_obj_set_style_pad_ver(toolbar, 6, 0);
    lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toolbar, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(toolbar, BTN_GAP, 0);
    lv_obj_remove_flag(toolbar, LV_OBJ_FLAG_SCROLLABLE);

    const export_btn_cfg_t cfgs[EXPORT_BTN_COUNT] = {
        { "Word",                    CLR_BTN_WORD,  export_word_cb,  BTN_WIDTH },
        { "PDF",                     CLR_BTN_PDF,   export_pdf_cb,   BTN_WIDTH },
        { "Excel",                   CLR_BTN_EXCEL, export_excel_cb, BTN_WIDTH },
        { "XML",                     CLR_BTN_XML,   export_xml_cb,   BTN_WIDTH },
        { LV_SYMBOL_DOWNLOAD " All", CLR_BTN_ALL,   export_all_cb,   130       },
    };

    int i;
    for (i = 0; i < EXPORT_BTN_COUNT; i++) {
        lv_obj_t *btn = lv_button_create(toolbar);
        lv_obj_set_size(btn, cfgs[i].width, BTN_HEIGHT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(cfgs[i].color_hex), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_add_event_cb(btn, cfgs[i].cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, cfgs[i].label);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
        lv_obj_center(lbl);
    }
}

/* ================================================================== */
/*  UI: 数据表格                                                        */
/* ================================================================== */

static const int COL_PCT[4] = { 8, 52, 22, 18 };

static void create_data_table(lv_obj_t *parent)
{
    data_table = lv_obj_create(parent);
    lv_obj_set_size(data_table, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(data_table, lv_color_hex(CLR_PANEL), 0);
    lv_obj_set_style_bg_opa(data_table, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(data_table, 1, 0);
    lv_obj_set_style_border_color(data_table, lv_color_hex(CLR_BORDER_PG), 0);
    lv_obj_set_style_radius(data_table, 8, 0);
    lv_obj_set_style_pad_all(data_table, 0, 0);
    lv_obj_set_flex_flow(data_table, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(data_table, 0, 0);
    lv_obj_remove_flag(data_table, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 表头 ---- */
    {
        lv_obj_t *hdr = lv_obj_create(data_table);
        lv_obj_set_size(hdr, LV_PCT(100), 40);
        lv_obj_set_style_bg_color(hdr, lv_color_hex(CLR_HEADER_BG), 0);
        lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(hdr, 0, 0);
        lv_obj_set_style_radius(hdr, 0, 0);
        lv_obj_set_style_pad_all(hdr, 0, 0);
        lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
        lv_obj_remove_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);

        const char *titles[4] = { "#", "Parameter", "Value", "Unit" };
        int c;
        for (c = 0; c < 4; c++) {
            lv_obj_t *cell = lv_obj_create(hdr);
            lv_obj_set_size(cell, LV_PCT(COL_PCT[c]), LV_PCT(100));
            lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_set_style_radius(cell, 0, 0);
            lv_obj_set_style_pad_all(cell, 4, 0);
            lv_obj_remove_flag(cell, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t *lbl = lv_label_create(cell);
            lv_label_set_text(lbl, titles[c]);
            lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
            lv_obj_center(lbl);
        }
    }

    /* ---- 数据行 ---- */
    int r;
    for (r = 0; r < VAR_MAP_COUNT; r++) {
        lv_obj_t *row = lv_obj_create(data_table);
        lv_obj_set_size(row, LV_PCT(100), 34);
        lv_obj_set_style_bg_color(row,
            lv_color_hex((r % 2 == 0) ? CLR_PANEL : CLR_EXP_ROW_ALT), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(CLR_BORDER_PG), 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_radius(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        char val_str[64];
        query_cur_val(VAR_MAP[r].bus_addr, val_str, sizeof(val_str));

        char num_str[8];
        snprintf(num_str, sizeof(num_str), "%d", r + 1);

        const char *cell_texts[4] = { num_str, VAR_DESCS[r], val_str, VAR_UNITS[r] };
        int c;
        for (c = 0; c < 4; c++) {
            lv_obj_t *cell = lv_obj_create(row);
            lv_obj_set_size(cell, LV_PCT(COL_PCT[c]), LV_PCT(100));
            lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_set_style_radius(cell, 0, 0);
            lv_obj_set_style_pad_all(cell, 2, 0);
            lv_obj_remove_flag(cell, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t *lbl = lv_label_create(cell);
            lv_label_set_text(lbl, cell_texts[c]);
            lv_obj_set_style_text_color(lbl, lv_color_hex(CLR_EXP_TEXT), 0);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
            if (c == 1) {
                lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
                lv_obj_set_width(lbl, LV_PCT(95));
            }
            lv_obj_center(lbl);
        }
    }
}

/* ================================================================== */
/*  页面生命周期                                                         */
/* ================================================================== */

void Experimental_Data_Weight(smp_ctx_t *ctx)
{
    lv_obj_t *parent = ctx->pages[NAV_PAGE_EXPERIMENT]->activity_content;
    
    page_container = lv_obj_create(parent);
    lv_obj_set_size(page_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_container, 0, 0);
    lv_obj_set_style_radius(page_container, 0, 0);
    lv_obj_set_style_pad_all(page_container, 8, 0);
    lv_obj_set_flex_flow(page_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page_container, 8, 0);
    lv_obj_remove_flag(page_container, LV_OBJ_FLAG_SCROLLABLE);

    /* 只创建工具栏 */
    create_toolbar(page_container);
}
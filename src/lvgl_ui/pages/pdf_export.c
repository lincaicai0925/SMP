/**
 * @file pdf_export.c
 * @brief PDF 报告导出引擎实现
 *
 * 依赖: libharu (hpdf.h), TTF 中文字体
 * 布局: A4 竖版，包含标题、元信息、数据表格、图表图片
 */

#include "pages/include/pdf_export.h"
#include "hpdf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ------------------------------------------------------------------ */
/*  常量                                                                */
/* ------------------------------------------------------------------ */

/** A4 尺寸 (pt) */
#define PAGE_W          595.0f
#define PAGE_H          842.0f

/** 页面边距 */
#define MARGIN_LEFT      50.0f
#define MARGIN_RIGHT     50.0f
#define MARGIN_TOP       60.0f
#define MARGIN_BOTTOM    50.0f

/** 内容区宽度 */
#define CONTENT_W       (PAGE_W - MARGIN_LEFT - MARGIN_RIGHT)

/** 字号 */
#define FONT_SIZE_TITLE  18.0f
#define FONT_SIZE_FIELD  10.0f
#define FONT_SIZE_HEADER 9.0f
#define FONT_SIZE_CELL   8.5f

/** 表格行高 */
#define ROW_HEIGHT       18.0f
#define HEADER_HEIGHT    22.0f

/** 表格列宽 (4 列: 变量名、描述、当前值、单位) */
#define COL_W_NAME       70.0f
#define COL_W_UNIT       50.0f
#define COL_W_VALUE      70.0f
#define COL_W_DESC       (CONTENT_W - COL_W_NAME - COL_W_VALUE - COL_W_UNIT)

/* ------------------------------------------------------------------ */
/*  内部辅助                                                            */
/* ------------------------------------------------------------------ */

static void set_error(pdf_export_result_t *r, const char *fmt, ...)
{
    if (!r) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(r->error_msg, sizeof(r->error_msg), fmt, ap);
    va_end(ap);
    r->success = 0;
}

static void hpdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
                                void *user_data)
{
    (void)user_data;
    printf("[libharu] error: 0x%04X, detail: %u\n",
           (unsigned)error_no, (unsigned)detail_no);
}

/** 画一条水平线 */
static void draw_hline(HPDF_Page page, float x, float y, float w)
{
    HPDF_Page_MoveTo(page, x, y);
    HPDF_Page_LineTo(page, x + w, y);
    HPDF_Page_Stroke(page);
}

/** 在矩形区域内绘制文本 (左对齐，垂直居中) */
static void draw_cell_text(HPDF_Page page, HPDF_Font font, float font_size,
                           float x, float y, float w, float h,
                           const char *text)
{
    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, font_size);
    /* 垂直居中: 基线 = y + (h - font_size) / 2 + 少量偏移 */
    float text_y = y + (h - font_size) / 2.0f + 1.0f;
    HPDF_Page_TextOut(page, x + 4.0f, text_y, text);
    HPDF_Page_EndText(page);
}

/** 画填充矩形 */
static void draw_filled_rect(HPDF_Page page, float x, float y, float w, float h,
                             float r, float g, float b)
{
    HPDF_Page_SetRGBFill(page, r, g, b);
    HPDF_Page_Rectangle(page, x, y, w, h);
    HPDF_Page_Fill(page);
}

/** 画带边框的矩形 */
static void draw_rect_border(HPDF_Page page, float x, float y, float w, float h)
{
    HPDF_Page_Rectangle(page, x, y, w, h);
    HPDF_Page_Stroke(page);
}

/* ------------------------------------------------------------------ */
/*  导出实现                                                            */
/* ------------------------------------------------------------------ */

int Pdf_Export(const pdf_export_config_t *config, pdf_export_result_t *result)
{
    pdf_export_result_t local_result;
    if (!result) result = &local_result;
    memset(result, 0, sizeof(*result));

    if (!config || !config->output_path) {
        set_error(result, "Invalid config or output_path");
        return -1;
    }

    /* 创建 PDF 文档 */
    HPDF_Doc pdf = HPDF_New(hpdf_error_handler, NULL);
    if (!pdf) {
        set_error(result, "HPDF_New failed");
        return -1;
    }

    /* 设置中文编码 */
    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, "UTF-8");

    /* 加载字体 */
    HPDF_Font font_cn  = NULL;
    HPDF_Font font_en  = NULL;

    if (config->font_path && config->font_path[0]) {
        const char *font_name = HPDF_LoadTTFontFromFile(pdf, config->font_path,
                                                         HPDF_TRUE);
        if (font_name) {
            font_cn = HPDF_GetFont(pdf, font_name, "UTF-8");
        }
    }

    /* 英文后备字体 (Helvetica) */
    font_en = HPDF_GetFont(pdf, "Helvetica", NULL);

    if (!font_cn) {
        font_cn = font_en;  /* 无中文字体时退回英文 */
    }

    /* ============================================================== */
    /*  第一页: 标题 + 元信息 + 数据表格                                  */
    /* ============================================================== */
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    HPDF_Page_SetLineWidth(page, 0.5f);

    float cur_y = PAGE_H - MARGIN_TOP;

    /* ---------- 标题 ---------- */
    if (config->title && config->title[0]) {
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font_cn, FONT_SIZE_TITLE);
        HPDF_Page_SetRGBFill(page, 0.1f, 0.1f, 0.1f);

        /* 标题居中 */
        float tw = HPDF_Page_TextWidth(page, config->title);
        float tx = (PAGE_W - tw) / 2.0f;
        if (tx < MARGIN_LEFT) tx = MARGIN_LEFT;
        HPDF_Page_TextOut(page, tx, cur_y, config->title);
        HPDF_Page_EndText(page);

        cur_y -= 30.0f;

        /* 标题下划线 */
        HPDF_Page_SetRGBStroke(page, 0.3f, 0.3f, 0.3f);
        draw_hline(page, MARGIN_LEFT, cur_y, CONTENT_W);
        cur_y -= 20.0f;
    }

    /* ---------- 元信息字段 ---------- */
    if (config->fields && config->field_count > 0) {
        uint32_t i;
        float field_label_w = 120.0f;
        for (i = 0; i < config->field_count; i++) {
            HPDF_Page_SetRGBFill(page, 0.2f, 0.2f, 0.2f);
            HPDF_Page_BeginText(page);
            HPDF_Page_SetFontAndSize(page, font_cn, FONT_SIZE_FIELD);
            HPDF_Page_TextOut(page, MARGIN_LEFT, cur_y,
                              config->fields[i].label);
            HPDF_Page_EndText(page);

            HPDF_Page_SetRGBFill(page, 0.1f, 0.1f, 0.1f);
            HPDF_Page_BeginText(page);
            HPDF_Page_SetFontAndSize(page, font_cn, FONT_SIZE_FIELD);
            HPDF_Page_TextOut(page, MARGIN_LEFT + field_label_w, cur_y,
                              config->fields[i].value);
            HPDF_Page_EndText(page);

            cur_y -= 16.0f;
        }
        cur_y -= 10.0f;
    }

    /* ---------- 数据表格 ---------- */
    if (config->rows && config->row_count > 0) {
        float table_x = MARGIN_LEFT;
        float col_x[4];
        col_x[0] = table_x;
        col_x[1] = table_x + COL_W_NAME;
        col_x[2] = table_x + COL_W_NAME + COL_W_DESC;
        col_x[3] = table_x + COL_W_NAME + COL_W_DESC + COL_W_VALUE;

        float col_w[4] = { COL_W_NAME, COL_W_DESC, COL_W_VALUE, COL_W_UNIT };
        const char *headers[4] = {
            "\xe5\x8f\x98\xe9\x87\x8f",       /* 变量 */
            "\xe6\x8f\x8f\xe8\xbf\xb0",       /* 描述 */
            "\xe5\xbd\x93\xe5\x89\x8d\xe5\x80\xbc", /* 当前值 */
            "\xe5\x8d\x95\xe4\xbd\x8d"        /* 单位 */
        };

        /* 表头背景 */
        draw_filled_rect(page, table_x, cur_y - HEADER_HEIGHT,
                         CONTENT_W, HEADER_HEIGHT,
                         0.15f, 0.16f, 0.19f);

        /* 表头文本 (白色) */
        HPDF_Page_SetRGBFill(page, 1.0f, 1.0f, 1.0f);
        {
            int j;
            for (j = 0; j < 4; j++) {
                draw_cell_text(page, font_cn, FONT_SIZE_HEADER,
                               col_x[j], cur_y - HEADER_HEIGHT,
                               col_w[j], HEADER_HEIGHT, headers[j]);
            }
        }

        cur_y -= HEADER_HEIGHT;

        /* 表头边框 */
        HPDF_Page_SetRGBStroke(page, 0.4f, 0.4f, 0.4f);
        draw_rect_border(page, table_x, cur_y, CONTENT_W, HEADER_HEIGHT);

        /* 数据行 */
        uint32_t i;
        for (i = 0; i < config->row_count; i++) {
            /* 检查是否需要换页 */
            if (cur_y - ROW_HEIGHT < MARGIN_BOTTOM) {
                page = HPDF_AddPage(pdf);
                HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
                HPDF_Page_SetLineWidth(page, 0.5f);
                cur_y = PAGE_H - MARGIN_TOP;
            }

            /* 交替行背景色 */
            if (i % 2 == 0) {
                draw_filled_rect(page, table_x, cur_y - ROW_HEIGHT,
                                 CONTENT_W, ROW_HEIGHT,
                                 0.96f, 0.96f, 0.97f);
            }

            /* 行文本 */
            HPDF_Page_SetRGBFill(page, 0.1f, 0.1f, 0.1f);
            draw_cell_text(page, font_cn, FONT_SIZE_CELL,
                           col_x[0], cur_y - ROW_HEIGHT,
                           col_w[0], ROW_HEIGHT, config->rows[i].name);
            draw_cell_text(page, font_cn, FONT_SIZE_CELL,
                           col_x[1], cur_y - ROW_HEIGHT,
                           col_w[1], ROW_HEIGHT, config->rows[i].desc);
            draw_cell_text(page, font_en, FONT_SIZE_CELL,
                           col_x[2], cur_y - ROW_HEIGHT,
                           col_w[2], ROW_HEIGHT, config->rows[i].value);
            draw_cell_text(page, font_en, FONT_SIZE_CELL,
                           col_x[3], cur_y - ROW_HEIGHT,
                           col_w[3], ROW_HEIGHT, config->rows[i].unit);

            /* 行边框 */
            HPDF_Page_SetRGBStroke(page, 0.8f, 0.8f, 0.8f);
            draw_rect_border(page, table_x, cur_y - ROW_HEIGHT,
                             CONTENT_W, ROW_HEIGHT);

            /* 列分隔线 */
            {
                int j;
                for (j = 1; j < 4; j++) {
                    HPDF_Page_MoveTo(page, col_x[j], cur_y);
                    HPDF_Page_LineTo(page, col_x[j], cur_y - ROW_HEIGHT);
                    HPDF_Page_Stroke(page);
                }
            }

            cur_y -= ROW_HEIGHT;
        }
    }

    /* ============================================================== */
    /*  图表图片 (新页)                                                  */
    /* ============================================================== */
    if (config->images && config->image_count > 0) {
        page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
        HPDF_Page_SetLineWidth(page, 0.5f);
        cur_y = PAGE_H - MARGIN_TOP;

        /* 图表标题 */
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font_cn, 14.0f);
        HPDF_Page_SetRGBFill(page, 0.1f, 0.1f, 0.1f);

        {
            const char *chart_title = "\xe5\x9b\xbe\xe8\xa1\xa8";  /* 图表 */
            float tw = HPDF_Page_TextWidth(page, chart_title);
            HPDF_Page_TextOut(page, (PAGE_W - tw) / 2.0f, cur_y, chart_title);
        }
        HPDF_Page_EndText(page);
        cur_y -= 25.0f;

        /* 每张图占半页宽度，2x2 网格 */
        float img_w = (CONTENT_W - 10.0f) / 2.0f;
        float img_h = img_w * 0.6f;  /* 宽高比约 5:3 */

        uint32_t i;
        for (i = 0; i < config->image_count; i++) {
            float ix, iy;
            uint32_t col_idx = i % 2;
            uint32_t row_idx = i / 2;

            ix = MARGIN_LEFT + col_idx * (img_w + 10.0f);
            iy = cur_y - (row_idx + 1) * (img_h + 10.0f);

            /* 检查是否需要换页 */
            if (iy < MARGIN_BOTTOM) {
                page = HPDF_AddPage(pdf);
                HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
                cur_y = PAGE_H - MARGIN_TOP;
                iy = cur_y - (img_h + 10.0f);
            }

            HPDF_Image img = HPDF_LoadJpegImageFromMem(pdf,
                config->images[i].data, config->images[i].data_size);

            if (img) {
                HPDF_Page_DrawImage(page, img, ix, iy, img_w, img_h);

                /* 图片边框 */
                HPDF_Page_SetRGBStroke(page, 0.7f, 0.7f, 0.7f);
                draw_rect_border(page, ix, iy, img_w, img_h);
            }
        }
    }

    /* ============================================================== */
    /*  保存文件                                                        */
    /* ============================================================== */
    HPDF_STATUS status = HPDF_SaveToFile(pdf, config->output_path);
    HPDF_Free(pdf);

    if (status != HPDF_OK) {
        set_error(result, "HPDF_SaveToFile failed: 0x%04X", (unsigned)status);
        return -1;
    }

    result->success = 1;
    return 0;
}

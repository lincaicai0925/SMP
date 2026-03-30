#include "pages/include/log_manage.h"
#include "log_export.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


#define EXPORT_PATH  "C:\\zdl\\work1\\SMP2026.1.16\\SMP\\Log Report\\"


static void rtf_write_utf8(FILE *fp, const char *utf8_str)
{
    const unsigned char *p = (const unsigned char *)utf8_str;
    while (*p) {
        unsigned int codepoint = 0;
        int bytes = 0;

        if (*p < 0x80) {
        
            if (*p == '\\') fputs("\\\\", fp);
            else if (*p == '{')  fputs("\\{",  fp);
            else if (*p == '}')  fputs("\\}",  fp);
            else                 fputc(*p, fp);
            p++;
            continue;
        } else if ((*p & 0xE0) == 0xC0) {
            codepoint = *p & 0x1F; bytes = 1;
        } else if ((*p & 0xF0) == 0xE0) {
            codepoint = *p & 0x0F; bytes = 2;
        } else if ((*p & 0xF8) == 0xF0) {
            codepoint = *p & 0x07; bytes = 3;
        }
        p++;
        for (int i = 0; i < bytes; i++) {
            codepoint = (codepoint << 6) | (*p & 0x3F);
            p++;
        }

       
        fprintf(fp, "\\u%u?", codepoint);
    }
}

/* ── 写入带样式的单元格 ──────────────────────── */
static void rtf_cell(FILE *fp, const char *text, bool is_header)
{
    if (is_header) {
        /* 表头：灰色背景 + 加粗 */
        fputs("\\pard\\intbl\\qc"
              "\\cb3"              /* 背景色 index 3 = 浅灰 */
              "\\b ",              /* 加粗 */
              fp);
    } else {
        fputs("\\pard\\intbl\\qc"
              "\\b0 ",
              fp);
    }
    rtf_write_utf8(fp, text);
    fputs("\\cell\n", fp);
}

/* ── 写入一行── */
static void rtf_row(FILE *fp)
{
    fputs("\\row\n", fp);
}


bool Log_Export_To_Word(const log_data_t *logs, int count, const char *filename)
{
    if (!logs || count <= 0) return false;

    /* 生成文件路径 */
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.rtf", EXPORT_PATH, filename);

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        /* 目录不存在时尝试在当前目录创建 */
        snprintf(filepath, sizeof(filepath), "%s.rtf", filename);
        fp = fopen(filepath, "w");
        if (!fp) return false;
    }

    /* ──────────────────────────────────────────
     * RTF 文件头
     * ────────────────────────────────────────── */
    fputs("{\\rtf1"
          "\\ansi"
          "\\ansicpg936"    /* GBK 代码页，支持中文 */
          "\\deff0"
          "\\deflang2052"   /* 语言：简体中文 */
          "\n", fp);

    /* 字体表 */
    fputs("{\\fonttbl"
          "{\\f0\\fswiss\\fcharset134 Microsoft YaHei;}"   /* 微软雅黑 */
          "{\\f1\\fmodern\\fcharset0 Courier New;}"
          "}\n", fp);

    /* 颜色表 */
    fputs("{\\colortbl;"
          "\\red0\\green0\\blue0;"          /* 1: 黑色（默认文字）*/
          "\\red63\\green118\\blue183;"     /* 2: 蓝色（标题）*/
          "\\red240\\green240\\blue240;"    /* 3: 浅灰（表头背景）*/
          "\\red220\\green53\\blue53;"      /* 4: 红色（异常）*/
          "\\red34\\green139\\blue34;"      /* 5: 绿色（正常）*/
          "\\red255\\green165\\blue0;"      /* 6: 橙色（警告）*/
          "}\n", fp);

    /* 文档默认样式 */
    fputs("\\f0\\fs22"        /* 微软雅黑 11pt */
          "\\widowctrl"
          "\\hyphauto0\n",
          fp);

    
    /* 主标题 */
    fputs("\\pard\\qc"
          "\\cf2\\b\\fs36 ",        /* 蓝色、加粗、18pt */
          fp);
    rtf_write_utf8(fp, "阀门驱控综合测试系统");
    fputs("\\par\n", fp);

    fputs("\\pard\\qc"
          "\\cf2\\b\\fs28 ",        /* 蓝色、加粗、14pt */
          fp);
    rtf_write_utf8(fp, "日志导出报告");
    fputs("\\par\n"
          "\\pard\\qc\\cf1\\b0\\fs22 "
          "\\brdrb\\brdrs\\brdrw10\\brsp100 ",   /* 下划线分隔 */
          fp);
    fputs("\\par\n", fp);

    /* 导出时间 */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y.%m.%d  %H:%M:%S", t);

    fputs("\\pard\\qc\\cf1\\fs20 ", fp);
    rtf_write_utf8(fp, "导出时间：");
    fputs(time_buf, fp);
    fputs("\\par\n", fp);

    /* 统计：按分类计数 */
    int cnt_test = 0, cnt_dev = 0, cnt_alarm = 0, cnt_comm = 0;
    int cnt_info = 0, cnt_warn = 0, cnt_error = 0;
    for (int i = 0; i < count; i++) {
        if (strstr(logs[i].log_category, "\xe6\xb5\x8b\xe8\xaf\x95")) cnt_test++;   /* 测试 */
        else if (strstr(logs[i].log_category, "\xe8\xae\xbe\xe5\xa4\x87")) cnt_dev++; /* 设备 */
        else if (strstr(logs[i].log_category, "\xe9\x80\x9a\xe4\xbf\xa1")) cnt_comm++;/* 通信 */
        else cnt_alarm++;

        if      (strcmp(logs[i].event_level, "Info")    == 0) cnt_info++;
        else if (strcmp(logs[i].event_level, "Warning") == 0) cnt_warn++;
        else                                                    cnt_error++;
    }

    char stat_buf[256];
    fputs("\\pard\\qc\\cf1\\fs20 ", fp);
    rtf_write_utf8(fp, "共导出：");
    fprintf(fp, "%d", count);
    rtf_write_utf8(fp, " 条  |  ");
    rtf_write_utf8(fp, "测试流程：");
    fprintf(fp, "%d", cnt_test);
    rtf_write_utf8(fp, "  |  ");
    rtf_write_utf8(fp, "设备运行：");
    fprintf(fp, "%d", cnt_dev);
    rtf_write_utf8(fp, "  |  ");
    rtf_write_utf8(fp, "异常报警：");
    fprintf(fp, "%d", cnt_alarm);
    rtf_write_utf8(fp, "  |  ");
    rtf_write_utf8(fp, "通信交互：");
    fprintf(fp, "%d", cnt_comm);
    fputs("\\par\n\\par\n", fp);

    /* ──────────────────────────────────────────
     * 数据表格
     * ────────────────────────────────────────── */
    #define CW_NO    600
    #define CW_TIME  1800
    #define CW_DEV   1000
    #define CW_CAT   1300
    #define CW_LEVEL  900
    #define CW_TYPE  1300
    #define CW_CONT  2500
    #define CW_STAT   900

    /* 列右边界（从左累加）*/
    #define POS1  (CW_NO)
    #define POS2  (POS1 + CW_TIME)
    #define POS3  (POS2 + CW_DEV)
    #define POS4  (POS3 + CW_CAT)
    #define POS5  (POS4 + CW_LEVEL)
    #define POS6  (POS5 + CW_TYPE)
    #define POS7  (POS6 + CW_CONT)
    #define POS8  (POS7 + CW_STAT)

    /* 表格行定义（每行都需要重复声明列宽）*/
    #define RTF_ROW_DEF \
        "\\trowd\\trqc\\trgaph60\n" \
        "\\cellx" #POS1 "\n" \
        "\\cellx" #POS2 "\n" \
        "\\cellx" #POS3 "\n" \
        "\\cellx" #POS4 "\n" \
        "\\cellx" #POS5 "\n" \
        "\\cellx" #POS6 "\n" \
        "\\cellx" #POS7 "\n" \
        "\\cellx" #POS8 "\n"

    /* ── 表头行 ── */
    fputs("\\trowd\\trqc\\trgaph60\n"
          "\\clcbpat3"   /* 单元格背景：浅灰 */
          "\\cellx600\n"
          "\\clcbpat3\\cellx2400\n"
          "\\clcbpat3\\cellx3400\n"
          "\\clcbpat3\\cellx4700\n"
          "\\clcbpat3\\cellx5600\n"
          "\\clcbpat3\\cellx6900\n"
          "\\clcbpat3\\cellx9400\n"
          "\\clcbpat3\\cellx10300\n",
          fp);

    rtf_cell(fp, "\xe5\xba\x8f\xe5\x8f\xb7",   true);  /* 序号 */
    rtf_cell(fp, "\xe6\x97\xb6\xe9\x97\xb4",   true);  /* 时间 */
    rtf_cell(fp, "\xe8\xae\xbe\xe5\xa4\x87",   true);  /* 设备 */
    rtf_cell(fp, "\xe5\x88\x86\xe7\xb1\xbb",   true);  /* 分类 */
    rtf_cell(fp, "\xe7\xba\xa7\xe5\x88\xab",   true);  /* 级别 */
    rtf_cell(fp, "\xe4\xba\x8b\xe4\xbb\xb6\xe7\xb1\xbb\xe5\x9e\x8b", true); /* 事件类型 */
    rtf_cell(fp, "\xe4\xba\x8b\xe4\xbb\xb6\xe5\x86\x85\xe5\xae\xb9", true); /* 事件内容 */
    rtf_cell(fp, "\xe7\x8a\xb6\xe6\x80\x81",   true);  /* 状态 */
    rtf_row(fp);

    /* ── 数据行 ── */
    for (int i = 0; i < count; i++) {
        const log_data_t *d = &logs[i];

        /* 判断状态和级别颜色 */
        bool is_alarm  = (strstr(d->log_category, "\xe5\xbc\x82\xe5\xb8\xb8") ||  /* 异常 */
                          strstr(d->log_category, "\xe6\x8a\xa5\xe8\xad\xa6")); /* 报警 */
        bool is_warn   = (strcmp(d->event_level, "Warning") == 0);
        bool is_error  = (strcmp(d->event_level, "Error")   == 0 ||
                          strcmp(d->event_level, "Fatal")   == 0);

        uint32_t level_color = is_error ? 4 : (is_warn ? 6 : 5);  /* 红/橙/绿 */

        /* 表格行定义 */
        fputs("\\trowd\\trqc\\trgaph60\n"
              "\\cellx600\n"
              "\\cellx2400\n"
              "\\cellx3400\n"
              "\\cellx4700\n"
              "\\cellx5600\n"
              "\\cellx6900\n"
              "\\cellx9400\n"
              "\\cellx10300\n",
              fp);

        /* 序号 */
        char no_buf[8];
        snprintf(no_buf, sizeof(no_buf), "%d", i + 1);
        rtf_cell(fp, no_buf, false);

        /* 时间（日期 + 时间拼接）*/
        char time_str[32];
        snprintf(time_str, sizeof(time_str), "%s %s", d->date, d->start_time);
        rtf_cell(fp, time_str, false);

        /* 设备 */
        rtf_cell(fp, d->dev_name, false);

        /* 分类 */
        rtf_cell(fp, d->log_category, false);

        /* 级别（带颜色）*/
        fprintf(fp, "\\pard\\intbl\\qc\\cf%u\\b0 ", level_color);
        rtf_write_utf8(fp, d->event_level);
        fputs("\\cf1\\cell\n", fp);

        /* 事件类型 */
        rtf_cell(fp, d->event_type, false);

        /* 事件内容（左对齐）*/
        fputs("\\pard\\intbl\\ql\\b0 ", fp);
        rtf_write_utf8(fp, d->event_content);
        fputs("\\cell\n", fp);

        /* 状态 */
        const char *status;
        uint32_t    status_color;
        if (is_error) {
            status = "\xe5\xbc\x82\xe5\xb8\xb8";  /* 异常 */
            status_color = 4;
        } else if (is_warn) {
            status = "\xe8\xad\xa6\xe5\x91\x8a";  /* 警告 */
            status_color = 6;
        } else {
            status = "\xe6\xad\xa3\xe5\xb8\xb8";  /* 正常 */
            status_color = 5;
        }
        fprintf(fp, "\\pard\\intbl\\qc\\cf%u\\b0 ", status_color);
        rtf_write_utf8(fp, status);
        fputs("\\cf1\\cell\n", fp);

        rtf_row(fp);
    }

    /* ── 文档结尾 ── */
    fputs("\\pard\n"
          "\\par\n",
          fp);

    fputs("\\pard\\qr\\fs18\\cf2 ",  fp);
    rtf_write_utf8(fp, "—— 阀门驱控综合测试系统  自动导出");
    fputs("\\par\n", fp);

    fputs("}\n", fp);   /* 关闭 RTF 根括号 */
    fclose(fp);
    return true;
}

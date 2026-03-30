/**
 * @file docx_export.c
 * @brief Word (.docx) 导出引擎实现
 *
 * 依赖: miniz.h (ZIP 读写), 标准 C 库
 * 编译: 需要链接 miniz.c，通过 CMake include_directories 找到 miniz.h
 */

#include "pages/include/docx_export.h"
#include "miniz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ------------------------------------------------------------------ */
/*  内部宏与常量                                                        */
/* ------------------------------------------------------------------ */

/** 初始 XML 缓冲区大小 (4 MB)，按需自动扩展 */
#define INITIAL_BUF_SIZE    (4 * 1024 * 1024)

/** XML 文件最大大小 (32 MB)，防止意外分配过大内存 */
#define MAX_XML_SIZE        (32 * 1024 * 1024)

/** 判断 ZIP 条目是否为 word/ 下的 XML 文件 */
#define IS_WORD_XML(name)   (strncmp((name), "word/", 5) == 0 && \
                             strstr((name), ".xml") != NULL)

/* ------------------------------------------------------------------ */
/*  内部辅助函数                                                        */
/* ------------------------------------------------------------------ */

/** 设置错误信息到 result */
static void set_error(docx_export_result_t *result, const char *fmt, ...)
{
    if (!result) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(result->error_msg, sizeof(result->error_msg), fmt, ap);
    va_end(ap);
    result->success = 0;
}

/**
 * XML 转义：将 & < > " ' 转为 XML 实体
 * @param src  原始 UTF-8 文本
 * @param dst  输出缓冲区
 * @param dst_size  缓冲区大小
 * @return 转义后的字符串长度，-1 表示缓冲区不足
 */
static int xml_escape(const char *src, char *dst, size_t dst_size)
{
    size_t pos = 0;
    const char *p = src;

    while (*p) {
        const char *ent = NULL;
        size_t ent_len = 0;

        switch (*p) {
        case '&':  ent = "&amp;";  ent_len = 5; break;
        case '<':  ent = "&lt;";   ent_len = 4; break;
        case '>':  ent = "&gt;";   ent_len = 4; break;
        case '"':  ent = "&quot;"; ent_len = 6; break;
        case '\'': ent = "&apos;"; ent_len = 6; break;
        default:   break;
        }

        if (ent) {
            if (pos + ent_len >= dst_size) return -1;
            memcpy(dst + pos, ent, ent_len);
            pos += ent_len;
        } else {
            if (pos + 1 >= dst_size) return -1;
            dst[pos++] = *p;
        }
        p++;
    }
    dst[pos] = '\0';
    return (int)pos;
}

/**
 * 在 XML 缓冲区中替换单个书签的内容
 *
 * 查找模式:
 *   <w:bookmarkStart w:name="XXX" w:id="N"/>
 *   ... 原有内容 (一个或多个 <w:r>...</w:r>) ...
 *   <w:bookmarkEnd w:id="N"/>
 *
 * 替换策略:
 *   1. 保留 bookmarkStart 和 bookmarkEnd 标签
 *   2. 提取原有 <w:rPr>...</w:rPr> 用于格式继承
 *   3. 删除 start 和 end 之间的所有原有内容
 *   4. 插入新的 <w:r> 节点，带继承格式和转义文本
 *
 * @param xml       XML 缓冲区 (会被 realloc)
 * @param xml_len   当前 XML 长度
 * @param xml_cap   缓冲区容量
 * @param bookmark  书签替换项
 * @return 1=替换成功, 0=未找到该书签
 */
static int replace_bookmark(char **xml, size_t *xml_len, size_t *xml_cap,
                            const bookmark_replace_t *bookmark)
{
    /* 构造搜索模式: w:name="书签名" */
    char name_pattern[128];
    snprintf(name_pattern, sizeof(name_pattern), "w:name=\"%s\"", bookmark->name);

    /* 在整个 XML 中搜索包含该书签名的 bookmarkStart */
    char *search_pos = *xml;
    char *bm_start_tag = NULL;

    while ((search_pos = strstr(search_pos, "<w:bookmarkStart")) != NULL) {
        /* 找到这个标签的结束 /> */
        char *tag_end = strstr(search_pos, "/>");
        if (!tag_end) { search_pos++; continue; }

        /* 检查此 bookmarkStart 标签内是否包含目标 name */
        size_t tag_span = (size_t)(tag_end - search_pos + 2);
        char *name_pos = strstr(search_pos, name_pattern);
        if (name_pos && name_pos < tag_end) {
            bm_start_tag = search_pos;
            break;
        }
        search_pos = tag_end + 2;
    }

    if (!bm_start_tag) return 0;  /* 未找到 */

    /* bookmarkStart 标签结束位置 (/>之后) */
    char *start_tag_end = strstr(bm_start_tag, "/>");
    if (!start_tag_end) return 0;
    start_tag_end += 2;  /* 跳过 /> */

    /* 从 bookmarkStart 之后找最近的 bookmarkEnd */
    /* 提取 bookmarkStart 的 w:id 值 */
char id_pattern[64];
{
    char *id_pos = strstr(bm_start_tag, "w:id=\"");
    if (!id_pos || id_pos > start_tag_end) return 0;
    id_pos += 6;
    char *id_end = strchr(id_pos, '"');
    if (!id_end) return 0;
    size_t id_len = (size_t)(id_end - id_pos);
    snprintf(id_pattern, sizeof(id_pattern), "<w:bookmarkEnd w:id=\"%.*s\"/>", (int)id_len, id_pos);
}
char *bm_end_tag = strstr(start_tag_end, id_pattern);
    if (!bm_end_tag) return 0;

   char *end_tag_close = bm_end_tag + strlen(id_pattern);
    if (!end_tag_close) return 0;
    end_tag_close += 2;  /* bookmarkEnd 标签完全结束的位置 */

    /* ---- 提取原有格式 (rPr) ---- */
    char rpr_buf[1024] = {0};
    {
        char *rpr_start = strstr(start_tag_end, "<w:rPr>");
        if (rpr_start && rpr_start < bm_end_tag) {
            char *rpr_end = strstr(rpr_start, "</w:rPr>");
            if (rpr_end && rpr_end < bm_end_tag) {
                size_t rpr_len = (size_t)(rpr_end - rpr_start + 8); /* 8 = strlen("</w:rPr>") */
                if (rpr_len < sizeof(rpr_buf)) {
                    memcpy(rpr_buf, rpr_start, rpr_len);
                    rpr_buf[rpr_len] = '\0';
                }
            }
        }
    }

    /* ---- 构建替换内容 ---- */
    char escaped_text[2048];
    if (xml_escape(bookmark->text, escaped_text, sizeof(escaped_text)) < 0) {
        /* 文本过长，截断 */
        strncpy(escaped_text, bookmark->text, sizeof(escaped_text) - 1);
        escaped_text[sizeof(escaped_text) - 1] = '\0';
    }

    /* 构造新的 <w:r> 节点 */
    char new_run[4096];
    if (rpr_buf[0]) {
        snprintf(new_run, sizeof(new_run),
                 "<w:r>%s<w:t xml:space=\"preserve\">%s</w:t></w:r>",
                 rpr_buf, escaped_text);
    } else {
        snprintf(new_run, sizeof(new_run),
                 "<w:r><w:t xml:space=\"preserve\">%s</w:t></w:r>",
                 escaped_text);
    }

    size_t new_run_len = strlen(new_run);

    /* ---- 执行替换：删除 start→end 之间的内容，插入新 run ---- */
    /* 保留: [... bookmarkStart/>] [新run] [<bookmarkEnd .../>] ... */
    size_t old_content_len = (size_t)(bm_end_tag - start_tag_end);
    size_t new_content_len = new_run_len;
    int size_diff = (int)new_content_len - (int)old_content_len;

    /* 确保缓冲区足够 */
    if (size_diff > 0) {
        while (*xml_len + (size_t)size_diff + 1 > *xml_cap) {
            *xml_cap *= 2;
            char *tmp = (char *)realloc(*xml, *xml_cap);
            if (!tmp) return 0;

            /* 重新计算指针偏移 */
            ptrdiff_t offset = tmp - *xml;
            start_tag_end += offset;
            bm_end_tag    += offset;
            end_tag_close += offset;
            *xml = tmp;
        }
    }

    /* 移动 bookmarkEnd 之后的内容 */
    memmove(start_tag_end + new_content_len,
            bm_end_tag,
            *xml_len - (size_t)(bm_end_tag - *xml) + 1); /* +1 包含 \0 */

    /* 写入新内容 */
    memcpy(start_tag_end, new_run, new_content_len);

    *xml_len = (size_t)((int)*xml_len + size_diff);

    return 1;
}

/**
 * 对一段 XML 内容执行所有书签替换
 * @return 替换成功的书签数量
 */

static int process_xml_bookmarks(char **xml, size_t *xml_len, size_t *xml_cap,
                                 const bookmark_replace_t *bookmarks,
                                 uint32_t bookmark_count)
{
    int replaced = 0;
    uint32_t i;
    for (i = 0; i < bookmark_count; i++) {
        if (replace_bookmark(xml, xml_len, xml_cap, &bookmarks[i])) {
            replaced++;
        }
    }
    return replaced;
}
/* ------------------------------------------------------------------ */
/*  公共接口实现                                                        */
/* ------------------------------------------------------------------ */

int docx_export(const docx_export_config_t *config, docx_export_result_t *result)
{
    docx_export_result_t local_result;
    memset(&local_result, 0, sizeof(local_result));

    if (!result) result = &local_result;
    memset(result, 0, sizeof(*result));
    result->bookmarks_total = (int)config->bookmark_count;

    /* ---- 参数校验 ---- */
    if (!config || !config->template_path || !config->output_path) {
        set_error(result, "Invalid config: null pointers");
        return -1;
    }

    /* ---- 打开模板 ZIP ---- */
    mz_zip_archive src_zip;
    memset(&src_zip, 0, sizeof(src_zip));
    if (!mz_zip_reader_init_file(&src_zip, config->template_path, 0)) {
        set_error(result, "Cannot open template: %s", config->template_path);
        return -1;
    }

    /* ---- 创建输出 ZIP ---- */
    mz_zip_archive dst_zip;
    memset(&dst_zip, 0, sizeof(dst_zip));
    if (!mz_zip_writer_init_file(&dst_zip, config->output_path, 0)) {
        set_error(result, "Cannot create output: %s", config->output_path);
        mz_zip_reader_end(&src_zip);
        return -1;
    }

    int total_bookmarks_replaced = 0;
    int total_images_replaced = 0;
    int error_occurred = 0;

    /* ---- 遍历模板 ZIP 中的所有条目 ---- */
    mz_uint num_files = mz_zip_reader_get_num_files(&src_zip);
    mz_uint file_idx;

    for (file_idx = 0; file_idx < num_files; file_idx++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&src_zip, file_idx, &file_stat)) {
            continue;  /* 跳过无法读取的条目 */
        }

        /* 跳过目录条目 */
        if (mz_zip_reader_is_file_a_directory(&src_zip, file_idx)) {
            continue;
        }

        const char *entry_name = file_stat.m_filename;

        /* ---- 检查是否需要图片替换 ---- */
        int image_replaced = 0;
        if (config->images && config->image_count > 0) {
            uint32_t img_i;
            for (img_i = 0; img_i < config->image_count; img_i++) {
                if (strcmp(entry_name, config->images[img_i].inner_path) == 0) {
                    /* 用传入的图片数据替换 */
                    if (!mz_zip_writer_add_mem(&dst_zip, entry_name,
                                               config->images[img_i].data,
                                               config->images[img_i].data_size,
                                               MZ_DEFAULT_COMPRESSION)) {
                        set_error(result, "Failed to write image: %s", entry_name);
                        error_occurred = 1;
                        break;
                    }
                    total_images_replaced++;
                    image_replaced = 1;
                    break;
                }
            }
            if (error_occurred) break;
            if (image_replaced) continue;
        }

        /* ---- 处理 word/*.xml 文件：书签替换 ---- */
        if (IS_WORD_XML(entry_name) && config->bookmarks && config->bookmark_count > 0) {
            /* 提取 XML 到内存 */
            size_t xml_size = 0;
            void *xml_raw = mz_zip_reader_extract_to_heap(&src_zip, file_idx,
                                                           &xml_size, 0);
            if (!xml_raw) {
                set_error(result, "Failed to extract: %s", entry_name);
                error_occurred = 1;
                break;
            }

            if (xml_size > MAX_XML_SIZE) {
                free(xml_raw);
                set_error(result, "XML too large: %s (%zu bytes)", entry_name, xml_size);
                error_occurred = 1;
                break;
            }

            /* 分配可写缓冲区 (多留空间给替换后可能增长的文本) */
            size_t xml_cap = xml_size + INITIAL_BUF_SIZE;
            char *xml_buf = (char *)malloc(xml_cap);
            if (!xml_buf) {
                free(xml_raw);
                set_error(result, "Out of memory for XML buffer");
                error_occurred = 1;
                break;
            }
            memcpy(xml_buf, xml_raw, xml_size);
            xml_buf[xml_size] = '\0';
            free(xml_raw);

            size_t xml_len = xml_size;
            /* 调试：打印书签标签实际内容 */

            /* 执行书签替换 */
            int replaced = process_xml_bookmarks(&xml_buf, &xml_len, &xml_cap,
                                                  config->bookmarks,
                                                  config->bookmark_count);
            total_bookmarks_replaced += replaced;

            /* 写入输出 ZIP */
            if (!mz_zip_writer_add_mem(&dst_zip, entry_name,
                                        xml_buf, xml_len,
                                        MZ_DEFAULT_COMPRESSION)) {
                free(xml_buf);
                set_error(result, "Failed to write XML: %s", entry_name);
                error_occurred = 1;
                break;
            }
            free(xml_buf);

        } else {
            /* ---- 其余文件原样复制 ---- */
            size_t raw_size = 0;
            void *raw_data = mz_zip_reader_extract_to_heap(&src_zip, file_idx,
                                                            &raw_size, 0);
            if (!raw_data) {
                set_error(result, "Failed to extract: %s", entry_name);
                error_occurred = 1;
                break;
            }

            if (!mz_zip_writer_add_mem(&dst_zip, entry_name,
                                        raw_data, raw_size,
                                        MZ_DEFAULT_COMPRESSION)) {
                free(raw_data);
                set_error(result, "Failed to copy: %s", entry_name);
                error_occurred = 1;
                break;
            }
            free(raw_data);
        }
    }

    /* ---- 完成并关闭 ---- */
    if (!error_occurred) {
        if (!mz_zip_writer_finalize_archive(&dst_zip)) {
            set_error(result, "Failed to finalize output ZIP");
            error_occurred = 1;
        }
    }

    mz_zip_writer_end(&dst_zip);
    mz_zip_reader_end(&src_zip);

    if (error_occurred) {
        /* 清理失败的输出文件 */
        remove(config->output_path);
        return -1;
    }

    result->success = 1;
    result->bookmarks_replaced = total_bookmarks_replaced;
    result->images_replaced = total_images_replaced;
    return 0;
}

int docx_export_simple(const char *template_path, const char *output_path,
                       const bookmark_replace_t *items, uint32_t item_count)
{
    docx_export_config_t config;
    memset(&config, 0, sizeof(config));
    config.template_path  = template_path;
    config.output_path    = output_path;
    config.bookmarks      = items;
    config.bookmark_count = item_count;
    config.images         = NULL;
    config.image_count    = 0;

    return docx_export(&config, NULL);
}

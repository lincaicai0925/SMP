#ifndef DOCX_EXPORT_H
#define DOCX_EXPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/*  数据结构                                                            */
/* ------------------------------------------------------------------ */

/** 书签替换项：将模板中 w:name 匹配的书签内容替换为指定文本 */
typedef struct {
    char name[64];          /**< 书签名 (对应 Word 模板中的 w:name) */
    char text[512];         /**< 替换文本 (UTF-8，自动做 XML 转义) */
} bookmark_replace_t;

/** 图片替换项：将 ZIP 内指定路径的图片替换为传入的二进制数据 */
typedef struct {
    const char    *inner_path;   /**< zip 内路径, 如 "word/media/chart1.png" */
    const uint8_t *data;         /**< 图片二进制数据 (PNG) */
    uint32_t       data_size;    /**< 数据大小 (字节) */
} image_replace_t;

/** 导出配置 */
typedef struct {
    const char               *template_path;   /**< 模板 docx 文件路径 */
    const char               *output_path;     /**< 输出 docx 文件路径 */
    const bookmark_replace_t *bookmarks;        /**< 书签替换数组 */
    uint32_t                  bookmark_count;   /**< 书签替换数量 */
    const image_replace_t    *images;           /**< 图片替换数组 (可为 NULL) */
    uint32_t                  image_count;      /**< 图片替换数量 */
} docx_export_config_t;

/** 导出结果 */
typedef struct {
    int   success;              /**< 1=成功, 0=失败 */
    int   bookmarks_replaced;   /**< 实际替换的书签数量 */
    int   bookmarks_total;      /**< 配置中请求替换的总数 */
    int   images_replaced;      /**< 实际替换的图片数量 */
    char  error_msg[256];       /**< 失败时的错误信息 */
} docx_export_result_t;

/* ------------------------------------------------------------------ */
/*  公共接口                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief 完整导出接口 — 支持书签替换 + 图片替换
 * @param config  导出配置
 * @param result  导出结果 (可为 NULL，此时忽略详细结果)
 * @return 0=成功, -1=失败
 */
int docx_export(const docx_export_config_t *config, docx_export_result_t *result);

/**
 * @brief 简化导出接口 — 仅书签替换，无图片替换
 * @param template_path  模板路径
 * @param output_path    输出路径
 * @param items          书签替换数组
 * @param item_count     替换数量
 * @return 0=成功, -1=失败
 */
int docx_export_simple(const char *template_path, const char *output_path,
                       const bookmark_replace_t *items, uint32_t item_count);

#ifdef __cplusplus
}
#endif

#endif /* DOCX_EXPORT_H */

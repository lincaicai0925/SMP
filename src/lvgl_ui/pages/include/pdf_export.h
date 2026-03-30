#ifndef PDF_EXPORT_H
#define PDF_EXPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/*  数据结构                                                            */
/* ------------------------------------------------------------------ */

/** 键值对：报告中的一行元信息 */
typedef struct {
    char label[64];         /**< 标签名 (UTF-8) */
    char value[512];        /**< 值文本 (UTF-8) */
} pdf_field_t;

/** 数据表格行 */
typedef struct {
    char name[64];          /**< 变量名 */
    char desc[256];         /**< 描述 */
    char value[64];         /**< 当前值 */
    char unit[32];          /**< 单位 */
} pdf_table_row_t;

/** 图片数据（JPEG 格式） */
typedef struct {
    const uint8_t *data;        /**< JPEG 二进制数据 */
    uint32_t       data_size;   /**< 数据大小 (字节) */
    uint32_t       width;       /**< 原始宽度 (像素) */
    uint32_t       height;      /**< 原始高度 (像素) */
} pdf_image_t;

/** PDF 导出配置 */
typedef struct {
    const char             *output_path;    /**< 输出 PDF 文件路径 */
    const char             *title;          /**< 报告标题 */
    const char             *font_path;      /**< TTF 字体路径 (支持中文) */
    const pdf_field_t      *fields;         /**< 元信息数组 */
    uint32_t                field_count;     /**< 元信息数量 */
    const pdf_table_row_t  *rows;           /**< 数据表格行数组 */
    uint32_t                row_count;      /**< 数据行数量 */
    const pdf_image_t      *images;         /**< 图片数组 (可为 NULL) */
    uint32_t                image_count;    /**< 图片数量 */
} pdf_export_config_t;

/** PDF 导出结果 */
typedef struct {
    int   success;              /**< 1=成功, 0=失败 */
    char  error_msg[256];       /**< 失败时的错误信息 */
} pdf_export_result_t;

/* ------------------------------------------------------------------ */
/*  公共接口                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief 导出 PDF 报告
 * @param config  导出配置
 * @param result  导出结果 (可为 NULL)
 * @return 0=成功, -1=失败
 */
int Pdf_Export(const pdf_export_config_t *config, pdf_export_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* PDF_EXPORT_H */

#ifndef DICT_CONF_POPUP_H
#define DICT_CONF_POPUP_H

#include "smp_private.h"
#include "pages/include/data_dictionary.h"

/**
 * @brief  打开寄存器配置弹窗（全屏遮罩 + 居中卡片面板）
 * @param  ctx  全局上下文，用于获取图片资源等
 *
 * 用户在弹窗中编辑一条 data_dict_item_dsc_t 记录，
 * 点击"保存配置"后：
 *   1. 读取 out/conf/data_dict_conf.json
 *   2. 将新记录插入数组头部
 *   3. 写回文件
 *   4. 刷新数据字典表格
 */
void Dict_Conf_Popup_Open(smp_ctx_t *ctx, data_dict_param_cb cb);

#endif /* DICT_CONF_POPUP_H */

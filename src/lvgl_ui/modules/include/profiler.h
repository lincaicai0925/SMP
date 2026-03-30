/**
 * @file profiler.h
 * @brief LVGL 性能分析器模块 - 用于测量和记录代码执行时间
 *
 * @details
 * 此模块基于 LVGL 内置的性能分析器，提供了一个简单易用的接口来：
 * - 测量代码段的执行时间
 * - 记录线程和 CPU 信息
 * - 生成性能分析日志文件
 *
 * @section usage 使用方法
 *
 * 1. **初始化性能分析器**
 *    在程序启动时调用一次：
 *    @code
 *    My_Profiler_Init();
 *    @endcode
 *
 * 2. **在需要测量的代码段添加测量点**
 *    使用 LVGL 提供的宏来标记代码段：
 *    @code
 *    // 示例 1: 测量函数执行时间
 *    void my_function(void) {
 *        LV_PROFILER_BEGIN;  // 在函数开始处添加
 *
 *        // 你的代码...
 *        for(int i = 0; i < 1000; i++) {
 *            // 某些耗时操作
 *        }
 *
 *        LV_PROFILER_END;    // 在函数结束处添加
 *    }
 *
 *    // 示例 2: 测量代码块执行时间
 *    void process_data(void) {
 *        LV_PROFILER_BEGIN_TAG("数据处理");
 *        // 数据处理代码
 *        LV_PROFILER_END_TAG("数据处理");
 *
 *        LV_PROFILER_BEGIN_TAG("渲染");
 *        // 渲染代码
 *        LV_PROFILER_END_TAG("渲染");
 *    }
 *    @endcode
 *
 * 3. **运行程序**
 *    程序运行时，性能数据会实时写入日志文件
 *
 * 4. **查看结果**
 *    性能分析日志会保存到程序运行目录下的 **profiler_log.txt** 文件中
 *    日志包含：
 *    - 时间戳（微秒精度）
 *    - 线程 ID
 *    - CPU 核心编号
 *    - 函数名/标签
 *    - 执行时长
 *
 * @section output 输出文件位置
 *
 * **文件名**: profiler_log.txt
 * **保存路径**: 程序可执行文件所在目录
 *
 * 例如：
 * - 如果程序在 E:\cjl\SMP\build\Debug\app.exe 运行
 * - 日志文件将保存在 E:\cjl\SMP\build\Debug\profiler_log.txt
 *
 * @section notes 注意事项
 *
 * - 必须在使用测量点之前先调用 My_Profiler_Init()
 * - 每个 LV_PROFILER_BEGIN 必须有对应的 LV_PROFILER_END
 * - 测量点会带来少量性能开销，建议在性能优化时使用
 * - 文件会在每次程序启动时清空重写
 * - 日志数据会实时刷新到文件，不会因程序崩溃而丢失
 *
 * @section example 完整示例
 *
 * @code
 * #include "profiler.h"
 *
 * int main(void) {
 *     // 1. 初始化性能分析器
 *     My_Profiler_Init();
 *
 *     // 2. 在需要测量的地方添加测量点
 *     LV_PROFILER_BEGIN;
 *
 *     // 初始化 LVGL
 *     lv_init();
 *
 *     LV_PROFILER_END;
 *
 *     // 主循环
 *     while(1) {
 *         LV_PROFILER_BEGIN_TAG("主循环");
 *
 *         lv_timer_handler();
 *         lv_tick_inc(5);
 *
 *         LV_PROFILER_END_TAG("主循环");
 *
 *         Sleep(5);
 *     }
 *
 *     return 0;
 * }
 * @endcode
 *
 * @author  CJL
 * @date    2026
 * @version 1.0
 */

#ifndef PROFILER_H
#define PROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化性能分析器
 *
 * @details
 * 此函数会：
 * - 创建并打开 profiler_log.txt 日志文件（如果文件已存在会被覆盖）
 * - 配置时间戳回调函数（微秒精度）
 * - 配置线程 ID 回调函数
 * - 配置 CPU 核心编号回调函数
 * - 初始化 LVGL 内置性能分析器
 *
 * @note 必须在程序启动时调用一次，在使用任何测量点之前调用
 * @note 如果日志文件创建失败，函数会静默返回，性能分析器将不可用
 *
 * @par 调用时机
 * 建议在 main() 函数开始时、lv_init() 之前调用
 */
void My_Profiler_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* PROFILER_H */

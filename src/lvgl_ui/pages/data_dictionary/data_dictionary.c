// TODO
/*
    设置一个全局主题map，存储主题指针，根据偏移地址找到该指针，发布数据，订阅该偏移地址数据的控件，都订阅这个主题。
当总线有数据时，调用统一的回调函数，根据虚拟地址计算，数组的偏移地址，然后得到对应的主题，之后设置
该主题的数据，那么所有订阅该主题的控件都将收到数据以完成数据的更新。
*/

#include "pages/include/data_dictionary.h"
#include "pages/include/dict_conf_popup.h"
#include "modules/include/textarea.h"
#include "modules/include/button.h"
#include "modules/include/tips_box.h"
#include "modules/include/progress_bar.h"
#include "clabez/include/cthread.h"
#include "clabez/include/catomic.h"
#include "clabez/include/cfs.h"
#include "cjson/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#include <commdlg.h>
#endif

// 全局数据字典
cmap_uint32_t_data_dict_item_dsc_t* g_data_dict = NULL;

typedef struct{
    lv_obj_t* base_obj;
    uint32_t row_num;
}table_dsc_t;

static table_dsc_t g_table;

// 分页状态
static uint32_t *s_filtered_keys = NULL;        // 筛选后的 key 数组（升序）
static uint32_t  s_filtered_cnt  = 0;           // 筛选后的条目数
static uint32_t  s_cur_page      = 1;           // 当前页码（1-based）
static uint32_t  s_total_pages   = 0;           // 总页数

// 分页 UI 控件
static lv_obj_t *s_page_lbl      = NULL;        // "显示 x-y / 共 z 条"
static lv_obj_t *s_page_num_cont = NULL;        // 页码数字容器
static lv_obj_t *s_go_ta         = NULL;        // 跳转输入框
static lv_obj_t *s_filter_ta     = NULL;        // 筛选输入框

// 线程加载上下文
#define SKIP_DETAILS_SIZE 2048
typedef struct {
    char file_path[1024];
    catomic_int32 loaded_cnt;                   // 已加载条目数
    catomic_int32 total_cnt;                    // 总条目数
    catomic_int32 finished;                     // 0=运行中, 1=成功, -1=失败
    uint32_t skipped_cnt;                       // 跳过的条目数
    uint32_t parsed_cnt;                        // 已解析的条目序号
    char skip_details[SKIP_DETAILS_SIZE];       // 跳过原因详情
    int skip_details_len;                       // 已写入长度
    cthread thread;
} dict_load_ctx_t;

static dict_load_ctx_t s_load_ctx;
static progress_bar_t  s_progress_bar;
static lv_timer_t     *s_progress_timer = NULL;

LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_24);
LV_FONT_DECLARE(lv_font_founder_kaiti_simplified_16);
static smp_ctx_t* g_ctx = NULL;

lv_obj_t* Get_Table_Cell(table_dsc_t *t, uint32_t row, uint32_t col);
void Set_Table_Text(table_dsc_t *t, uint32_t row, uint32_t col, const char* txt);
static void Create_Row(lv_obj_t* base_obj);
static void Cell_Delete_Cb(lv_event_t* e);
static void Refresh_Table(lv_obj_t* base_obj, const data_dict_item_dsc_t *item);
static void Table_Move_Up(lv_event_t* e);
static void Table_Move_Top(lv_event_t* e);
static void Table_Move_Down(lv_event_t* e);
static void Table_Lable_Mode_Cb(lv_event_t* e);
static void Parse_JSON_Obj(const char* data_buf, uint32_t len, dict_load_ctx_t *ctx);
static int8_t Parse_JSON_Item(const char* json_str, data_dict_item_dsc_t *item, const char **missing_field);
static inline void _safe_strcpy(char *dst, size_t dst_size, const char *src);
static void* Dict_Load_Thread(void *arg);
static int8_t Refresh_Filtered_List(const char *filter_text);
static int8_t Rebuild_Filtered_List(const char *filter_text);
static void Render_Page(uint32_t page);
static void Update_Page_Label(void);
static void Prev_Page_Cb(lv_event_t *e);
static void Next_Page_Cb(lv_event_t *e);
static void Go_Page_Cb(lv_event_t *e);
static void Page_Num_Click_Cb(lv_event_t *e);
static void Filter_Apply_Cb(lv_event_t *e);
static void Filter_Reset_Cb(lv_event_t *e);
static void Create_Filterbar(lv_obj_t *parent);
static void Create_Pagination(lv_obj_t *parent);
static void Update_Page_Numbers(void);
static void Progress_Timer_Cb(lv_timer_t *timer);

static int _cmp_fun(const uint32_t *a, const uint32_t *b) { return (*a > *b) - (*a < *b); }

void Data_Dictionary_Weight(smp_ctx_t *ctx)
{
    g_ctx = ctx;
    lv_obj_t* activity_content = ctx->pages[NAV_PAGE_DATA_DICTIONARY]->activity_content;
    lv_obj_t* menu = ctx->pages[NAV_PAGE_DATA_DICTIONARY]->menu;
    uint8_t flag_hidden =  ctx->pages[NAV_PAGE_DATA_DICTIONARY]->cur_page_is_hidden;
    lv_obj_set_style_bg_color(activity_content, lv_color_hex(0x1a1d23), LV_PART_MAIN);

    lv_obj_set_flex_flow(activity_content, LV_FLEX_FLOW_COLUMN);
    memset(&s_load_ctx, 0, sizeof(s_load_ctx));
    // 筛选栏和加载字典按钮放在二级菜单中
    Create_Filterbar(menu);

    // 表格内容区域
    lv_obj_t* content_container = lv_obj_create(activity_content);
    lv_obj_set_flex_grow(content_container,1);
    lv_obj_set_size(content_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_set_scroll_dir(content_container, LV_DIR_VER);
    lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_user_data(content_container, ctx);
    g_table.row_num = 0;
    g_table.base_obj = content_container;

    cJSON_Hooks hook = {.malloc_fn = lv_malloc, .free_fn = lv_free};
    cJSON_InitHooks(&hook);

    // 创建标题行
    Create_Row(content_container);
    Set_Table_Text(&g_table, 0, 0, "总线地址");
    Set_Table_Text(&g_table, 0, 1, "设备名称");
    Set_Table_Text(&g_table, 0, 2, "设备描述");
    Set_Table_Text(&g_table, 0, 3, "数据类型");
    Set_Table_Text(&g_table, 0, 4, "数据长度");
    Set_Table_Text(&g_table, 0, 5, "读/写");
    Set_Table_Text(&g_table, 0, 6, "告警级别");
    Set_Table_Text(&g_table, 0, 7, "操作");
    // 更改标题行样式
    lv_obj_t *header_row = lv_obj_get_child(content_container, 0);
    lv_obj_set_style_bg_color(header_row, lv_color_hex(CLR_HEADER_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header_row, LV_OPA_COVER, LV_PART_MAIN);
    for(int j = 0; j < TABLE_COLUMN_CNT; j++)
    {
        lv_obj_set_style_bg_color(Get_Table_Cell(&g_table, 0, j), lv_color_hex(CLR_HEADER_BG), LV_PART_MAIN);
    }

    // 分页栏
    Create_Pagination(activity_content);
}


static void Progress_Timer_Cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    int32_t loaded = catomic_load_int32(&s_load_ctx.loaded_cnt, MEM_ORDER_ACQUIRE);
    int32_t total  = catomic_load_int32(&s_load_ctx.total_cnt, MEM_ORDER_ACQUIRE);
    int32_t done   = catomic_load_int32(&s_load_ctx.finished, MEM_ORDER_ACQUIRE);

    int32_t pct = 0;
    if(total > 0)
        pct = (int32_t)((int64_t)loaded * 100 / total);

    char info[512];
    lv_snprintf(info, sizeof(info), "%d / %d Byte", (int)loaded, (int)total);
    Progress_Bar_Update(&s_progress_bar, pct, info);

    if(done != 0) {
        // 加载完成或失败
        cthread_join(&s_load_ctx.thread);
        Progress_Bar_Destroy(&s_progress_bar);
        lv_timer_delete(s_progress_timer);
        s_progress_timer = NULL;

        if(done < 0) {
            Create_Tips_Box("数据字典加载失败", 0xff0000);
        } else {
            // 构建筛选列表并渲染第一页
            if(Refresh_Filtered_List(NULL) == 0)
                Render_Page(1);

            if(s_load_ctx.skipped_cnt > 0) {
                char msg[SKIP_DETAILS_SIZE + 128];
                snprintf(msg, sizeof(msg), "加载完成，跳过 %u 条异常条目:\n%s",
                    s_load_ctx.skipped_cnt, s_load_ctx.skip_details);
                Create_Tips_Box(msg, 0xffff00);
            }
        }
    }
}

/**
 * @brief 跨平台文件选择对话框
 */
static bool Open_File_Dialog(char *out_path, size_t out_len)
{
#if defined(_WIN32)
    memset(out_path, 0, out_len);
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = NULL;
    ofn.lpstrFilter  = "JSON Files (*.json)\0*.json\0";
    ofn.lpstrFile    = out_path;
    ofn.nMaxFile     = (DWORD)out_len;
    ofn.lpstrTitle   = "select the data dictionary file";
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameA(&ofn) != 0;
#else
    /* Linux: 优先 zenity, 回退 kdialog */
    FILE *fp = popen(
        "zenity --file-selection --file-filter='JSON files|*.json' --title='select the data dictionary file' 2>/dev/null "
        "|| kdialog --getopenfilename . 'JSON files (*.json)' 2>/dev/null",
        "r");
    if(!fp) return false;

    if(!fgets(out_path, (int)out_len, fp)) {
        pclose(fp);
        return false;
    }
    pclose(fp);

    /* 去除末尾换行符 */
    size_t len = strlen(out_path);
    while(len > 0 && (out_path[len - 1] == '\n' || out_path[len - 1] == '\r'))
        out_path[--len] = '\0';

    return len > 0;
#endif
}

void Load_Dict(lv_event_t* e)
{
    LV_UNUSED(e);
    if(!g_table.base_obj) return;

    if(s_progress_timer != NULL) return;

    char file_path[1024] = {0};
    if(!Open_File_Dialog(file_path, sizeof(file_path)))
        return;  /* 用户取消 */

    memset(&s_load_ctx, 0, sizeof(s_load_ctx));
    _safe_strcpy(s_load_ctx.file_path, sizeof(s_load_ctx.file_path), file_path);
    catomic_store_int32(&s_load_ctx.loaded_cnt, 0, MEM_ORDER_RELAXED);
    catomic_store_int32(&s_load_ctx.total_cnt, 0, MEM_ORDER_RELAXED);
    catomic_store_int32(&s_load_ctx.finished, 0, MEM_ORDER_RELAXED);

    s_progress_bar = Progress_Bar_Create("正在加载数据字典...");

    cthread_create(&s_load_ctx.thread, Dict_Load_Thread, &s_load_ctx);

    s_progress_timer = lv_timer_create(Progress_Timer_Cb, PROGRESS_BAR_DELAY, NULL);
}


lv_obj_t* Get_Table_Cell(table_dsc_t *t, uint32_t row, uint32_t col)
{
    if(row >= g_table.row_num || col > TABLE_COLUMN_CNT)
        return NULL;

    lv_obj_t* child = lv_obj_get_child(lv_obj_get_child(t->base_obj,row), col);

    return child;
}


void Set_Table_Text(table_dsc_t *t, uint32_t row, uint32_t col, const char* txt)
{
    lv_obj_t* cell = Get_Table_Cell(t, row, col);
    lv_obj_t* lable = lv_obj_get_child(cell, 0);
    lv_label_set_text(lable, txt);
}

static void Refresh_Table(lv_obj_t* base_obj, const data_dict_item_dsc_t *item)
{
    LV_ASSERT_MSG(item, "can not refresh data to table");
    Create_Row(base_obj);
    uint32_t r = g_table.row_num - 1;

    char tmp[32];

    // col 0: 总线地址
    snprintf(tmp, sizeof(tmp), "%u", item->bus_addr);
    Set_Table_Text(&g_table, r, 0, tmp);
    // col 1: 设备名称
    Set_Table_Text(&g_table, r, 1, item->device_name);
    // col 2: 设备描述
    Set_Table_Text(&g_table, r, 2, item->desc);
    // col 3: 数据类型
    Set_Table_Text(&g_table, r, 3, item->data_type);
    // col 4: 数据长度
    snprintf(tmp, sizeof(tmp), "%u", item->data_len);
    Set_Table_Text(&g_table, r, 4, tmp);
    // col 5: 读/写
    Set_Table_Text(&g_table, r, 5, item->rw);
    // col 6: 告警级别
    snprintf(tmp, sizeof(tmp), "%u", item->alarm_level);
    Set_Table_Text(&g_table, r, 6, tmp);
    // col 7: 操作栏 — 由 Create_Row 中 case 处理
}

static inline void Create_Lable_Cell(lv_obj_t* base_obj)
{
    lv_obj_t* label = lv_label_create(base_obj);
    lv_obj_set_size(label, lv_pct(100), TABLE_ROW_HIGHT-10);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

static void Table_Row_Operate(lv_obj_t* base_obj)
{
    lv_obj_t* btn;
    lv_obj_t* img_up;
    btn = Create_Button(base_obj, TABLE_ROW_HIGHT, lv_pct(100), "");
    lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
    lv_obj_clean(btn);
    img_up = lv_image_create(btn);
    lv_image_set_src(img_up, g_ctx->imgs[IMG_DICT_ARROW_UP]);
    lv_obj_center(img_up);
    lv_obj_add_event_cb(btn, Table_Move_Up, LV_EVENT_CLICKED, base_obj);

    btn = Create_Button(base_obj, TABLE_ROW_HIGHT, lv_pct(100), "");
    lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
    lv_obj_clean(btn);
    img_up = lv_image_create(btn);
    lv_image_set_src(img_up, g_ctx->imgs[IMG_DICT_ARROW_TOP]);
    lv_obj_center(img_up);
    lv_obj_add_event_cb(btn, Table_Move_Top, LV_EVENT_CLICKED, base_obj);

    btn = Create_Button(base_obj, TABLE_ROW_HIGHT, lv_pct(100), "");
    lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
    lv_obj_clean(btn);
    img_up = lv_image_create(btn);
    lv_image_set_src(img_up, g_ctx->imgs[IMG_DICT_ARROW_DOWN]);
    lv_obj_center(img_up);
    lv_obj_add_event_cb(btn, Table_Move_Down, LV_EVENT_CLICKED, base_obj);
}

static void Create_Row(lv_obj_t* base_obj)
{
    lv_obj_t* row = lv_obj_create(base_obj);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_style_max_height(row, TABLE_ROW_HIGHT, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(row, TABLE_BG_COLOR, LV_PART_MAIN);
    // lv_obj_set_style_border_width(row, 1, LV_PART_MAIN);
    // lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    // lv_obj_set_style_border_color(row, lv_color_hex(0xffffff), LV_PART_MAIN);
    // lv_obj_set_style_border_opa(row, LV_OPA_10, LV_PART_MAIN);
    
    for(int idx = 0; idx < TABLE_COLUMN_CNT; idx++)
    {
        lv_obj_t* r_child = lv_obj_create(row);
        lv_obj_set_size(r_child, lv_pct(100), lv_pct(100));
        lv_obj_set_flex_grow(r_child, 1);
        lv_obj_set_style_bg_opa(r_child, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_color(r_child, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_opa(r_child, LV_OPA_10, LV_PART_MAIN);
        lv_obj_set_flex_flow(r_child, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(r_child, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_remove_flag(r_child, LV_OBJ_FLAG_SCROLLABLE);

        switch (idx)
        {
            case 0: // 总线地址 — 窄列
                lv_obj_set_flex_grow(r_child, 0);
                lv_obj_set_width(r_child, 100);
                Create_Lable_Cell(r_child);
                break;

            case 1: // 设备名称 — 填充
                Create_Lable_Cell(r_child);
                break;

            case 2: // 设备描述 — 填充，按住滚动
                if(g_table.row_num != 0)
                    lv_obj_add_event_cb(r_child, Table_Lable_Mode_Cb, LV_EVENT_ALL, NULL);
                Create_Lable_Cell(r_child);
                break;

            case 3: // 数据类型 — 窄列
                lv_obj_set_flex_grow(r_child, 0);
                lv_obj_set_width(r_child, 100);
                Create_Lable_Cell(r_child);
                break;

            case 4: // 数据长度 — 窄列
                lv_obj_set_flex_grow(r_child, 0);
                lv_obj_set_width(r_child, 120);
                Create_Lable_Cell(r_child);
                break;

            case 5: // 读/写 — 窄列
                lv_obj_set_flex_grow(r_child, 0);
                lv_obj_set_width(r_child, 80);
                Create_Lable_Cell(r_child);
                break;

            case 6: // 告警级别 — 窄列
                lv_obj_set_flex_grow(r_child, 0);
                lv_obj_set_width(r_child, 120);
                Create_Lable_Cell(r_child);
                break;

            case TABLE_COLUMN_CNT - 1: // 操作栏
                lv_obj_set_flex_grow(r_child, 0);
                lv_obj_set_size(r_child, TABLE_ROW_HIGHT * 3 + 4, TABLE_ROW_HIGHT);
                if(g_table.row_num == 0) {
                    Create_Lable_Cell(r_child);
                } else {
                    lv_obj_set_flex_flow(r_child, LV_FLEX_FLOW_ROW);
                    lv_obj_set_style_pad_column(r_child, 2, LV_PART_MAIN);
                    Table_Row_Operate(r_child);
                }
                break;

            default:
                Create_Lable_Cell(r_child);
                break;
        }
    }

    g_table.row_num++;
}
static void Table_Lable_Mode_Cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_t* container = lv_event_get_target(e);
        lv_obj_t* label = lv_obj_get_child(container, 0);
        if(label == NULL) return;
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    }

    else if(code == LV_EVENT_RELEASED) {
        lv_obj_t* container = lv_event_get_target(e);
        lv_obj_t* label = lv_obj_get_child(container, 0);
        if(label == NULL) return;
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    }
}

static void Table_Move_Up(lv_event_t* e)
{
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_user_data(e);

    lv_obj_t* tar_obj = lv_obj_get_parent(obj);
    int32_t index = lv_obj_get_index(tar_obj);
    if(index != 1)
        lv_obj_move_to_index(tar_obj, index-1);
}

static void Table_Move_Top(lv_event_t* e){
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_user_data(e);
    lv_obj_t* tar_obj = lv_obj_get_parent(obj);
    int32_t index = lv_obj_get_index(tar_obj);
    if(index != 1)
        lv_obj_move_to_index(tar_obj, 1);
}

static void Table_Move_Down(lv_event_t* e)
{
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_user_data(e);
    lv_obj_t* tar_obj = lv_obj_get_parent(obj);
    int32_t index = lv_obj_get_index(tar_obj);
    if(index < lv_obj_get_child_count(lv_obj_get_parent(tar_obj)))
        lv_obj_move_to_index(tar_obj, index+1);
}

static void Cell_Delete_Cb(lv_event_t* e)
{
    uint8_t* c = (uint8_t*)lv_event_get_user_data(e);
    if(c)
    {
        lv_free(c);
    }
}

/* ========== 分页系统 ========== */
static int8_t Refresh_Filtered_List(const char *filter_text)
{    
    int8_t res = Rebuild_Filtered_List(filter_text);
    if(res == -1)
        Create_Tips_Box("还没有数据字典, 请先加载字典!", 0xff0000);
    else if(res == -2)
        Create_Tips_Box("字典中的条目为 0, 请检查文件!", 0xff0000);
    else if(res == -3)
        Create_Tips_Box("内存空间不足!", 0xff0000);
    return res;
}

static int8_t Rebuild_Filtered_List(const char *filter_text)
{
    if(s_filtered_keys) {
        lv_free(s_filtered_keys);
        s_filtered_keys = NULL;
    }
    s_filtered_cnt = 0;

    if(!g_data_dict) return -1;

    // 统计 map 中条目总数
    uint32_t map_size = cmap_uint32_t_data_dict_item_dsc_t_size(g_data_dict);
    if(map_size == 0) return -2;

    s_filtered_keys = (uint32_t *)lv_malloc(map_size * sizeof(uint32_t));
    if(!s_filtered_keys) return -3;

    // 是否有筛选条件
    bool has_filter = (filter_text && filter_text[0] != '\0');
    uint32_t filter_addr = 0;
    bool filter_by_addr = false;
    if(has_filter) {
        // 如果输入是纯数字，则按地址筛选
        char *endptr = NULL;
        unsigned long val = strtoul(filter_text, &endptr, 10);
        if(endptr && *endptr == '\0' && endptr != filter_text) {
            filter_by_addr = true;
            filter_addr = (uint32_t)val;
        }
    }

    // 中序遍历 map（按 key 升序）
    cmap_as_foreach(g_data_dict, uint32_t, data_dict_item_dsc_t, it, uint32_t_data_dict_item_dsc_t) {
        bool match = true;
        if(has_filter) {
            if(filter_by_addr) {
                match = (*it->key_ptr == filter_addr);
            } else {
                // 按设备名称或描述模糊匹配
                match = (strstr(it->value_ptr->device_name, filter_text) != NULL) ||
                        (strstr(it->value_ptr->desc, filter_text) != NULL);
            }
        }
        if(match) {
            s_filtered_keys[s_filtered_cnt++] = *it->key_ptr;
        }
    }

    s_total_pages = (s_filtered_cnt + LAZY_LOAD_BATCH_SIZE - 1) / LAZY_LOAD_BATCH_SIZE;
    if(s_total_pages == 0) s_total_pages = 1;
    return 0;
}

static void Update_Page_Label(void)
{
    if(!s_page_lbl) return;

    uint32_t start_idx = (s_cur_page - 1) * LAZY_LOAD_BATCH_SIZE;
    uint32_t end_idx   = start_idx + LAZY_LOAD_BATCH_SIZE;
    if(end_idx > s_filtered_cnt) end_idx = s_filtered_cnt;

    char buf[128];
    if(s_filtered_cnt == 0)
        lv_snprintf(buf, sizeof(buf), "显示 0 - 0 / 共 0 条");
    else
        lv_snprintf(buf, sizeof(buf), "显示 %u - %u / 共 %u 条",
            (unsigned)(start_idx + 1), (unsigned)end_idx, (unsigned)s_filtered_cnt);
    lv_label_set_text(s_page_lbl, buf);

    Update_Page_Numbers();
}

static void Update_Page_Numbers(void)
{
    if(!s_page_num_cont) return;

    // 清除旧页码按钮
    lv_obj_clean(s_page_num_cont);

    if(s_total_pages <= 1) {
        // 只有1页时显示单个页码
        lv_obj_t *b = lv_obj_create(s_page_num_cont);
        lv_obj_set_size(b, 28, 28);
        lv_obj_set_style_bg_color(b, lv_color_hex(CLR_ACCENT), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius(b, 3, LV_PART_MAIN);
        lv_obj_set_style_border_width(b, 0, LV_PART_MAIN);
        lv_obj_t *l = lv_label_create(b);
        lv_label_set_text(l, "1");
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(l, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
        lv_obj_center(l);
        return;
    }

    // 计算显示窗口：最多显示5个页码
    uint32_t win_start = 1, win_end = s_total_pages;
    if(s_total_pages > 5) {
        win_start = (s_cur_page > 3) ? (s_cur_page - 2) : 1;
        win_end   = win_start + 4;
        if(win_end > s_total_pages) {
            win_end   = s_total_pages;
            win_start = win_end - 4;
        }
    }

    // 左侧 "..."
    if(win_start > 1) {
        lv_obj_t *dot = lv_label_create(s_page_num_cont);
        lv_label_set_text(dot, "...");
        lv_obj_set_style_text_color(dot, lv_color_hex(CLR_TEXT_DIM), LV_PART_MAIN);
        lv_obj_set_style_text_font(dot, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    }

    // 页码按钮
    for(uint32_t p = win_start; p <= win_end; p++) {
        lv_obj_t *b = lv_btn_create(s_page_num_cont);
        lv_obj_remove_style_all(b);
        lv_obj_set_size(b, 28, 28);
        lv_obj_set_style_radius(b, 3, LV_PART_MAIN);

        if(p == s_cur_page) {
            lv_obj_set_style_bg_color(b, lv_color_hex(CLR_ACCENT), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(b, lv_color_hex(CLR_BG_PG), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_color(b, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
            lv_obj_set_style_border_width(b, 1, LV_PART_MAIN);
            // 按下效果
            lv_obj_set_style_bg_color(b, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
        }

        char num_buf[8];
        lv_snprintf(num_buf, sizeof(num_buf), "%u", (unsigned)p);
        lv_obj_t *l = lv_label_create(b);
        lv_label_set_text(l, num_buf);
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(l, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
        lv_obj_center(l);

        lv_obj_add_event_cb(b, Page_Num_Click_Cb, LV_EVENT_CLICKED, (void*)(uintptr_t)p);
    }

    // 右侧 "..."
    if(win_end < s_total_pages) {
        lv_obj_t *dot = lv_label_create(s_page_num_cont);
        lv_label_set_text(dot, "...");
        lv_obj_set_style_text_color(dot, lv_color_hex(CLR_TEXT_DIM), LV_PART_MAIN);
        lv_obj_set_style_text_font(dot, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    }
}

static void Page_Num_Click_Cb(lv_event_t *e)
{
    uint32_t pg = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    if(pg >= 1 && pg <= s_total_pages && pg != s_cur_page)
        Render_Page(pg);
}

static void Render_Page(uint32_t page)
{
    if(!g_table.base_obj) return;

    // 清除旧数据行（保留第 0 行标题行）
    uint32_t child_cnt = lv_obj_get_child_count(g_table.base_obj);
    for(int i = (int)child_cnt - 1; i > 0; i--)
        lv_obj_delete(lv_obj_get_child(g_table.base_obj, i));
    g_table.row_num = 1;

    if(page < 1) page = 1;
    if(page > s_total_pages) page = s_total_pages;
    s_cur_page = page;

    uint32_t start_idx = (page - 1) * LAZY_LOAD_BATCH_SIZE;
    uint32_t end_idx   = start_idx + LAZY_LOAD_BATCH_SIZE;
    if(end_idx > s_filtered_cnt) end_idx = s_filtered_cnt;

    cmap_uint32_t_data_dict_item_dsc_t_node *map_end =
        cmap_uint32_t_data_dict_item_dsc_t_end(g_data_dict);
    for(uint32_t i = start_idx; i < end_idx; i++) {
        cmap_uint32_t_data_dict_item_dsc_t_node *node =
            cmap_uint32_t_data_dict_item_dsc_t_find(g_data_dict, s_filtered_keys[i]);
        if(node != map_end) {
            Refresh_Table(g_table.base_obj, node->value_ptr);
            // 交替行颜色
            uint32_t row_idx = i - start_idx;
            uint32_t row_bg = (row_idx % 2 == 0) ? CLR_ROW_ODD : CLR_ROW_EVEN;
            lv_obj_t *row = lv_obj_get_child(g_table.base_obj, (int)(g_table.row_num - 1));
            lv_obj_set_style_bg_color(row, lv_color_hex(row_bg), LV_PART_MAIN);
        }
    }

    Update_Page_Label();

    // 滚动到顶部
    lv_obj_scroll_to_y(g_table.base_obj, 0, LV_ANIM_OFF);
}

static void Prev_Page_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(s_cur_page > 1)
        Render_Page(s_cur_page - 1);
}

static void Next_Page_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(s_cur_page < s_total_pages)
        Render_Page(s_cur_page + 1);
}

static void Go_Page_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(!s_go_ta) return;
    const char *txt = lv_textarea_get_text(s_go_ta);
    if(!txt || txt[0] == '\0') return;

    uint32_t pg = (uint32_t)atoi(txt);
    if(pg < 1) pg = 1;
    if(pg > s_total_pages) pg = s_total_pages;
    Render_Page(pg);
    lv_textarea_set_text(s_go_ta, "");
}

static void Filter_Apply_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(!s_filter_ta) return;
    const char *txt = lv_textarea_get_text(s_filter_ta);
    if(!txt || txt[0] == '\0') return;
    if(Refresh_Filtered_List(txt) == 0)
        Render_Page(1);
}

static void Filter_Reset_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(s_filter_ta)
        lv_textarea_set_text(s_filter_ta, "");

    if(Refresh_Filtered_List(NULL) == 0)
        Render_Page(1);
}

static int JSON_Escape_Str(char *dst, size_t dst_size, const char *src)
{
    size_t di = 0;
    for(const char *s = src; *s && di + 2 < dst_size; s++) {
        switch(*s) {
            case '"':  dst[di++] = '\\'; dst[di++] = '"';  break;
            case '\\': dst[di++] = '\\'; dst[di++] = '\\'; break;
            case '\n': dst[di++] = '\\'; dst[di++] = 'n';  break;
            case '\r': dst[di++] = '\\'; dst[di++] = 'r';  break;
            case '\t': dst[di++] = '\\'; dst[di++] = 't';  break;
            default:   dst[di++] = *s;                      break;
        }
    }
    dst[di] = '\0';
    return (int)di;
}

static int Item_To_JSON(char *buf, size_t buf_size, const data_dict_item_dsc_t *item)
{
    char esc_desc[DESC_MAX_LEN * 2];
    char esc_name[DEVICE_NAME_MAX_LEN * 2];
    char esc_maddr[MODBUS_ADDR_MAX_LEN * 2];
    char esc_dtype[DATA_TYPW_MAX_LEN * 2];
    char esc_rw[RW_MAX_LEN * 2];

    JSON_Escape_Str(esc_desc,  sizeof(esc_desc),  item->desc);
    JSON_Escape_Str(esc_name,  sizeof(esc_name),  item->device_name);
    JSON_Escape_Str(esc_maddr, sizeof(esc_maddr), item->modbus_addr);
    JSON_Escape_Str(esc_dtype, sizeof(esc_dtype), item->data_type);
    JSON_Escape_Str(esc_rw,    sizeof(esc_rw),    item->rw);

    int len = snprintf(buf, buf_size,
        "{\n\t\"bus_addr\":%u,\n\t\"device_name\":\"%s\",\n\t\"desc\":\"%s\",\n"
        "\t\"modbus_addr\":\"%s\",\n\t\"data_type\":\"%s\",\n\t\"data_len\":%u,\n"
        "\t\"rw\":\"%s\",\n\t\"thmin\":%.6g,\n\t\"thmax\":%.6g,\n"
        "\t\"alarm_level\":%u,\n\t\"default_value\":%.6g,\n"
        "\t\"decimal\":%u,\n\t\"subtype\":%u,\n\t\"bit_offset\":%u\n}",
        item->bus_addr, esc_name, esc_desc,
        esc_maddr, esc_dtype, item->data_len,
        esc_rw, item->thmin, item->thmax,
        item->alarm_level, item->default_value,
        item->decimal, item->subtype, item->bit_offset);

    return len;
}

static int8_t Write_Item_To_File(const char *file_path, const data_dict_item_dsc_t *item)
{
    char json_item[JSON_BUFFER_SIZE];
    Item_To_JSON(json_item, sizeof(json_item), item);

    FILE *fp = fopen(file_path, "rb+");
    if(fp) {
        /* 从文件末尾往前找到 ']' 的位置 */
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        long pos = file_size - 1;
        int ch;
        while(pos >= 0) {
            fseek(fp, pos, SEEK_SET);
            ch = fgetc(fp);
            if(ch == ']') break;
            pos--;
        }
        if(pos < 0) {
            /* 文件内容无效，重新写 */
            fclose(fp);
            fp = fopen(file_path, "wb");
            if(!fp) return -1;
            fprintf(fp, "[\n%s\n]", json_item);
            fclose(fp);
            return 0;
        }
        /* 在 ']' 位置覆写：追加 ,\n{新条目}\n] */
        fseek(fp, pos, SEEK_SET);
        fprintf(fp, ",\n%s\n]", json_item);
        fclose(fp);
    } else {
        /* 新文件 */
        fp = fopen(file_path, "wb");
        if(!fp) return -1;
        fprintf(fp, "[\n%s\n]", json_item);
        fclose(fp);
    }

    return 0;
}

static int8_t Save_Conf(const data_dict_item_dsc_t* arg)
{
    if(s_load_ctx.file_path[0] == '\0' || g_data_dict == NULL)
    {
        /* 初始化 g_data_dict */
        if(!g_data_dict)
            g_data_dict = cmap_uint32_t_data_dict_item_dsc_t_create_with_cmp(_cmp_fun);

        cmap_uint32_t_data_dict_item_dsc_t_node *node = cmap_uint32_t_data_dict_item_dsc_t_find(g_data_dict, arg->bus_addr);
        if(node != cmap_uint32_t_data_dict_item_dsc_t_end(g_data_dict)) {
            Create_Tips_Box("该总线地址已被占用无法添加", 0xff0000);
            return -2;
        }

        /* 确保目录存在 */
        cfs_create_dir("./conf");

        const char *path = "./conf/data_dict_conf.json";
        if(Write_Item_To_File(path, arg) != 0) {
            Create_Tips_Box("写入文件失败", 0xff0000);
            return -1;
        }

        cmap_uint32_t_data_dict_item_dsc_t_insert_or_assign(g_data_dict, arg->bus_addr, *arg);
        _safe_strcpy(s_load_ctx.file_path, sizeof(s_load_ctx.file_path), path);

        Refresh_Table(g_table.base_obj, arg);
        Create_Tips_Box("已写入默认字典文件 ./conf/data_dict_conf.json ", 0x22C55E);
        return 0;
    }

    cmap_uint32_t_data_dict_item_dsc_t_node *node = cmap_uint32_t_data_dict_item_dsc_t_find(g_data_dict, arg->bus_addr);
    if (node != cmap_uint32_t_data_dict_item_dsc_t_end(g_data_dict)) {
        Create_Tips_Box("该总线地址已被占用无法添加", 0xff0000);
        return -2;
    }

    if(Write_Item_To_File(s_load_ctx.file_path, arg) != 0) {
        Create_Tips_Box("写入文件失败", 0xff0000);
        return -1;
    }

    cmap_uint32_t_data_dict_item_dsc_t_insert_or_assign(g_data_dict, arg->bus_addr, *arg);

    Refresh_Table(g_table.base_obj, arg);
    lv_obj_t * ele = lv_obj_get_child(g_table.base_obj, -1);
    if(ele != NULL)
        lv_obj_move_to_index(ele, 1);
    Create_Tips_Box("配置已加入并写入文件中", 0x22C55E);
    return 0;
}

static void Add_Conf_Cb(lv_event_t *e)
{
    LV_UNUSED(e);
    Dict_Conf_Popup_Open(g_ctx, Save_Conf);
}

static void Create_Filterbar(lv_obj_t *menu)
{
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(menu, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(menu, 10, LV_PART_MAIN);

    /* 添加配置按钮 */
    lv_obj_t *add_btn = lv_btn_create(menu);
    lv_obj_remove_style_all(add_btn);
    lv_obj_set_size(add_btn, lv_pct(100), 36);
    lv_obj_set_style_bg_opa(add_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(add_btn, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(add_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(add_btn, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_color(add_btn, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(add_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *al = lv_label_create(add_btn);
    lv_label_set_text(al, "添加配置");
    lv_obj_set_style_text_color(al, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(al, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);
    lv_obj_center(al);
    lv_obj_add_event_cb(add_btn, Add_Conf_Cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *load_btn = lv_btn_create(menu);
    lv_obj_remove_style_all(load_btn);
    lv_obj_set_size(load_btn, lv_pct(100), 36);
    lv_obj_set_style_bg_opa(load_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(load_btn, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(load_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(load_btn, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_color(load_btn, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(load_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *ll = lv_label_create(load_btn);
    lv_label_set_text(ll, "加载字典");
    lv_obj_set_style_text_color(ll, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ll, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);
    lv_obj_center(ll);
    lv_obj_add_event_cb(load_btn, Load_Dict, LV_EVENT_CLICKED, NULL);

    lv_obj_t *sep1 = lv_obj_create(menu);
    lv_obj_set_size(sep1, lv_pct(100), 1);
    lv_obj_set_style_bg_color(sep1, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sep1, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(sep1, 0, LV_PART_MAIN);

    lv_obj_t *lbl_row = lv_obj_create(menu);
    lv_obj_set_size(lbl_row, lv_pct(100), 32);
    lv_obj_set_style_bg_opa(lbl_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(lbl_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(lbl_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(lbl_row, 6, LV_PART_MAIN);
    lv_obj_set_flex_flow(lbl_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(lbl_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(lbl_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *search_icon = lv_obj_create(lbl_row);
    lv_obj_set_style_bg_opa(search_icon, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_size(search_icon, 24, 24);
    lv_obj_set_style_bg_image_src(search_icon, g_ctx->imgs[IMG_DICT_SEARCH], LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(lbl_row);
    lv_label_set_text(lbl, "筛选");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xA0A8B4), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);

    s_filter_ta = Create_Textarea(menu, lv_pct(100), 32, "地址/设备名/描述");

    /* ---- 搜索 / 重置按钮行 ---- */
    lv_obj_t *btn_row = lv_obj_create(menu);
    lv_obj_set_size(btn_row, lv_pct(100), 34);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_row, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(btn_row, 8, LV_PART_MAIN);
    lv_obj_remove_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    // 搜索按钮
    lv_obj_t *search_btn = lv_btn_create(btn_row);
    lv_obj_remove_style_all(search_btn);
    lv_obj_set_size(search_btn, lv_pct(48), 32);
    lv_obj_set_style_bg_opa(search_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(search_btn, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(search_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(search_btn, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_color(search_btn, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(search_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *sl = lv_label_create(search_btn);
    lv_label_set_text(sl, "搜索");
    lv_obj_set_style_text_color(sl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(sl, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);
    lv_obj_center(sl);
    lv_obj_add_event_cb(search_btn, Filter_Apply_Cb, LV_EVENT_CLICKED, NULL);

    // 重置按钮
    lv_obj_t *reset_btn = lv_btn_create(btn_row);
    lv_obj_remove_style_all(reset_btn);
    lv_obj_set_size(reset_btn, lv_pct(48), 32);
    lv_obj_set_style_bg_opa(reset_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(reset_btn, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(reset_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(reset_btn, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(reset_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *rl = lv_label_create(reset_btn);
    lv_label_set_text(rl, "重置");
    lv_obj_set_style_text_color(rl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(rl, &lv_font_founder_kaiti_simplified_24, LV_PART_MAIN);
    lv_obj_center(rl);
    lv_obj_add_event_cb(reset_btn, Filter_Reset_Cb, LV_EVENT_CLICKED, NULL);
}

static void Create_Pagination(lv_obj_t *parent)
{
    lv_obj_t *pg = lv_obj_create(parent);
    lv_obj_set_size(pg, LV_PCT(100), PAGER_H);
    lv_obj_set_flex_grow(pg, 0);
    lv_obj_set_style_bg_color(pg, lv_color_hex(CLR_PANEL), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_flex_flow(pg, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(pg, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_right(pg, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(pg, 8, LV_PART_MAIN);
    lv_obj_remove_flag(pg, LV_OBJ_FLAG_SCROLLABLE);

    s_page_lbl = lv_label_create(pg);
    lv_label_set_text(s_page_lbl, "显示 0 - 0 / 共 0 条");
    lv_obj_set_style_text_color(s_page_lbl, lv_color_hex(CLR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_page_lbl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);

    lv_obj_t *sp = lv_obj_create(pg);
    lv_obj_set_flex_grow(sp, 1);
    lv_obj_set_height(sp, 1);
    lv_obj_set_style_bg_opa(sp, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sp, 0, LV_PART_MAIN);

    lv_obj_t *prev = lv_btn_create(pg);
    lv_obj_remove_style_all(prev);
    lv_obj_set_size(prev, 28, 28);
    lv_obj_set_style_bg_color(prev, lv_color_hex(CLR_BG_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(prev, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(prev, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_border_width(prev, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(prev, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_color(prev, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *pl = lv_label_create(prev);
    lv_label_set_text(pl, "<");
    lv_obj_set_style_text_color(pl, lv_color_hex(CLR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(pl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(pl);
    lv_obj_add_event_cb(prev, Prev_Page_Cb, LV_EVENT_CLICKED, NULL);

    s_page_num_cont = lv_obj_create(pg);
    lv_obj_set_size(s_page_num_cont, LV_SIZE_CONTENT, 28);
    lv_obj_set_style_bg_opa(s_page_num_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_page_num_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_page_num_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(s_page_num_cont, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(s_page_num_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_page_num_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(s_page_num_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *nxt = lv_button_create(pg);
    lv_obj_remove_style_all(nxt);
    lv_obj_set_size(nxt, 28, 28);
    lv_obj_set_style_bg_color(nxt, lv_color_hex(CLR_BG_PG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(nxt, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(nxt, lv_color_hex(CLR_BORDER_PG), LV_PART_MAIN);
    lv_obj_set_style_border_width(nxt, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(nxt, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_color(nxt, lv_color_hex(CLR_ACCENT), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *nl = lv_label_create(nxt);
    lv_label_set_text(nl, ">");
    lv_obj_set_style_text_color(nl, lv_color_hex(CLR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(nl, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(nl);
    lv_obj_add_event_cb(nxt, Next_Page_Cb, LV_EVENT_CLICKED, NULL);

    s_go_ta = Create_Textarea(pg, 60, 28, "页码");
    lv_textarea_set_accepted_chars(s_go_ta, "0123456789");
    lv_obj_set_style_radius(s_go_ta, 8, LV_PART_MAIN);

    lv_obj_t *go_btn = lv_btn_create(pg);
    lv_obj_remove_style_all(go_btn);
    lv_obj_set_size(go_btn, 40, 28);
    lv_obj_set_style_bg_color(go_btn, lv_color_hex(CLR_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(go_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(go_btn, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_color(go_btn, lv_color_hex(CLR_ACCENT_DARK), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *go_l = lv_label_create(go_btn);
    lv_label_set_text(go_l, "跳转");
    lv_obj_set_style_text_color(go_l, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(go_l, &lv_font_founder_kaiti_simplified_16, LV_PART_MAIN);
    lv_obj_center(go_l);
    lv_obj_add_event_cb(go_btn, Go_Page_Cb, LV_EVENT_CLICKED, NULL);
}

/*  解析相关函数  */

static inline void _safe_strcpy(char *dst, size_t dst_size, const char *src)
{
    if(!src) { dst[0] = '\0'; return; }
    size_t len = strlen(src);
    if(len >= dst_size) len = dst_size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static int8_t Parse_JSON_Item(const char* json_str, data_dict_item_dsc_t *item, const char **missing_field)
{
    if(!json_str || !item) return -1;
    memset(item, 0, sizeof(*item));
    if(missing_field) *missing_field = NULL;

    cJSON *json = cJSON_Parse(json_str);
    if(!json) {
        if(missing_field) *missing_field = "JSON";
        return -1;
    }

    /* 检查所有必要字段是否存在 */
    static const char * const str_fields[] = {
        "desc", "device_name", "modbus_addr", "data_type", "rw"
    };
    static const char * const num_fields[] = {
        "bus_addr", "data_len", "thmin", "thmax",
        "alarm_level", "default_value", "decimal", "subtype", "bit_offset"
    };

    for(int i = 0; i < (int)(sizeof(str_fields)/sizeof(str_fields[0])); i++) {
        if(!cJSON_GetObjectItem(json, str_fields[i])) {
            if(missing_field) *missing_field = str_fields[i];
            cJSON_Delete(json);
            return -1;
        }
    }
    for(int i = 0; i < (int)(sizeof(num_fields)/sizeof(num_fields[0])); i++) {
        if(!cJSON_GetObjectItem(json, num_fields[i])) {
            if(missing_field) *missing_field = num_fields[i];
            cJSON_Delete(json);
            return -1;
        }
    }

    cJSON *j;

    j = cJSON_GetObjectItem(json, "desc");
    _safe_strcpy(item->desc, DESC_MAX_LEN, j->valuestring);

    j = cJSON_GetObjectItem(json, "device_name");
    _safe_strcpy(item->device_name, DEVICE_NAME_MAX_LEN, j->valuestring);

    j = cJSON_GetObjectItem(json, "modbus_addr");
    _safe_strcpy(item->modbus_addr, MODBUS_ADDR_MAX_LEN, j->valuestring);

    j = cJSON_GetObjectItem(json, "data_type");
    _safe_strcpy(item->data_type, DATA_TYPW_MAX_LEN, j->valuestring);

    j = cJSON_GetObjectItem(json, "rw");
    _safe_strcpy(item->rw, RW_MAX_LEN, j->valuestring);

    j = cJSON_GetObjectItem(json, "bus_addr");
    item->bus_addr = (uint32_t)j->valueint;

    j = cJSON_GetObjectItem(json, "data_len");
    item->data_len = (uint32_t)j->valueint;

    // thmin/thmax 可能是字符串或数字
    j = cJSON_GetObjectItem(json, "thmin");
    if(cJSON_IsNumber(j))
        item->thmin = j->valuedouble;
    else if(cJSON_IsString(j) && j->valuestring)
        item->thmin = strtod(j->valuestring, NULL);

    j = cJSON_GetObjectItem(json, "thmax");
    if(cJSON_IsNumber(j))
        item->thmax = j->valuedouble;
    else if(cJSON_IsString(j) && j->valuestring)
        item->thmax = strtod(j->valuestring, NULL);

    j = cJSON_GetObjectItem(json, "alarm_level");
    if(cJSON_IsNumber(j))
        item->alarm_level = (uint32_t)j->valueint;
    else if(cJSON_IsString(j) && j->valuestring)
        item->alarm_level = (uint32_t)strtoul(j->valuestring, NULL, 10);

    j = cJSON_GetObjectItem(json, "default_value");
    item->default_value = cJSON_IsNumber(j) ? j->valuedouble : 0.0;

    j = cJSON_GetObjectItem(json, "decimal");
    item->decimal = (uint32_t)j->valueint;

    j = cJSON_GetObjectItem(json, "subtype");
    item->subtype = (uint32_t)j->valueint;

    j = cJSON_GetObjectItem(json, "bit_offset");
    item->bit_offset = (uint32_t)j->valueint;

    cJSON_Delete(json);
    return 0;
}

static void Parse_JSON_Obj(const char* data_buf, uint32_t len, dict_load_ctx_t *ctx)
{
    // 注意：一个对象的大小不要大于prase_buf的大小，否则会导致缓冲区溢出，而造成程序崩溃
    static char prase_buf[JSON_BUFFER_SIZE+1];
    static uint8_t incomplete_fram_flag = 0;
    static int idx = 0;
    const char *ptr = data_buf;
    int i = 0;
    for(;*ptr != '\0' && i < (int)len; ptr++, i++)
    {
        if(*ptr == '{' || incomplete_fram_flag)
        {
            // 将整个对象读取到解析缓冲区中等待解析
            while(*ptr != '}' && idx < JSON_BUFFER_SIZE - 3)
            {
                // 如果这时候碰到了缓冲区结尾，那么这是这个不完整的对象，剩余数据需要下一次读取
                if(*ptr == '\0')
                {
                    incomplete_fram_flag = 1;
                    return;
                }
                prase_buf[idx++] = *ptr++;
            }
            // 添加结尾标志和空终止符
            prase_buf[idx]   = '}';
            prase_buf[idx+1] = '\0';

            ctx->parsed_cnt++;
            data_dict_item_dsc_t item;
            // 常量指针他将直接指向出错的字段名称（如果存在字段错误的话）
            const char *missing = NULL;
            if(Parse_JSON_Item(prase_buf, &item, &missing) == 0) {
                cmap_uint32_t_data_dict_item_dsc_t_insert_or_assign(g_data_dict, item.bus_addr, item);
            } else {
                ctx->skipped_cnt++;
                // 缓冲区未满时追加详情
                int remain = SKIP_DETAILS_SIZE - ctx->skip_details_len - 1;
                if(remain > 0) {
                    int n = snprintf(ctx->skip_details + ctx->skip_details_len, remain,
                        "第%u条: %s%s\n",
                        ctx->parsed_cnt,
                        missing ? "缺少字段 " : "",
                        missing ? missing : "未知错误");
                    if(n > 0 && n < remain)
                        ctx->skip_details_len += n;
                    else
                        ctx->skip_details_len = SKIP_DETAILS_SIZE - 1; // 标记已满
                }
            }

            // 更新已加载计数
            // 方括号 以及逗号并没有被记录，所以实际值并不会达到文件大小这么多
            catomic_fetch_add_int32(&ctx->loaded_cnt, strnlen(prase_buf, JSON_BUFFER_SIZE+1), MEM_ORDER_RELEASE);

            idx = 0;
            lv_memset(prase_buf, 0, JSON_BUFFER_SIZE);
            // 若不完整帧被置1需要重设为0
            if(incomplete_fram_flag)    incomplete_fram_flag = 0;
        }
    }
}

static void* Dict_Load_Thread(void *arg)
{
    dict_load_ctx_t *ctx = (dict_load_ctx_t *)arg;

    FILE *fp = fopen(ctx->file_path, "rb");
    if(!fp) {
        catomic_store_int32(&ctx->finished, -1, MEM_ORDER_RELEASE);
        return NULL;
    }

    // 获取文件大小，用于估算总条目数
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if(file_size <= 0) {
        fclose(fp);
        catomic_store_int32(&ctx->finished, -1, MEM_ORDER_RELEASE);
        return NULL;
    }

    catomic_store_int32(&ctx->total_cnt, file_size, MEM_ORDER_RELEASE);

    /* 使用标准 malloc 解析（lv_malloc 可能非线程安全）。
     * 注意：cJSON_InitHooks 修改全局状态，此线程运行期间主线程不应调用 cJSON API，
     * 否则会导致分配器不匹配（malloc 分配的内存被 lv_free 释放，或反之）。 */
    cJSON_Hooks std_hooks = {.malloc_fn = malloc, .free_fn = free};
    cJSON_InitHooks(&std_hooks);

    // 清空旧 map 并重建
    if(g_data_dict) {
        cmap_uint32_t_data_dict_item_dsc_t_destroy(&g_data_dict);
    }
    g_data_dict = cmap_uint32_t_data_dict_item_dsc_t_create_with_cmp(_cmp_fun);

    char data_buf[JSON_BUFFER_SIZE];
    size_t buf_len = sizeof(data_buf) - 1;
    size_t br;

    do {
        br = fread(data_buf, 1, buf_len, fp);
        if(br > 0) {
            data_buf[br] = '\0';
            Parse_JSON_Obj(data_buf, (uint32_t)br, ctx);
        }

        cthread_timespec t = {.tv_sec = 0, .tv_nsec = (PROGRESS_BAR_DELAY * 1000000)};
        cthread_sleep_for(&t);
    } while(br >= buf_len);

    fclose(fp);

    /* 恢复 lv_malloc/lv_free 给 cJSON，确保此前所有 cJSON 对象已通过 cJSON_Delete 释放 */
    cJSON_Hooks lv_hooks = {.malloc_fn = lv_malloc, .free_fn = lv_free};
    cJSON_InitHooks(&lv_hooks);

    catomic_store_int32(&ctx->finished, 1, MEM_ORDER_RELEASE);
    return NULL;
}


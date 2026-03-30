#ifndef C_CORE_CSTRING_H_
#define C_CORE_CSTRING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef CC_EXPORTS
#ifdef CC_STATIC
#define CC_API
#else
#define CC_API __declspec(dllimport)
#endif
#else
#define CC_API __declspec(dllexport)
#endif
#define CC_CALL __cdecl
#elif defined(__unix) || defined(__linux)
#ifndef CC_API
#define CC_API __attribute__((visibility("default")))
#endif
#define CC_CALL
#else
#define CC_CALL
#define CC_API
#endif

/** \addtogroup cstring 现代C字符串库（对标C++23 std::string）
 * @{
 */

/* ============================================================================
 * 数据结构定义
 * ============================================================================ */

/**
 * @brief cstr - ANSI/ASCII字符串（对标std::string）
 * 使用SSO（Small String Optimization）优化小字符串
 */
typedef struct cstr {
    char *data;           /* 字符串数据指针 */
    size_t length;        /* 字符串长度（不包括'\0'） */
    size_t capacity;      /* 已分配的容量 */
    char sso_buffer[32];  /* SSO缓冲区，优化小字符串 */
} cstr;

/**
 * @brief cstrw - 宽字符串（对标std::wstring）
 * Windows上为UCS-2，Linux上为UTF-32
 */
typedef struct cstrw {
    wchar_t *data;
    size_t length;
    size_t capacity;
    wchar_t sso_buffer[16];
} cstrw;

/**
 * @brief cstr8 - UTF-8字符串（对标std::u8string）
 * 支持完整的UTF-8字符操作
 */
typedef struct cstr8 {
    char *data;
    size_t length;        /* 字节长度 */
    size_t char_count;    /* UTF-8字符数量 */
    size_t capacity;
    char sso_buffer[32];
} cstr8;

/**
 * @brief 字符串视图，不拥有内存（对标std::string_view）
 */
typedef struct cstr_view {
    const char *data;
    size_t length;
} cstr_view;

typedef struct cstrw_view {
    const wchar_t *data;
    size_t length;
} cstrw_view;

typedef struct cstr8_view {
    const char *data;
    size_t length;
} cstr8_view;

/**
 * @brief 编码类型
 */
typedef enum cstring_encoding {
    CSTRING_ENC_UNKNOWN = 0,
    CSTRING_ENC_ASCII = 1,
    CSTRING_ENC_UTF8 = 2,
    CSTRING_ENC_UTF16LE = 3,
    CSTRING_ENC_UTF16BE = 4,
    CSTRING_ENC_UTF32LE = 5,
    CSTRING_ENC_UTF32BE = 6,
    CSTRING_ENC_GBK = 7,
    CSTRING_ENC_BIG5 = 8
} cstring_encoding;

/**
 * @brief 字符串查找结果
 */
#define CSTR_NPOS ((size_t)-1)

/* ============================================================================
 * cstr - ANSI字符串接口
 * ============================================================================ */

/* --- 构造/析构 --- */
CC_API cstr* CC_CALL cstr_create(void);
CC_API cstr* CC_CALL cstr_create_from_cstr(const char *str);
CC_API cstr* CC_CALL cstr_create_from_buffer(const char *buffer, size_t len);
CC_API cstr* CC_CALL cstr_create_from_cstr_n(const char *str, size_t n);
CC_API cstr* CC_CALL cstr_create_with_capacity(size_t capacity);
CC_API cstr* CC_CALL cstr_create_fill(size_t count, char c);
CC_API cstr* CC_CALL cstr_clone(const cstr *src);
CC_API void CC_CALL cstr_destroy(cstr **pstr);

/* 栈上对象初始化 */
CC_API int CC_CALL cstr_init(cstr *str);
CC_API int CC_CALL cstr_init_from_cstr(cstr *str, const char *s);
CC_API void CC_CALL cstr_uninit(cstr *str);

/* --- 容量操作 --- */
CC_API size_t CC_CALL cstr_length(const cstr *str);
CC_API size_t CC_CALL cstr_size(const cstr *str);
CC_API size_t CC_CALL cstr_capacity(const cstr *str);
CC_API int CC_CALL cstr_empty(const cstr *str);
CC_API int CC_CALL cstr_reserve(cstr *str, size_t new_capacity);
CC_API int CC_CALL cstr_shrink_to_fit(cstr *str);
CC_API void CC_CALL cstr_clear(cstr *str);
CC_API int CC_CALL cstr_resize(cstr *str, size_t new_size, char fill_char);

/* --- 元素访问 --- */
CC_API char CC_CALL cstr_at(const cstr *str, size_t pos);
CC_API char* CC_CALL cstr_at_ptr(cstr *str, size_t pos);
CC_API const char* CC_CALL cstr_data(const cstr *str);
CC_API const char* CC_CALL cstr_c_str(const cstr *str);
CC_API char CC_CALL cstr_front(const cstr *str);
CC_API char CC_CALL cstr_back(const cstr *str);

/* --- 修改操作 --- */
CC_API int CC_CALL cstr_assign(cstr *str, const cstr *src);
CC_API int CC_CALL cstr_assign_cstr(cstr *str, const char *s);
CC_API int CC_CALL cstr_assign_buffer(cstr *str, const char *buffer, size_t len);
CC_API int CC_CALL cstr_assign_fill(cstr *str, size_t count, char c);

CC_API int CC_CALL cstr_append(cstr *str, const cstr *src);
CC_API int CC_CALL cstr_append_cstr(cstr *str, const char *s);
CC_API int CC_CALL cstr_append_buffer(cstr *str, const char *buffer, size_t len);
CC_API int CC_CALL cstr_append_char(cstr *str, char c);
CC_API int CC_CALL cstr_append_fill(cstr *str, size_t count, char c);

CC_API int CC_CALL cstr_insert(cstr *str, size_t pos, const cstr *src);
CC_API int CC_CALL cstr_insert_cstr(cstr *str, size_t pos, const char *s);
CC_API int CC_CALL cstr_insert_buffer(cstr *str, size_t pos, const char *buffer, size_t len);
CC_API int CC_CALL cstr_insert_char(cstr *str, size_t pos, char c);
CC_API int CC_CALL cstr_insert_fill(cstr *str, size_t pos, size_t count, char c);

CC_API int CC_CALL cstr_erase(cstr *str, size_t pos, size_t len);
CC_API void CC_CALL cstr_pop_back(cstr *str);
CC_API int CC_CALL cstr_push_back(cstr *str, char c);

CC_API int CC_CALL cstr_replace(cstr *str, size_t pos, size_t len, const cstr *src);
CC_API int CC_CALL cstr_replace_cstr(cstr *str, size_t pos, size_t len, const char *s);
CC_API int CC_CALL cstr_replace_buffer(cstr *str, size_t pos, size_t len, const char *buffer, size_t buffer_len);
CC_API int CC_CALL cstr_replace_all(cstr *str, const char *old_str, const char *new_str);

CC_API void CC_CALL cstr_swap(cstr *str1, cstr *str2);

/* --- 查找操作 --- */
CC_API size_t CC_CALL cstr_find(const cstr *str, const cstr *substr, size_t pos);
CC_API size_t CC_CALL cstr_find_cstr(const cstr *str, const char *substr, size_t pos);
CC_API size_t CC_CALL cstr_find_char(const cstr *str, char c, size_t pos);

CC_API size_t CC_CALL cstr_rfind(const cstr *str, const cstr *substr, size_t pos);
CC_API size_t CC_CALL cstr_rfind_cstr(const cstr *str, const char *substr, size_t pos);
CC_API size_t CC_CALL cstr_rfind_char(const cstr *str, char c, size_t pos);

CC_API size_t CC_CALL cstr_find_first_of(const cstr *str, const char *char_set, size_t pos);
CC_API size_t CC_CALL cstr_find_last_of(const cstr *str, const char *char_set, size_t pos);
CC_API size_t CC_CALL cstr_find_first_not_of(const cstr *str, const char *char_set, size_t pos);
CC_API size_t CC_CALL cstr_find_last_not_of(const cstr *str, const char *char_set, size_t pos);

/* --- 比较操作 --- */
CC_API int CC_CALL cstr_compare(const cstr *str1, const cstr *str2);
CC_API int CC_CALL cstr_compare_cstr(const cstr *str, const char *s);
CC_API int CC_CALL cstr_compare_n(const cstr *str1, const cstr *str2, size_t n);
CC_API int CC_CALL cstr_equals(const cstr *str1, const cstr *str2);
CC_API int CC_CALL cstr_equals_cstr(const cstr *str, const char *s);

/* --- C++20/23 新增接口 --- */
CC_API int CC_CALL cstr_starts_with(const cstr *str, const char *prefix);
CC_API int CC_CALL cstr_starts_with_char(const cstr *str, char c);
CC_API int CC_CALL cstr_ends_with(const cstr *str, const char *suffix);
CC_API int CC_CALL cstr_ends_with_char(const cstr *str, char c);
CC_API int CC_CALL cstr_contains(const cstr *str, const char *substr);
CC_API int CC_CALL cstr_contains_char(const cstr *str, char c);

/* --- 子字符串操作 --- */
CC_API cstr* CC_CALL cstr_substr(const cstr *str, size_t pos, size_t len);
CC_API cstr_view CC_CALL cstr_substr_view(const cstr *str, size_t pos, size_t len);

/* --- 字符串转换 --- */
CC_API void CC_CALL cstr_to_upper(cstr *str);
CC_API void CC_CALL cstr_to_lower(cstr *str);
CC_API cstr* CC_CALL cstr_to_upper_copy(const cstr *str);
CC_API cstr* CC_CALL cstr_to_lower_copy(const cstr *str);

CC_API void CC_CALL cstr_trim(cstr *str);
CC_API void CC_CALL cstr_trim_left(cstr *str);
CC_API void CC_CALL cstr_trim_right(cstr *str);
CC_API cstr* CC_CALL cstr_trim_copy(const cstr *str);

CC_API void CC_CALL cstr_reverse(cstr *str);
CC_API cstr* CC_CALL cstr_reverse_copy(const cstr *str);

/* --- 格式化 --- */
CC_API int CC_CALL cstr_format(cstr *str, const char *fmt, ...);
CC_API cstr* CC_CALL cstr_format_new(const char *fmt, ...);

/* ============================================================================
 * cstrw - 宽字符串接口（与cstr接口对应）
 * ============================================================================ */

/* --- 构造/析构 --- */
CC_API cstrw* CC_CALL cstrw_create(void);
CC_API cstrw* CC_CALL cstrw_create_from_wcs(const wchar_t *str);
CC_API cstrw* CC_CALL cstrw_create_from_buffer(const wchar_t *buffer, size_t len);
CC_API cstrw* CC_CALL cstrw_create_with_capacity(size_t capacity);
CC_API cstrw* CC_CALL cstrw_create_fill(size_t count, wchar_t c);
CC_API cstrw* CC_CALL cstrw_clone(const cstrw *src);
CC_API void CC_CALL cstrw_destroy(cstrw **pstr);

CC_API int CC_CALL cstrw_init(cstrw *str);
CC_API int CC_CALL cstrw_init_from_wcs(cstrw *str, const wchar_t *s);
CC_API void CC_CALL cstrw_uninit(cstrw *str);

/* --- 容量操作 --- */
CC_API size_t CC_CALL cstrw_length(const cstrw *str);
CC_API size_t CC_CALL cstrw_size(const cstrw *str);
CC_API size_t CC_CALL cstrw_capacity(const cstrw *str);
CC_API int CC_CALL cstrw_empty(const cstrw *str);
CC_API int CC_CALL cstrw_reserve(cstrw *str, size_t new_capacity);
CC_API int CC_CALL cstrw_shrink_to_fit(cstrw *str);
CC_API void CC_CALL cstrw_clear(cstrw *str);
CC_API int CC_CALL cstrw_resize(cstrw *str, size_t new_size, wchar_t fill_char);

/* --- 元素访问 --- */
CC_API wchar_t CC_CALL cstrw_at(const cstrw *str, size_t pos);
CC_API wchar_t* CC_CALL cstrw_at_ptr(cstrw *str, size_t pos);
CC_API const wchar_t* CC_CALL cstrw_data(const cstrw *str);
CC_API const wchar_t* CC_CALL cstrw_c_str(const cstrw *str);
CC_API wchar_t CC_CALL cstrw_front(const cstrw *str);
CC_API wchar_t CC_CALL cstrw_back(const cstrw *str);

/* --- 修改操作 --- */
CC_API int CC_CALL cstrw_assign(cstrw *str, const cstrw *src);
CC_API int CC_CALL cstrw_assign_wcs(cstrw *str, const wchar_t *s);
CC_API int CC_CALL cstrw_assign_buffer(cstrw *str, const wchar_t *buffer, size_t len);
CC_API int CC_CALL cstrw_assign_fill(cstrw *str, size_t count, wchar_t c);

CC_API int CC_CALL cstrw_append(cstrw *str, const cstrw *src);
CC_API int CC_CALL cstrw_append_wcs(cstrw *str, const wchar_t *s);
CC_API int CC_CALL cstrw_append_buffer(cstrw *str, const wchar_t *buffer, size_t len);
CC_API int CC_CALL cstrw_append_char(cstrw *str, wchar_t c);

CC_API int CC_CALL cstrw_insert(cstrw *str, size_t pos, const cstrw *src);
CC_API int CC_CALL cstrw_insert_wcs(cstrw *str, size_t pos, const wchar_t *s);
CC_API int CC_CALL cstrw_insert_char(cstrw *str, size_t pos, wchar_t c);

CC_API int CC_CALL cstrw_erase(cstrw *str, size_t pos, size_t len);
CC_API void CC_CALL cstrw_pop_back(cstrw *str);
CC_API int CC_CALL cstrw_push_back(cstrw *str, wchar_t c);

CC_API int CC_CALL cstrw_replace(cstrw *str, size_t pos, size_t len, const cstrw *src);
CC_API int CC_CALL cstrw_replace_wcs(cstrw *str, size_t pos, size_t len, const wchar_t *s);
CC_API int CC_CALL cstrw_replace_all(cstrw *str, const wchar_t *old_str, const wchar_t *new_str);

CC_API void CC_CALL cstrw_swap(cstrw *str1, cstrw *str2);

/* --- 查找操作 --- */
CC_API size_t CC_CALL cstrw_find(const cstrw *str, const cstrw *substr, size_t pos);
CC_API size_t CC_CALL cstrw_find_wcs(const cstrw *str, const wchar_t *substr, size_t pos);
CC_API size_t CC_CALL cstrw_find_char(const cstrw *str, wchar_t c, size_t pos);

CC_API size_t CC_CALL cstrw_rfind(const cstrw *str, const cstrw *substr, size_t pos);
CC_API size_t CC_CALL cstrw_rfind_wcs(const cstrw *str, const wchar_t *substr, size_t pos);
CC_API size_t CC_CALL cstrw_rfind_char(const cstrw *str, wchar_t c, size_t pos);

CC_API size_t CC_CALL cstrw_find_first_of(const cstrw *str, const wchar_t *char_set, size_t pos);
CC_API size_t CC_CALL cstrw_find_last_of(const cstrw *str, const wchar_t *char_set, size_t pos);
CC_API size_t CC_CALL cstrw_find_first_not_of(const cstrw *str, const wchar_t *char_set, size_t pos);
CC_API size_t CC_CALL cstrw_find_last_not_of(const cstrw *str, const wchar_t *char_set, size_t pos);

/* --- 比较操作 --- */
CC_API int CC_CALL cstrw_compare(const cstrw *str1, const cstrw *str2);
CC_API int CC_CALL cstrw_compare_wcs(const cstrw *str, const wchar_t *s);
CC_API int CC_CALL cstrw_equals(const cstrw *str1, const cstrw *str2);
CC_API int CC_CALL cstrw_equals_wcs(const cstrw *str, const wchar_t *s);

/* --- C++20/23 新增接口 --- */
CC_API int CC_CALL cstrw_starts_with(const cstrw *str, const wchar_t *prefix);
CC_API int CC_CALL cstrw_starts_with_char(const cstrw *str, wchar_t c);
CC_API int CC_CALL cstrw_ends_with(const cstrw *str, const wchar_t *suffix);
CC_API int CC_CALL cstrw_ends_with_char(const cstrw *str, wchar_t c);
CC_API int CC_CALL cstrw_contains(const cstrw *str, const wchar_t *substr);
CC_API int CC_CALL cstrw_contains_char(const cstrw *str, wchar_t c);

/* --- 子字符串操作 --- */
CC_API cstrw* CC_CALL cstrw_substr(const cstrw *str, size_t pos, size_t len);

/* --- 字符串转换 --- */
CC_API void CC_CALL cstrw_to_upper(cstrw *str);
CC_API void CC_CALL cstrw_to_lower(cstrw *str);
CC_API void CC_CALL cstrw_trim(cstrw *str);
CC_API void CC_CALL cstrw_trim_left(cstrw *str);
CC_API void CC_CALL cstrw_trim_right(cstrw *str);
CC_API void CC_CALL cstrw_reverse(cstrw *str);

CC_API int CC_CALL cstrw_format(cstrw *str, const wchar_t *fmt, ...);
CC_API cstrw* CC_CALL cstrw_format_new(const wchar_t *fmt, ...);

/* ============================================================================
 * cstr8 - UTF-8字符串接口
 * ============================================================================ */

/* --- 构造/析构 --- */
CC_API cstr8* CC_CALL cstr8_create(void);
CC_API cstr8* CC_CALL cstr8_create_from_utf8(const char *str);
CC_API cstr8* CC_CALL cstr8_create_from_buffer(const char *buffer, size_t byte_len);
CC_API cstr8* CC_CALL cstr8_create_with_capacity(size_t capacity);
CC_API cstr8* CC_CALL cstr8_clone(const cstr8 *src);
CC_API void CC_CALL cstr8_destroy(cstr8 **pstr);

CC_API int CC_CALL cstr8_init(cstr8 *str);
CC_API int CC_CALL cstr8_init_from_utf8(cstr8 *str, const char *s);
CC_API void CC_CALL cstr8_uninit(cstr8 *str);

/* --- 容量操作 --- */
CC_API size_t CC_CALL cstr8_length(const cstr8 *str);        /* 字节长度 */
CC_API size_t CC_CALL cstr8_char_count(const cstr8 *str);    /* UTF-8字符数量 */
CC_API size_t CC_CALL cstr8_size(const cstr8 *str);
CC_API size_t CC_CALL cstr8_capacity(const cstr8 *str);
CC_API int CC_CALL cstr8_empty(const cstr8 *str);
CC_API int CC_CALL cstr8_reserve(cstr8 *str, size_t new_capacity);
CC_API int CC_CALL cstr8_shrink_to_fit(cstr8 *str);
CC_API void CC_CALL cstr8_clear(cstr8 *str);
CC_API int CC_CALL cstr8_resize(cstr8 *str, size_t new_byte_size);

/* --- 元素访问（UTF-8字符）--- */
CC_API uint32_t CC_CALL cstr8_char_at(const cstr8 *str, size_t char_index);
CC_API const char* CC_CALL cstr8_data(const cstr8 *str);
CC_API const char* CC_CALL cstr8_c_str(const cstr8 *str);
CC_API uint32_t CC_CALL cstr8_front_char(const cstr8 *str);
CC_API uint32_t CC_CALL cstr8_back_char(const cstr8 *str);

/* --- 修改操作 --- */
CC_API int CC_CALL cstr8_assign(cstr8 *str, const cstr8 *src);
CC_API int CC_CALL cstr8_assign_utf8(cstr8 *str, const char *s);
CC_API int CC_CALL cstr8_assign_buffer(cstr8 *str, const char *buffer, size_t byte_len);

CC_API int CC_CALL cstr8_append(cstr8 *str, const cstr8 *src);
CC_API int CC_CALL cstr8_append_utf8(cstr8 *str, const char *s);
CC_API int CC_CALL cstr8_append_buffer(cstr8 *str, const char *buffer, size_t byte_len);
CC_API int CC_CALL cstr8_append_codepoint(cstr8 *str, uint32_t codepoint);

CC_API int CC_CALL cstr8_insert(cstr8 *str, size_t byte_pos, const cstr8 *src);
CC_API int CC_CALL cstr8_insert_utf8(cstr8 *str, size_t byte_pos, const char *s);
CC_API int CC_CALL cstr8_insert_codepoint(cstr8 *str, size_t char_index, uint32_t codepoint);

CC_API int CC_CALL cstr8_erase(cstr8 *str, size_t byte_pos, size_t byte_len);
CC_API int CC_CALL cstr8_erase_char(cstr8 *str, size_t char_index, size_t char_count);
CC_API void CC_CALL cstr8_pop_back(cstr8 *str);
CC_API int CC_CALL cstr8_push_back(cstr8 *str, uint32_t codepoint);

CC_API int CC_CALL cstr8_replace(cstr8 *str, size_t byte_pos, size_t byte_len, const cstr8 *src);
CC_API int CC_CALL cstr8_replace_utf8(cstr8 *str, size_t byte_pos, size_t byte_len, const char *s);
CC_API int CC_CALL cstr8_replace_all(cstr8 *str, const char *old_str, const char *new_str);

CC_API void CC_CALL cstr8_swap(cstr8 *str1, cstr8 *str2);

/* --- 查找操作 --- */
CC_API size_t CC_CALL cstr8_find(const cstr8 *str, const cstr8 *substr, size_t byte_pos);
CC_API size_t CC_CALL cstr8_find_utf8(const cstr8 *str, const char *substr, size_t byte_pos);
CC_API size_t CC_CALL cstr8_find_codepoint(const cstr8 *str, uint32_t codepoint, size_t byte_pos);

CC_API size_t CC_CALL cstr8_rfind(const cstr8 *str, const cstr8 *substr, size_t byte_pos);
CC_API size_t CC_CALL cstr8_rfind_utf8(const cstr8 *str, const char *substr, size_t byte_pos);
CC_API size_t CC_CALL cstr8_rfind_codepoint(const cstr8 *str, uint32_t codepoint, size_t byte_pos);

/* --- 比较操作 --- */
CC_API int CC_CALL cstr8_compare(const cstr8 *str1, const cstr8 *str2);
CC_API int CC_CALL cstr8_compare_utf8(const cstr8 *str, const char *s);
CC_API int CC_CALL cstr8_equals(const cstr8 *str1, const cstr8 *str2);
CC_API int CC_CALL cstr8_equals_utf8(const cstr8 *str, const char *s);

/* --- C++20/23 新增接口 --- */
CC_API int CC_CALL cstr8_starts_with(const cstr8 *str, const char *prefix);
CC_API int CC_CALL cstr8_ends_with(const cstr8 *str, const char *suffix);
CC_API int CC_CALL cstr8_contains(const cstr8 *str, const char *substr);
CC_API int CC_CALL cstr8_contains_codepoint(const cstr8 *str, uint32_t codepoint);

/* --- 子字符串操作 --- */
CC_API cstr8* CC_CALL cstr8_substr(const cstr8 *str, size_t byte_pos, size_t byte_len);
CC_API cstr8* CC_CALL cstr8_substr_chars(const cstr8 *str, size_t char_index, size_t char_count);

/* --- 字符串转换 --- */
CC_API void CC_CALL cstr8_to_upper(cstr8 *str);
CC_API void CC_CALL cstr8_to_lower(cstr8 *str);
CC_API void CC_CALL cstr8_trim(cstr8 *str);
CC_API void CC_CALL cstr8_trim_left(cstr8 *str);
CC_API void CC_CALL cstr8_trim_right(cstr8 *str);
CC_API void CC_CALL cstr8_reverse(cstr8 *str);

/* --- UTF-8 特殊功能 --- */
CC_API int CC_CALL cstr8_validate(const cstr8 *str);
CC_API int CC_CALL cstr8_validate_utf8(const char *str, size_t byte_len);
CC_API size_t CC_CALL cstr8_byte_index_to_char_index(const cstr8 *str, size_t byte_index);
CC_API size_t CC_CALL cstr8_char_index_to_byte_index(const cstr8 *str, size_t char_index);

/* ============================================================================
 * 字符串互相转换
 * ============================================================================ */

/* cstr <-> cstrw */
CC_API cstrw* CC_CALL cstr_to_cstrw(const cstr *str);
CC_API cstr* CC_CALL cstrw_to_cstr(const cstrw *str);

/* cstr <-> cstr8 */
CC_API cstr8* CC_CALL cstr_to_cstr8(const cstr *str);
CC_API cstr* CC_CALL cstr8_to_cstr(const cstr8 *str);

/* cstrw <-> cstr8 */
CC_API cstr8* CC_CALL cstrw_to_cstr8(const cstrw *str);
CC_API cstrw* CC_CALL cstr8_to_cstrw(const cstr8 *str);

/* 原地转换 */
CC_API int CC_CALL cstr_assign_from_cstrw(cstr *dst, const cstrw *src);
CC_API int CC_CALL cstr_assign_from_cstr8(cstr *dst, const cstr8 *src);
CC_API int CC_CALL cstrw_assign_from_cstr(cstrw *dst, const cstr *src);
CC_API int CC_CALL cstrw_assign_from_cstr8(cstrw *dst, const cstr8 *src);
CC_API int CC_CALL cstr8_assign_from_cstr(cstr8 *dst, const cstr *src);
CC_API int CC_CALL cstr8_assign_from_cstrw(cstr8 *dst, const cstrw *src);

/* ============================================================================
 * 编码探测
 * ============================================================================ */

/**
 * @brief 探测内存块的字符编码
 * @param data 内存块指针
 * @param size 内存块大小
 * @return 编码类型
 */
CC_API cstring_encoding CC_CALL cstring_detect_encoding(const void *data, size_t size);

/**
 * @brief 检查是否为有效的UTF-8编码
 */
CC_API int CC_CALL cstring_is_valid_utf8(const char *data, size_t size);

/**
 * @brief 检查是否为有效的UTF-16编码
 */
CC_API int CC_CALL cstring_is_valid_utf16(const void *data, size_t size);

/**
 * @brief 检查是否为纯ASCII编码（所有字节 < 128）
 */
CC_API int CC_CALL cstring_is_ascii(const char *data, size_t size);

/**
 * @brief 检查是否包含BOM（Byte Order Mark）
 * @return BOM类型，如果没有BOM则返回CSTRING_ENC_UNKNOWN
 */
CC_API cstring_encoding CC_CALL cstring_detect_bom(const void *data, size_t size);

/* ============================================================================
 * UTF-8 编解码辅助函数
 * ============================================================================ */

/**
 * @brief 将Unicode码点编码为UTF-8
 * @param codepoint Unicode码点
 * @param out_buffer 输出缓冲区（至少4字节）
 * @return 写入的字节数，错误返回0
 */
CC_API int CC_CALL cstring_utf8_encode(uint32_t codepoint, char *out_buffer);

/**
 * @brief 从UTF-8字节序列解码为Unicode码点
 * @param utf8_str UTF-8字符串
 * @param out_codepoint 输出码点
 * @return 消耗的字节数，错误返回0
 */
CC_API int CC_CALL cstring_utf8_decode(const char *utf8_str, uint32_t *out_codepoint);

/**
 * @brief 获取UTF-8字符的字节长度
 * @param first_byte UTF-8字符的第一个字节
 * @return 字符的字节长度（1-4），错误返回0
 */
CC_API int CC_CALL cstring_utf8_char_len(unsigned char first_byte);

/**
 * @brief 计算UTF-8字符串中的字符数量
 * @param utf8_str UTF-8字符串
 * @param byte_len 字节长度
 * @return 字符数量
 */
CC_API size_t CC_CALL cstring_utf8_strlen(const char *utf8_str, size_t byte_len);

/* ============================================================================
 * 字符串分割和连接
 * ============================================================================ */

/**
 * @brief 分割字符串
 * @param str 源字符串
 * @param delimiter 分隔符
 * @param out_count 输出分割后的字符串数量
 * @return 字符串数组，需要调用cstring_split_free释放
 */
CC_API cstr** CC_CALL cstr_split(const cstr *str, const char *delimiter, size_t *out_count);
CC_API cstrw** CC_CALL cstrw_split(const cstrw *str, const wchar_t *delimiter, size_t *out_count);
CC_API cstr8** CC_CALL cstr8_split(const cstr8 *str, const char *delimiter, size_t *out_count);

/**
 * @brief 释放split返回的字符串数组
 */
CC_API void CC_CALL cstr_split_free(cstr **strings, size_t count);
CC_API void CC_CALL cstrw_split_free(cstrw **strings, size_t count);
CC_API void CC_CALL cstr8_split_free(cstr8 **strings, size_t count);

/**
 * @brief 连接字符串数组
 * @param strings 字符串数组
 * @param count 数组大小
 * @param delimiter 分隔符
 * @return 连接后的字符串
 */
CC_API cstr* CC_CALL cstr_join(cstr **strings, size_t count, const char *delimiter);
CC_API cstrw* CC_CALL cstrw_join(cstrw **strings, size_t count, const wchar_t *delimiter);
CC_API cstr8* CC_CALL cstr8_join(cstr8 **strings, size_t count, const char *delimiter);

/* ============================================================================
 * 便利别名和辅助函数（简化版函数名）
 * ============================================================================ */

/* cstr构造函数别名 */
#define cstr_from_cstr          cstr_create_from_cstr
#define cstr_from_buffer        cstr_create_from_buffer
#define cstr_from_cstr_n        cstr_create_from_cstr_n
#define cstr_with_capacity      cstr_create_with_capacity

/* 字符串到数值转换 */
CC_API int CC_CALL cstr_to_int(const cstr *str);
CC_API long CC_CALL cstr_to_long(const cstr *str);
CC_API double CC_CALL cstr_to_double(const cstr *str);
CC_API float CC_CALL cstr_to_float(const cstr *str);
CC_API int CC_CALL cstr_to_int_base(const cstr *str, int base);

/* 格式化追加和释放 */
CC_API int CC_CALL cstr_append_format(cstr *str, const char *fmt, ...);
CC_API char* CC_CALL cstr_release(cstr *str);

/* 大小写不敏感比较 */
CC_API int CC_CALL cstr_compare_ignore_case(const cstr *str1, const cstr *str2);
CC_API int CC_CALL cstr_compare_ignore_case_cstr(const cstr *str, const char *s);

/* 移除操作 */
CC_API size_t CC_CALL cstr_remove_all_cstr(cstr *str, const char *target);
CC_API int CC_CALL cstr_remove_first_cstr(cstr *str, const char *target);

/* 替换操作 */
CC_API size_t CC_CALL cstr_replace_all_cstr(cstr *str, const char *old_str, const char *new_str);
CC_API int CC_CALL cstr_replace_first_cstr(cstr *str, const char *old_str, const char *new_str);

/* starts_with / ends_with / contains 别名 */
#define cstr_starts_with_cstr   cstr_starts_with
#define cstr_ends_with_cstr     cstr_ends_with
#define cstr_contains_cstr      cstr_contains

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* C_CORE_CSTRING_H_ */


#ifndef C_CORE_CFILESYSTEM_H_
#define C_CORE_CFILESYSTEM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

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

/** \addtogroup cfilesystem 跨平台文件系统库（对标C++23 std::filesystem）
 * @{
 */

/* ============================================================================
 * 常量定义
 * ============================================================================ */

#define CFS_MAX_PATH 4096  /* 最大路径长度 */

/* ============================================================================
 * 枚举类型定义
 * ============================================================================ */

/**
 * @brief 文件类型枚举（对标std::filesystem::file_type）
 */
typedef enum cfs_file_type {
    CFS_FILE_TYPE_NONE = 0,       /**< 文件类型未知或不存在 */
    CFS_FILE_TYPE_NOT_FOUND = 1,  /**< 文件不存在 */
    CFS_FILE_TYPE_REGULAR = 2,    /**< 普通文件 */
    CFS_FILE_TYPE_DIRECTORY = 3,  /**< 目录 */
    CFS_FILE_TYPE_SYMLINK = 4,    /**< 符号链接 */
    CFS_FILE_TYPE_BLOCK = 5,      /**< 块设备 */
    CFS_FILE_TYPE_CHARACTER = 6,  /**< 字符设备 */
    CFS_FILE_TYPE_FIFO = 7,       /**< 命名管道(FIFO) */
    CFS_FILE_TYPE_SOCKET = 8,     /**< socket文件 */
    CFS_FILE_TYPE_UNKNOWN = 9     /**< 存在但类型未知 */
} cfs_file_type;

/**
 * @brief 权限位（对标std::filesystem::perms）
 */
typedef enum cfs_perms {
    CFS_PERMS_NONE = 0,
    /* 所有者权限 */
    CFS_PERMS_OWNER_READ = 0400,    /**< 所有者读权限 */
    CFS_PERMS_OWNER_WRITE = 0200,   /**< 所有者写权限 */
    CFS_PERMS_OWNER_EXEC = 0100,    /**< 所有者执行权限 */
    CFS_PERMS_OWNER_ALL = 0700,     /**< 所有者全部权限 */
    /* 组权限 */
    CFS_PERMS_GROUP_READ = 040,     /**< 组读权限 */
    CFS_PERMS_GROUP_WRITE = 020,    /**< 组写权限 */
    CFS_PERMS_GROUP_EXEC = 010,     /**< 组执行权限 */
    CFS_PERMS_GROUP_ALL = 070,      /**< 组全部权限 */
    /* 其他用户权限 */
    CFS_PERMS_OTHERS_READ = 04,     /**< 其他用户读权限 */
    CFS_PERMS_OTHERS_WRITE = 02,    /**< 其他用户写权限 */
    CFS_PERMS_OTHERS_EXEC = 01,     /**< 其他用户执行权限 */
    CFS_PERMS_OTHERS_ALL = 07,      /**< 其他用户全部权限 */
    /* 全部权限 */
    CFS_PERMS_ALL = 0777,           /**< 所有权限 */
    /* 特殊位 */
    CFS_PERMS_SET_UID = 04000,      /**< set-user-ID位 */
    CFS_PERMS_SET_GID = 02000,      /**< set-group-ID位 */
    CFS_PERMS_STICKY_BIT = 01000,   /**< sticky位 */
    CFS_PERMS_MASK = 07777          /**< 全部权限掩码 */
} cfs_perms;

/**
 * @brief 权限操作选项（对标std::filesystem::perm_options）
 */
typedef enum cfs_perm_options {
    CFS_PERM_REPLACE = 0,  /**< 替换权限 */
    CFS_PERM_ADD = 1,      /**< 添加权限 */
    CFS_PERM_REMOVE = 2,   /**< 移除权限 */
    CFS_PERM_NOFOLLOW = 4  /**< 不跟随符号链接 */
} cfs_perm_options;

/**
 * @brief 复制选项（对标std::filesystem::copy_options）
 */
typedef enum cfs_copy_options {
    CFS_COPY_NONE = 0,
    CFS_COPY_SKIP_EXISTING = 1,        /**< 跳过已存在的文件 */
    CFS_COPY_OVERWRITE_EXISTING = 2,   /**< 覆盖已存在的文件 */
    CFS_COPY_UPDATE_EXISTING = 4,      /**< 仅在源文件较新时更新 */
    CFS_COPY_RECURSIVE = 8,            /**< 递归复制子目录 */
    CFS_COPY_COPY_SYMLINKS = 16,       /**< 复制符号链接本身 */
    CFS_COPY_SKIP_SYMLINKS = 32,       /**< 跳过符号链接 */
    CFS_COPY_DIRECTORIES_ONLY = 64,    /**< 仅复制目录结构 */
    CFS_COPY_CREATE_SYMLINKS = 128,    /**< 创建符号链接而非复制 */
    CFS_COPY_CREATE_HARD_LINKS = 256   /**< 创建硬链接而非复制 */
} cfs_copy_options;

/**
 * @brief 目录选项（对标std::filesystem::directory_options）
 */
typedef enum cfs_directory_options {
    CFS_DIR_NONE = 0,
    CFS_DIR_FOLLOW_SYMLINKS = 1,      /**< 跟随符号链接 */
    CFS_DIR_SKIP_PERMISSION_DENIED = 2 /**< 跳过权限被拒绝的目录 */
} cfs_directory_options;

/**
 * @brief 错误码（对标std::errc）
 */
typedef enum cfs_errc {
    CFS_SUCCESS = 0,              /**< 成功 */
    CFS_ERROR_NO_MEMORY = 1,      /**< 内存不足 */
    CFS_ERROR_NOT_FOUND = 2,      /**< 文件不存在 */
    CFS_ERROR_EXISTS = 3,         /**< 文件已存在 */
    CFS_ERROR_NOT_DIRECTORY = 4,  /**< 不是目录 */
    CFS_ERROR_IS_DIRECTORY = 5,   /**< 是目录 */
    CFS_ERROR_NOT_EMPTY = 6,      /**< 目录非空 */
    CFS_ERROR_PERMISSION = 7,     /**< 权限被拒绝 */
    CFS_ERROR_INVALID_PATH = 8,   /**< 无效路径 */
    CFS_ERROR_TOO_LONG = 9,       /**< 路径过长 */
    CFS_ERROR_IO = 10,            /**< I/O错误 */
    CFS_ERROR_CROSS_DEVICE = 11,  /**< 跨设备链接 */
    CFS_ERROR_BUSY = 12,          /**< 设备或资源忙 */
    CFS_ERROR_UNKNOWN = 99        /**< 未知错误 */
} cfs_errc;

/* ============================================================================
 * 结构体定义
 * ============================================================================ */

/**
 * @brief 路径对象（对标std::filesystem::path）
 * 存储文件系统路径，支持路径操作
 */
typedef struct cfs_path {
    char data[CFS_MAX_PATH];  /**< 路径字符串 */
    size_t length;            /**< 路径长度 */
    bool is_absolute;         /**< 是否为绝对路径 */
} cfs_path;

/**
 * @brief 文件状态（对标std::filesystem::file_status）
 */
typedef struct cfs_file_status {
    cfs_file_type type;  /**< 文件类型 */
    int perms;           /**< 文件权限 */
} cfs_file_status;

/**
 * @brief 空间信息（对标std::filesystem::space_info）
 */
typedef struct cfs_space_info {
    uint64_t capacity;   /**< 总容量（字节） */
    uint64_t free;       /**< 可用空间（字节） */
    uint64_t available;  /**< 普通用户可用空间（字节） */
} cfs_space_info;

/**
 * @brief 文件时间（对标std::filesystem::file_time_type）
 * 使用time_t表示，秒级精度
 */
typedef struct cfs_file_time {
    time_t seconds;      /**< 秒数（从Epoch开始） */
    long nanoseconds;    /**< 纳秒部分 */
} cfs_file_time;

/**
 * @brief 目录项（对标std::filesystem::directory_entry）
 */
typedef struct cfs_directory_entry {
    cfs_path path;                /**< 文件路径 */
    cfs_file_status status;       /**< 文件状态 */
    cfs_file_status symlink_status; /**< 符号链接状态 */
    uint64_t file_size;           /**< 文件大小 */
    cfs_file_time last_write_time; /**< 最后修改时间 */
    bool status_known;            /**< 状态是否已知 */
    bool symlink_status_known;    /**< 符号链接状态是否已知 */
} cfs_directory_entry;

/**
 * @brief 目录迭代器（对标std::filesystem::directory_iterator）
 */
typedef struct cfs_directory_iterator {
    void *handle;                      /**< 平台相关的目录句柄 */
    cfs_directory_entry current_entry; /**< 当前目录项 */
    cfs_path directory_path;           /**< 目录路径 */
    cfs_directory_options options;     /**< 迭代选项 */
    bool is_end;                       /**< 是否到达末尾 */
    cfs_errc error_code;               /**< 错误码 */
} cfs_directory_iterator;

/**
 * @brief 递归目录迭代器（对标std::filesystem::recursive_directory_iterator）
 */
typedef struct cfs_recursive_directory_iterator {
    cfs_directory_iterator *stack;  /**< 迭代器栈 */
    size_t stack_size;              /**< 栈大小 */
    size_t stack_capacity;          /**< 栈容量 */
    int recursion_depth;            /**< 当前递归深度 */
    cfs_directory_options options;  /**< 迭代选项 */
    bool is_end;                    /**< 是否到达末尾 */
    cfs_errc error_code;            /**< 错误码 */
} cfs_recursive_directory_iterator;

/* ============================================================================
 * 路径操作函数（对标std::filesystem::path成员函数）
 * ============================================================================ */

/**
 * @brief 初始化路径对象
 * @param path 路径对象指针
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_init(cfs_path *path);

/**
 * @brief 从C字符串创建路径
 * @param path 路径对象指针
 * @param str C字符串
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_from_string(cfs_path *path, const char *str);

/**
 * @brief 清空路径
 * @param path 路径对象指针
 */
CC_API void CC_CALL cfs_path_clear(cfs_path *path);

/**
 * @brief 获取路径的C字符串
 * @param path 路径对象指针
 * @return C字符串（不要释放）
 */
CC_API const char* CC_CALL cfs_path_c_str(const cfs_path *path);

/**
 * @brief 检查路径是否为空
 * @param path 路径对象指针
 * @return true表示为空
 */
CC_API bool CC_CALL cfs_path_empty(const cfs_path *path);

/**
 * @brief 检查路径是否为绝对路径
 * @param path 路径对象指针
 * @return true表示为绝对路径
 */
CC_API bool CC_CALL cfs_path_is_absolute(const cfs_path *path);

/**
 * @brief 检查路径是否为相对路径
 * @param path 路径对象指针
 * @return true表示为相对路径
 */
CC_API bool CC_CALL cfs_path_is_relative(const cfs_path *path);

/**
 * @brief 获取路径的根名（Windows下为盘符如"C:"，POSIX下为空）
 * @param path 路径对象指针
 * @param out_root_name 输出根名
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_root_name(const cfs_path *path, cfs_path *out_root_name);

/**
 * @brief 获取路径的根目录（如"/"）
 * @param path 路径对象指针
 * @param out_root_dir 输出根目录
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_root_directory(const cfs_path *path, cfs_path *out_root_dir);

/**
 * @brief 获取路径的根路径（根名+根目录）
 * @param path 路径对象指针
 * @param out_root_path 输出根路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_root_path(const cfs_path *path, cfs_path *out_root_path);

/**
 * @brief 获取相对于根路径的相对路径
 * @param path 路径对象指针
 * @param out_relative_path 输出相对路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_relative_path(const cfs_path *path, cfs_path *out_relative_path);

/**
 * @brief 获取父路径
 * @param path 路径对象指针
 * @param out_parent_path 输出父路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_parent_path(const cfs_path *path, cfs_path *out_parent_path);

/**
 * @brief 获取文件名（包括扩展名）
 * @param path 路径对象指针
 * @param out_filename 输出文件名
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_filename(const cfs_path *path, cfs_path *out_filename);

/**
 * @brief 获取文件名（不包括扩展名）
 * @param path 路径对象指针
 * @param out_stem 输出文件名主干
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_stem(const cfs_path *path, cfs_path *out_stem);

/**
 * @brief 获取文件扩展名（包括.）
 * @param path 路径对象指针
 * @param out_extension 输出扩展名
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_extension(const cfs_path *path, cfs_path *out_extension);

/**
 * @brief 拼接路径
 * @param path 路径对象指针
 * @param other 要拼接的路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_append(cfs_path *path, const cfs_path *other);

/**
 * @brief 拼接路径（使用C字符串）
 * @param path 路径对象指针
 * @param str 要拼接的路径字符串
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_append_string(cfs_path *path, const char *str);

/**
 * @brief 连接路径（不添加分隔符）
 * @param path 路径对象指针
 * @param other 要连接的路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_concat(cfs_path *path, const cfs_path *other);

/**
 * @brief 替换文件名
 * @param path 路径对象指针
 * @param new_filename 新文件名
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_replace_filename(cfs_path *path, const char *new_filename);

/**
 * @brief 替换扩展名
 * @param path 路径对象指针
 * @param new_extension 新扩展名
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_replace_extension(cfs_path *path, const char *new_extension);

/**
 * @brief 规范化路径（解析.和..，删除冗余分隔符）
 * @param path 路径对象指针
 * @param out_normalized 输出规范化后的路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_path_lexically_normal(const cfs_path *path, cfs_path *out_normalized);

/**
 * @brief 比较两个路径
 * @param path1 路径1
 * @param path2 路径2
 * @return 0表示相等，<0表示path1小于path2，>0表示path1大于path2
 */
CC_API int CC_CALL cfs_path_compare(const cfs_path *path1, const cfs_path *path2);

/* ============================================================================
 * 文件系统查询函数（对标std::filesystem非成员函数）
 * ============================================================================ */

/**
 * @brief 获取当前工作目录
 * @param out_path 输出当前工作目录
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_current_path(cfs_path *out_path);

/**
 * @brief 设置当前工作目录
 * @param path 要设置的路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_current_path_set(const cfs_path *path);

/**
 * @brief 获取绝对路径
 * @param path 相对路径
 * @param out_absolute 输出绝对路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_absolute(const cfs_path *path, cfs_path *out_absolute);

/**
 * @brief 获取规范路径（绝对路径+规范化，解析符号链接）
 * @param path 输入路径
 * @param out_canonical 输出规范路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_canonical(const cfs_path *path, cfs_path *out_canonical);

/**
 * @brief 获取弱规范路径（不解析符号链接）
 * @param path 输入路径
 * @param out_weakly_canonical 输出弱规范路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_weakly_canonical(const cfs_path *path, cfs_path *out_weakly_canonical);

/**
 * @brief 获取相对路径
 * @param path 输入路径
 * @param base 基准路径
 * @param out_relative 输出相对路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_relative(const cfs_path *path, const cfs_path *base, cfs_path *out_relative);

/**
 * @brief 获取代理相对路径（proximate）
 * @param path 输入路径
 * @param base 基准路径
 * @param out_proximate 输出代理相对路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_proximate(const cfs_path *path, const cfs_path *base, cfs_path *out_proximate);

/**
 * @brief 检查文件或目录是否存在
 * @param path 路径
 * @return true表示存在
 */
CC_API bool CC_CALL cfs_exists(const cfs_path *path);

/**
 * @brief 检查两个路径是否指向同一文件
 * @param path1 路径1
 * @param path2 路径2
 * @param out_equivalent 输出是否相同
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_equivalent(const cfs_path *path1, const cfs_path *path2, bool *out_equivalent);

/**
 * @brief 获取文件大小
 * @param path 路径
 * @param out_size 输出文件大小
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_file_size(const cfs_path *path, uint64_t *out_size);

/**
 * @brief 获取硬链接数量
 * @param path 路径
 * @param out_count 输出硬链接数量
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_hard_link_count(const cfs_path *path, size_t *out_count);

/**
 * @brief 获取最后修改时间
 * @param path 路径
 * @param out_time 输出修改时间
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_last_write_time(const cfs_path *path, cfs_file_time *out_time);

/**
 * @brief 设置最后修改时间
 * @param path 路径
 * @param new_time 新的修改时间
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_last_write_time_set(const cfs_path *path, const cfs_file_time *new_time);

/**
 * @brief 获取文件权限
 * @param path 路径
 * @param out_perms 输出权限
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_permissions_get(const cfs_path *path, int *out_perms);

/**
 * @brief 设置文件权限
 * @param path 路径
 * @param perms 权限
 * @param options 权限操作选项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_permissions_set(const cfs_path *path, int perms, cfs_perm_options options);

/**
 * @brief 读取符号链接
 * @param path 符号链接路径
 * @param out_target 输出目标路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_read_symlink(const cfs_path *path, cfs_path *out_target);

/**
 * @brief 获取文件状态（跟随符号链接）
 * @param path 路径
 * @param out_status 输出文件状态
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_status(const cfs_path *path, cfs_file_status *out_status);

/**
 * @brief 获取符号链接状态（不跟随符号链接）
 * @param path 路径
 * @param out_status 输出符号链接状态
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_symlink_status(const cfs_path *path, cfs_file_status *out_status);

/**
 * @brief 获取空间信息
 * @param path 路径
 * @param out_space_info 输出空间信息
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_space(const cfs_path *path, cfs_space_info *out_space_info);

/**
 * @brief 检查是否为块设备
 * @param path 路径
 * @return true表示是块设备
 */
CC_API bool CC_CALL cfs_is_block_file(const cfs_path *path);

/**
 * @brief 检查是否为字符设备
 * @param path 路径
 * @return true表示是字符设备
 */
CC_API bool CC_CALL cfs_is_character_file(const cfs_path *path);

/**
 * @brief 检查是否为目录
 * @param path 路径
 * @return true表示是目录
 */
CC_API bool CC_CALL cfs_is_directory(const cfs_path *path);

/**
 * @brief 检查是否为空目录或空文件
 * @param path 路径
 * @return true表示为空
 */
CC_API bool CC_CALL cfs_is_empty(const cfs_path *path);

/**
 * @brief 检查是否为FIFO管道
 * @param path 路径
 * @return true表示是FIFO管道
 */
CC_API bool CC_CALL cfs_is_fifo(const cfs_path *path);

/**
 * @brief 检查是否为其他类型文件
 * @param path 路径
 * @return true表示是其他类型
 */
CC_API bool CC_CALL cfs_is_other(const cfs_path *path);

/**
 * @brief 检查是否为普通文件
 * @param path 路径
 * @return true表示是普通文件
 */
CC_API bool CC_CALL cfs_is_regular_file(const cfs_path *path);

/**
 * @brief 检查是否为socket文件
 * @param path 路径
 * @return true表示是socket文件
 */
CC_API bool CC_CALL cfs_is_socket(const cfs_path *path);

/**
 * @brief 检查是否为符号链接
 * @param path 路径
 * @return true表示是符号链接
 */
CC_API bool CC_CALL cfs_is_symlink(const cfs_path *path);

/* ============================================================================
 * 文件系统修改函数（对标std::filesystem非成员函数）
 * ============================================================================ */

/**
 * @brief 复制文件或目录
 * @param from 源路径
 * @param to 目标路径
 * @param options 复制选项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_copy(const cfs_path *from, const cfs_path *to, cfs_copy_options options);

/**
 * @brief 复制文件
 * @param from 源文件路径
 * @param to 目标文件路径
 * @param options 复制选项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_copy_file(const cfs_path *from, const cfs_path *to, cfs_copy_options options);

/**
 * @brief 复制符号链接
 * @param from 源符号链接路径
 * @param to 目标符号链接路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_copy_symlink(const cfs_path *from, const cfs_path *to);

/**
 * @brief 创建目录
 * @param path 目录路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_create_directory(const cfs_path *path);

/**
 * @brief 递归创建目录
 * @param path 目录路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_create_directories(const cfs_path *path);

/**
 * @brief 创建硬链接
 * @param target 目标路径
 * @param link 链接路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_create_hard_link(const cfs_path *target, const cfs_path *link);

/**
 * @brief 创建符号链接
 * @param target 目标路径
 * @param link 链接路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_create_symlink(const cfs_path *target, const cfs_path *link);

/**
 * @brief 创建目录符号链接
 * @param target 目标目录路径
 * @param link 链接路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_create_directory_symlink(const cfs_path *target, const cfs_path *link);

/**
 * @brief 删除文件或空目录
 * @param path 路径
 * @param out_removed 输出是否成功删除
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_remove(const cfs_path *path, bool *out_removed);

/**
 * @brief 递归删除目录及其内容
 * @param path 路径
 * @param out_removed_count 输出删除的文件/目录数量
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_remove_all(const cfs_path *path, size_t *out_removed_count);

/**
 * @brief 重命名或移动文件
 * @param old_path 原路径
 * @param new_path 新路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_rename(const cfs_path *old_path, const cfs_path *new_path);

/**
 * @brief 调整文件大小
 * @param path 文件路径
 * @param new_size 新大小
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_resize_file(const cfs_path *path, uint64_t new_size);

/**
 * @brief 获取临时目录路径
 * @param out_temp_path 输出临时目录路径
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_temp_directory_path(cfs_path *out_temp_path);

/* ============================================================================
 * 目录迭代器函数（对标std::filesystem::directory_iterator）
 * ============================================================================ */

/**
 * @brief 创建目录迭代器
 * @param it 迭代器指针
 * @param path 目录路径
 * @param options 迭代选项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_directory_iterator_create(cfs_directory_iterator *it, 
                                                        const cfs_path *path,
                                                        cfs_directory_options options);

/**
 * @brief 销毁目录迭代器
 * @param it 迭代器指针
 */
CC_API void CC_CALL cfs_directory_iterator_destroy(cfs_directory_iterator *it);

/**
 * @brief 检查迭代器是否到达末尾
 * @param it 迭代器指针
 * @return true表示已到达末尾
 */
CC_API bool CC_CALL cfs_directory_iterator_is_end(const cfs_directory_iterator *it);

/**
 * @brief 获取当前目录项
 * @param it 迭代器指针
 * @param out_entry 输出目录项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_directory_iterator_get(const cfs_directory_iterator *it, 
                                                     cfs_directory_entry *out_entry);

/**
 * @brief 移动到下一项
 * @param it 迭代器指针
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_directory_iterator_next(cfs_directory_iterator *it);

/* ============================================================================
 * 递归目录迭代器函数（对标std::filesystem::recursive_directory_iterator）
 * ============================================================================ */

/**
 * @brief 创建递归目录迭代器
 * @param it 迭代器指针
 * @param path 目录路径
 * @param options 迭代选项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_recursive_directory_iterator_create(cfs_recursive_directory_iterator *it,
                                                                  const cfs_path *path,
                                                                  cfs_directory_options options);

/**
 * @brief 销毁递归目录迭代器
 * @param it 迭代器指针
 */
CC_API void CC_CALL cfs_recursive_directory_iterator_destroy(cfs_recursive_directory_iterator *it);

/**
 * @brief 检查迭代器是否到达末尾
 * @param it 迭代器指针
 * @return true表示已到达末尾
 */
CC_API bool CC_CALL cfs_recursive_directory_iterator_is_end(const cfs_recursive_directory_iterator *it);

/**
 * @brief 获取当前目录项
 * @param it 迭代器指针
 * @param out_entry 输出目录项
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_recursive_directory_iterator_get(const cfs_recursive_directory_iterator *it,
                                                               cfs_directory_entry *out_entry);

/**
 * @brief 移动到下一项
 * @param it 迭代器指针
 * @return 错误码
 */
CC_API cfs_errc CC_CALL cfs_recursive_directory_iterator_next(cfs_recursive_directory_iterator *it);

/**
 * @brief 获取当前递归深度
 * @param it 迭代器指针
 * @return 递归深度（0表示根目录）
 */
CC_API int CC_CALL cfs_recursive_directory_iterator_depth(const cfs_recursive_directory_iterator *it);

/**
 * @brief 禁用当前目录的递归（跳过当前目录的子目录）
 * @param it 迭代器指针
 */
CC_API void CC_CALL cfs_recursive_directory_iterator_disable_recursion_pending(cfs_recursive_directory_iterator *it);

/**
 * @brief 弹出当前目录（返回上一级目录）
 * @param it 迭代器指针
 */
CC_API void CC_CALL cfs_recursive_directory_iterator_pop(cfs_recursive_directory_iterator *it);

/* ============================================================================
 * 辅助函数
 * ============================================================================ */

/**
 * @brief 获取错误码描述
 * @param err 错误码
 * @return 错误描述字符串
 */
CC_API const char* CC_CALL cfs_errc_message(cfs_errc err);

/**
 * @brief 获取平台路径分隔符
 * @return 路径分隔符字符（Windows为'\\'，POSIX为'/'）
 */
CC_API char CC_CALL cfs_path_separator(void);

/**
 * @brief 获取平台首选路径分隔符
 * @return 首选路径分隔符字符
 */
CC_API char CC_CALL cfs_path_preferred_separator(void);

/* ============================================================================
| * 兼容旧API的类型定义和宏
| * ============================================================================ */


typedef struct cfs_array
{
    char **files;
    size_t size;
} cfs_array;

/* ============================================================================
| * 兼容旧API的函数声明
| * ============================================================================ */

/**
 * @brief 获取系统字体文件夹路径
 * @param buf 输出缓冲
 * @param buf_len 输出缓冲长度
 * @return bool
 */
CC_API int CC_CALL cfs_get_font_dir(char *buf, int buf_len);

/**
 * @brief 获取系统临时文件夹路径
 * @param buf 输出缓冲
 * @param buf_len 输出缓冲长度
 * @return bool
 */
CC_API int CC_CALL cfs_get_temp_dir(char *buf, int buf_len);

/**
 * @brief 获取系统临时文件
 * @param file_name 指定文件名
 * @param unix_like_backslash 返回的路径使用是否unix风格的反斜杠
 * @return bool
 */
CC_API char *CC_CALL cfs_make_temp_file_path(char *file_name, int unix_like_backslash);

/**
 * @brief 获取当前程序的工作目录
 * @param buf 输出缓冲
 * @param buf_len 输出缓冲长度
 * @return bool
 */
CC_API int CC_CALL cfs_get_work_dir(char *buf, int buf_len);

/**
 * @brief 判断文件是否存在
 * @param filepath_utf8 文件路径
 * @return bool
 */
CC_API int CC_CALL cfs_is_file_exist(const char *filepath_utf8);

/**
 * @brief 判断文件夹是否存在
 * @param filepath_utf8 文件夹路径
 * @return bool
 */
CC_API int CC_CALL cfs_is_dir_exist(const char *filepath_utf8);

/**
 * @brief 判断文件或文件夹是否存在
 * @param filepath_utf8 文件或文件夹路径
 * @return bool
 */
CC_API int CC_CALL cfs_is_path_exist(const char *filepath_utf8);

/**
 * @brief 判断是否是相对路径
 * @param filepath_utf8 路径
 * @return bool
 */
CC_API int CC_CALL cfs_is_relative_path(const char *filepath_utf8);

/**
 * @brief 相对路径转绝对路径
 * @param filepath 路径
 * @param pre_dir 上级目录
 * @param out 输出缓冲
 * @param out_len 输出缓冲长度
 * @return bool
 */
CC_API int CC_CALL cfs_relative_to_absolute(const char *filepath, const char *pre_dir, char *out, int out_len);

/**
 * @brief 创建文件夹
 * @param filepath_utf8 文件夹路径
 * @return bool
 */
CC_API int CC_CALL cfs_create_dir(const char *filepath_utf8);

/**
 * @brief 判断文件大小
 * @param filepath 文件路径
 * @return 长度
 */
CC_API unsigned int CC_CALL cfs_get_file_size(const char *filepath);

/**
 * @brief 读取二进制文件的内容
 * @param filepath 文件路径
 * @param outbuf 输出缓冲
 * @param outbuf_len 输出缓冲长度,字节数
 * @return bool
 */
CC_API unsigned int CC_CALL cfs_read_binary_file(const char *filepath, void *outbuf, size_t outbuf_len);

/**
 * @brief 写二进制文件的内容
 * @param filepath 文件路径
 * @param buf 输出缓冲
 * @param buf_len 输出缓冲长度,字节数
 * @return bool
 */
CC_API unsigned int CC_CALL cfs_write_binary_file(const char *filepath, void *buf, size_t buf_len);

/**
 * @brief 删除文件（旧API版本）
 * @param filepath_utf8 文件路径
 * @return bool
 */
CC_API int CC_CALL cfs_remove_old(const char *filepath_utf8);

/**
 * @brief 设置文件属性
 * @param lpszFileName 文件名
 * @param wDate 日期
 * @param wTime 时间
 * @param wAttribs 属性
 * @return bool
 */
CC_API int CC_CALL cfs_set_file_attr_date(const char *lpszFileName, unsigned short wDate, unsigned short wTime,
    unsigned short wAttribs);

/**
* @brief 获取文件夹中的文件
*
* @param dir 文件夹
* @param recursive 是否包含子文件夹中的文件
* @return 文件夹数组，通过LabEZ_Arr_Foreach访问返回值
*/
CC_API cfs_array *CC_CALL cfs_get_dir_files(const char *dir, int recursive);

/**
* @brief 获取子文件夹
*
* @param dir 文件夹
* @param wild_mask 通配符
* @return 文件夹数组，通过LabEZ_Arr_Foreach访问返回值
*/
CC_API cfs_array *CC_CALL cfs_get_sub_dirs(const char *dir, const char *wild_mask);

/**
* @brief 获取所有目录，包括嵌套目录
*
* @param dir 文件夹
* @param wild_mask 通配符
* @return 文件夹数组，通过LabEZ_Arr_Foreach访问返回值
*/
CC_API cfs_array *CC_CALL cfs_get_dir_recursive(const char *dir, const char *wild_mask);

/**
* @brief 从环境变量中找出文件
*
* @param exe_or_dll_name 文件名
* @param unix_like_backslash 是否使用unix风格的字符串
* @return 字符串
*/
CC_API char *CC_CALL cfs_find_file_in_sys_path(const char *exe_or_dll_name, int unix_like_backslash);

/**
* @brief 获取文件夹大小
*
* @param folder_path
* @return 大小，字节数
*/
CC_API uint64_t CC_CALL cfs_get_folder_size(const char *folder_path);

/**
* @brief 计算文件crc32值
* @param file_path 文件路径
* @return crc32值
*/
CC_API uint32_t CC_CALL cfs_calc_file_crc32(const char *file_path);


/**
* @brief 从路径中获取文件名
* @param filepath 输入文件路径
* @param outbuf 输出字符缓存
* @param outbuf_len 输出字符缓存长度
* @param include_ext 是否包含扩展名
* @return 0表示成功，非0表示失败
*/
CC_API int CC_CALL cfs_get_base_name(const char * filepath, char * outbuf, unsigned int outbuf_len, int include_ext);


/** @} */ /* end of cfilesystem group */

#ifdef __cplusplus
}
#endif

#endif /* C_CORE_CFILESYSTEM_H_ */


#ifndef CMSG_H
#define CMSG_H

#include <string.h>

/* min/max 宏定义（Linux 标准库没有） */
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

/*
 * 通用的打包和解包私有协议的宏模块
 *
 * 使用X-Macro技术，一次字段定义自动生成encode和decode函数
 *
 * 使用方法：
 *
 * // 2. 生成encode和decode函数
 * GEN_MSG_CODEC(MYVAR)
 *
 * 或者使用内联定义方式（与用户的代码更接近）：
 * BEGIN_DEF_MSG(TYPE)
 *     DEF_FIELD_IN_BUF(...)
 * END_DEF_MSG()
 */

/**
 * 定义消息的encode和decode函数
 * @param TYPE 消息类型
 */
#define BEGIN_DEF_MSG(TYPE)                                         \
  static inline void TYPE##_endec(TYPE *inst, char *buf, int size,  \
                                  int is_encode);                   \
  static inline void TYPE##_encode(TYPE *inst, char *buf, int size) \
  {                                                                 \
    TYPE##_endec(inst, buf, size, 1);                               \
  }                                                                 \
  static inline void TYPE##_decode(TYPE *inst, char *buf, int size) \
  {                                                                 \
    TYPE##_endec(inst, buf, size, 0);                               \
  }                                                                 \
  static inline void TYPE##_endec(TYPE *inst, char *buf, int size,  \
                                  int is_encode)                    \
  {

/**
 * 定义字段，小端模式
 * @param field 字段名
 * @param offset 字段偏移量
 * @param type 字段类型
 */
#define DEF_FIELD_IN_LEBUF(field, offset, type)     \
  if (is_encode)                                    \
  {                                                 \
    DISABLE_WARNING_4244_BEGIN;                     \
    type tmp_val = inst->field;                     \
    DISABLE_WARNING_4244_END                        \
    memcpy(((char *)(buf)) + offset, &tmp_val,      \
           min(sizeof(type), sizeof(inst->field))); \
  }                                                 \
  else                                              \
  {                                                 \
    type tmp_val = 0;                               \
    memcpy(&tmp_val, ((char *)(buf)) + offset,      \
           min(sizeof(type), sizeof(inst->field))); \
    DISABLE_WARNING_4244_BEGIN;                     \
    inst->field = tmp_val;                          \
    DISABLE_WARNING_4244_END;                       \
  }

/**
 * 定义字段，大端模式
 * @param field 字段名
 * @param offset 字段偏移量
 * @param type 字段类型
 */
#define DEF_FIELD_IN_BEBUF(field, offset, type)                               \
  if (is_encode)                                                              \
  {                                                                           \
    DISABLE_WARNING_4244_BEGIN;                                               \
    type tmp_val = inst->field;                                               \
    DISABLE_WARNING_4244_END;                                                 \
    for (int i = 0; i < sizeof(type); i++)                                    \
    {                                                                         \
      ((char *)(buf + offset))[i] = ((char *)&tmp_val)[sizeof(type) - 1 - i]; \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    type tmp_val = 0;                                                         \
    for (int i = 0; i < min(sizeof(type), sizeof(inst->field)); i++)          \
    {                                                                         \
      ((char *)&tmp_val)[i] = ((char *)(buf + offset))[sizeof(type) - 1 - i]; \
    }                                                                         \
    DISABLE_WARNING_4244_BEGIN;                                               \
    inst->field = tmp_val;                                                    \
    DISABLE_WARNING_4244_END;                                                 \
  }

// 从小端内存中取一些BIT进行计算，再赋值给给变量
#define _BEGIN_DEF_BUF_BITS_TO_FIELD(mid_var_type, k, b, field) {

#define _END_DEF_BUF_BITS_TO_FIELD() }

/**
 * @brief 定义从小端字序内存视图中提取位域并进行线性变换后映射到字段
 *
 * @details 从小端字序（Little Endian）内存视图中的指定位置提取多个比特位，
 *          生成中间变量，然后通过线性变换公式 (y = k*x + b)
 * 转换后赋值给消息字段。 该宏支持双向操作：
 *          - 编码(encode)：从字段值通过反向线性变换 x = (y - b) / k
 * 计算中间变量， 然后将中间变量的各个位写入到缓冲区的指定位置
 *          - 解码(decode)：从缓冲区的指定位置读取位到中间变量，
 *            然后通过线性变换 y = k*x + b 计算并赋值给字段
 *
 * @note 使用注意事项：
 *       - 该宏假设存在 is_encode 变量来区分编码/解码模式
 *       - 该宏假设存在 buf 缓冲区指针和 inst 实例指针
 *       - 位的提取和写入按照大端位序进行（MSB first）
 *       - 当 k=0 时，内部会自动使用 k=1 避免除零错误
 *       - 适用于需要对位域数据进行缩放和偏移的场景，如传感器数据转换
 *
 * @param src_offset       源缓冲区的字节偏移量（从buf开始的偏移）
 * @param src_bit_index    源缓冲区中的起始位索引（相对于src_offset的位偏移）
 * @param bit_size         要读取/写入的位数
 * @param mid_var_bit_index 中间变量的位索引起始位置（用于位对齐）
 * @param mid_var_type     中间变量的数据类型（如 uint8_t, uint16_t, uint32_t
 * 等）
 * @param k                线性变换的斜率系数（倍率因子）
 * @param b                线性变换的偏移量（常量偏移）
 * @param field            目标字段名称（inst 结构体中的成员变量名）
 *
 * @return 无返回值（宏展开为语句块）
 *
 * @par 使用示例:
 * @code
 * // 假设要从缓冲区偏移 2 字节处，位索引 3 开始提取 10 个位
 * // 转换公式为：实际值 = 原始值 * 0.1 + (-40)
 * // 将结果存储到 inst->temperature 字段
 * DEF_BUF_MSB_BITS_ARR_TO_LE_VAR_KB_TO_FIELD(2, 3, 10, 0, uint16_t, 0.1, -40,
 * temperature);
 * @endcode
 */
#define DEF_BUF_MSB_BITS_ARR_TO_LE_VAR_KB_TO_FIELD(                             \
    src_offset, src_bit_index, bit_size, mid_var_bit_index, mid_var_type, k,    \
    b, field)                                                                   \
  do                                                                            \
  {                                                                             \
    mid_var_type mid_var = 0;                                                   \
    mid_var_type mask = 0x01;                                                   \
    float fk = k == 0 ? 1 : k;                                                  \
    float fb = b;                                                               \
    const int fzs = (bit_size % 8);                                             \
    const int byte_cnt = bit_size / 8;                                          \
    int zsbits = (byte_cnt * 8);                                                \
    int index = 0;                                                              \
    int bit_offset = 0;                                                         \
    int byte_offset = 0;                                                        \
    uint8_t *ptr = 0;                                                           \
    uint8_t bit = 0;                                                            \
    uint8_t st = 0;                                                             \
    if (fzs != 0)                                                               \
    {                                                                           \
      st = 8 - fzs;                                                             \
    }                                                                           \
    if (is_encode)                                                              \
    {                                                                           \
      /* 编码：从字段值反向计算中间变量 */                                      \
      DISABLE_WARNING_4244_BEGIN;                                               \
      mid_var = (mid_var_type)((inst->field * fk) + b);                         \
      DISABLE_WARNING_4244_END;                                                 \
                                                                                \
      /* 将中间变量的各个位写入到缓冲区 */                                      \
      for (int i = 0; i < bit_size; ++i)                                        \
      {                                                                         \
        index = (i + mid_var_bit_index);                                        \
        index = (index & (~7)) + (7 - (index & 7));                             \
        if (i >= zsbits)                                                        \
        {                                                                       \
          index = index - st;                                                   \
        }                                                                       \
        bit_offset = src_bit_index + i;                                         \
        byte_offset =                                                           \
            src_offset + (bit_offset >> 3); /* src_offset + (bit_offset / 8);   \
            >> 3 等价于 bit_offset / 8 */                                    \
        bit_offset = (7 - bit_offset);                                          \
        bit_offset &= 0x07; /* & 0x07 等价于 bit_offset % 8 */                  \
        ptr = (uint8_t *)buf + byte_offset;                                     \
        if (fzs && (i >= zsbits))                                               \
        {                                                                       \
          mask = 1 << index;                                                    \
        }                                                                       \
        else                                                                    \
        {                                                                       \
          mask = 1 << index; /*得到一个后面将要使用的mask*/                     \
        }                                                                       \
        bit = (mid_var & mask) != 0;                                            \
        /* 检查中间变量中的这一位是否为1 */                                     \
        if (bit)                                                                \
        {                                                                       \
          *ptr |= (1 << bit_offset); /* 设置位为1 */                            \
        }                                                                       \
        else                                                                    \
        {                                                                       \
          *ptr &= ~(1 << bit_offset); /* 清除位为0 */                           \
        }                                                                       \
      }                                                                         \
    }                                                                           \
    else                                                                        \
    {                                                                           \
      for (int i = 0; i < bit_size; ++i)                                        \
      {                                                                         \
        index = (i + mid_var_bit_index);                                        \
        index = (index & (~7)) + (7 - (index & 7));                             \
        if (i >= zsbits)                                                        \
        {                                                                       \
          index = index - st;                                                   \
        }                                                                       \
        bit_offset = src_bit_index + i;                                         \
        byte_offset =                                                           \
            src_offset + (bit_offset >> 3); /* src_offset + (bit_offset / 8);   \
                                               >> 3 等价于 bit_offset / 8 */ \
        bit_offset &= 0x07;                 /* & 0x07 等价于 bit_offset % 8 */  \
        ptr = ((uint8_t *)buf) + byte_offset;                                   \
        bit = ((*ptr) & (1 << (7 - bit_offset))) != 0;                          \
        if (fzs && (i >= zsbits))                                               \
        {                                                                       \
          mask = 1 << index;                                                    \
        }                                                                       \
        else                                                                    \
          mask = 1 << index; /*得到一个后面将要使用的mask*/                     \
        if (bit)             /*得到一个bit*/                                    \
        {                                                                       \
          mid_var |= mask;                                                      \
        }                                                                       \
      }                                                                         \
      DISABLE_WARNING_4244_BEGIN;                                               \
      inst->field = ((mid_var - b) / fk);                                       \
      DISABLE_WARNING_4244_END;                                                 \
    }                                                                           \
  } while (0)

// 从大端字序内存视图,中的一些bit，取出来后，生成中间变量，然后乘以K+B再输出到消息的某个字段。
#define DEF_BEBUFVIEW_BITS_TO_VAR_KB_TO_FIELD(src_offset, src_bit_index, size, \
                                              mid_var_bit_index, mid_var_type, \
                                              k, b, field)

/**
 * @brief 定义某个变量映射到buf中的某个bit位，将buf中的变量看成小端模式
 * @details 这个变量会被当做bool类型处理，然后将内存中的 offset 字节开始的
 * sizeof(type) 个字节看成是小端模式的type类型的变量。
 *          编码时就是将field转成bool类型，赋值给这个bit位。解码时就是将这个bit位转成bool类型，赋值给field。
 *          注意：bit_index 是指buf中的，小端模式，type类型变量中的位索引。
 * @param field 字段名
 * @param offset buf中的字节偏移量
 * @param type 字段类型
 * @param bit_index 在type变量中的位索引（bit 0为LSB）
 */
#define DEF_FIELD_IN_LEBUF_BIT_BOOL(field, offset, type, bit_index)      \
  do                                                                     \
  {                                                                      \
    int byte_idx =                                                       \
        (offset) + ((bit_index) >> 8); /*>> 3 等价于 bit_offset / 8*/    \
    int bit_idx = (bit_index & 0x07);  /* & 0x07 等价于 bit_index % 8 */ \
    if (is_encode)                                                       \
    {                                                                    \
      if (inst->field)                                                   \
        buf[byte_idx] |= (1 << bit_idx);                                 \
      else                                                               \
        buf[byte_idx] &= ~(1 << bit_idx);                                \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      if ((buf[byte_idx] >> bit_idx) & 1)                                \
        inst->field = 1;                                                 \
      else                                                               \
        inst->field = 0;                                                 \
    }                                                                    \
  } while (0)

/**
 * @brief 定义某个变量映射到buf中的某个bit位，将buf中的变量看成大端模式
 * @details 这个变量会被当做bool类型处理，然后将内存中的 offset 字节开始的
 * sizeof(type) 个字节看成是大端模式的type类型的变量。
 *          编码时就是将field转成bool类型，赋值给这个bit位。解码时就是将这个bit位转成bool类型，赋值给field。
 *          注意：bit_index 是指buf中的，大端模式，type类型变量中的位索引。
 * @param field 字段名
 * @param offset buf中的字节偏移量
 * @param type 字段类型
 * @param bit_index 在type变量中的位索引（bit 0为MSB）
 */
#define DEF_FIELD_IN_BEBUF_BIT_BOOL(field, offset, type, bit_index)      \
  do                                                                     \
  {                                                                      \
    int byte_idx = (offset + sizeof(type) - 1) +                         \
                   ((bit_index) >> 8); /*>> 3 等价于 bit_offset / 8*/    \
    int bit_idx = (bit_index & 0x07);  /* & 0x07 等价于 bit_index % 8 */ \
    if (is_encode)                                                       \
    {                                                                    \
      if (inst->field)                                                   \
        buf[byte_idx] |= (1 << bit_idx);                                 \
      else                                                               \
        buf[byte_idx] &= ~(1 << bit_idx);                                \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      inst->field = ((buf[byte_idx] >> bit_idx) & 1);                    \
    }                                                                    \
  } while (0)

/**
 * @brief 定义消息结构体字段到小端字节缓冲区的线性映射转换
 * @details 使用线性公式 y = k*x + b
 * 将字段值映射到缓冲区，缓冲区使用小端字节序(Little Endian)。
 *
 *          转换公式：
 *          - 编码时(field -> buf): buf_value = (field * k) + b
 *          - 解码时(buf -> field): field = (buf_value - b) / k
 *
 *          典型应用场景：
 *          1. 物理量单位转换：例如将摄氏度转换为0.1摄氏度单位存储（k=10, b=0）
 *          2. 数值范围映射：将浮点数映射到整数范围（设置合适的k和b值）
 *          3. 带偏移的线性缩放：例如温度传感器的线性校准
 *
 *          内存操作：
 *          - 使用memcpy进行数据拷贝，拷贝长度为field和buf_var_type中较小的size
 *          - 支持不同类型之间的转换（如float到int16_t）
 *          - 自动处理类型转换警告（通过DISABLE_WARNING_4244）
 *
 * @param field 结构体实例(inst)中的字段名称
 * @param k 线性缩放系数（斜率）。若k=0则自动使用k=1，即不缩放
 * @param b 线性偏移量（截距）
 * @param buf_offset 在buf缓冲区中的字节偏移量
 * @param buf_var_type buf中存储的变量类型（如uint8_t、int16_t、uint32_t等）
 *
 * @note 1. k值为0时会被自动替换为1，避免除零错误
 * @note 2.
 * 该宏依赖外部变量：is_encode(编码/解码标志)、inst(结构体实例指针)、buf(字节缓冲区)
 * @note 3. 涉及不同数值类型转换时需确保k和b的值域合理，避免溢出
 * @note 4. 缓冲区使用小端字节序，跨平台通信时需注意字节序一致性
 *
 * @warning 使用前必须确保buf_offset不会越界访问buf缓冲区
 *
 * @par 使用示例：
 * @code
 *   // 将温度字段(float类型，单位：℃)编码为int16_t(单位：0.01℃)
 *   DEF_FIELD_KB_TO_LEBUF(temperature, 100, 0, 0, int16_t);
 *
 *   // 将速度字段(float类型，单位：m/s)映射到uint8_t(范围0-255 -> 0-25.5 m/s)
 *   DEF_FIELD_KB_TO_LEBUF(speed, 10, 0, 2, uint8_t);
 * @endcode
 */
#define DEF_FIELD_KB_TO_LEBUF(field, k, b, buf_offset, buf_var_type) \
  do                                                                 \
  {                                                                  \
    float fk = k == 0 ? 1 : k;                                       \
    if (is_encode)                                                   \
    {                                                                \
      DISABLE_WARNING_4244_BEGIN;                                    \
      buf_var_type temp_val = ((inst->field * fk) + b);              \
      DISABLE_WARNING_4244_END;                                      \
      memcpy(buf + buf_offset, &temp_val,                            \
             min(sizeof(inst->field), sizeof(buf_var_type)));        \
    }                                                                \
    else                                                             \
    {                                                                \
      buf_var_type temp_val = 0;                                     \
      memcpy(&temp_val, buf + buf_offset,                            \
             min(sizeof(inst->field), sizeof(buf_var_type)));        \
      DISABLE_WARNING_4244_BEGIN;                                    \
      inst->field = ((temp_val - b) / fk);                           \
      DISABLE_WARNING_4244_END;                                      \
    }                                                                \
  } while (0)

/**
 * @brief 定义小端字节缓冲区到消息结构体字段的反向线性映射转换
 * @details 使用反向线性公式将缓冲区值映射到字段，缓冲区使用小端字节序(Little
 * Endian)。 这是DEF_FIELD_KB_TO_LEBUF的反向转换版本。
 *
 *          转换公式：
 *          - 编码时(field -> buf): buf_value = (field - b) / k
 *          - 解码时(buf -> field): field = (buf_value * k) + b
 *
 *          典型应用场景：
 *          1. 逆向物理量单位转换：已知buf中的缩放值，还原为原始物理量
 *          2.
 * 传感器原始值转换：将ADC原始值转换为实际物理量（k为增益，b为零点偏移）
 *          3. 协议适配：当协议定义的编码方式与DEF_FIELD_KB_TO_LEBUF相反时使用
 *
 *          公式对比（以解码为例）：
 *          - DEF_FIELD_KB_TO_LEBUF:     field = (buf_value - b) / k
 *          - DEF_LEBUF_VAR_KB_TO_FIELD: field = (buf_value * k) + b
 *
 *          内存操作：
 *          - 使用memcpy进行数据拷贝，拷贝长度为field和buf_var_type中较小的size
 *          - 支持不同类型之间的转换（如int16_t到float）
 *          - 自动处理类型转换警告（通过DISABLE_WARNING_4244）
 *
 * @param buf_offset 在buf缓冲区中的字节偏移量
 * @param buf_var_type buf中存储的变量类型（如uint8_t、int16_t、uint32_t等）
 * @param k 线性缩放系数（增益/斜率）。若k=0则自动使用k=1，即不缩放
 * @param b 线性偏移量（零点偏移/截距）
 * @param field 结构体实例(inst)中的字段名称
 *
 * @note 1. k值为0时会被自动替换为1，避免除零错误
 * @note 2.
 * 该宏依赖外部变量：is_encode(编码/解码标志)、inst(结构体实例指针)、buf(字节缓冲区)
 * @note 3. 参数顺序与DEF_FIELD_KB_TO_LEBUF不同，field参数在最后
 * @note 4. 涉及不同数值类型转换时需确保k和b的值域合理，避免溢出
 * @note 5. 缓冲区使用小端字节序，跨平台通信时需注意字节序一致性
 *
 * @warning 使用前必须确保buf_offset不会越界访问buf缓冲区
 *
 * @par 使用示例：
 * @code
 *   // 解码int16_t类型的ADC原始值为实际电压(单位：V)
 *   // ADC原始值范围0-4095, 对应0-3.3V, 即 voltage = adc_value * 0.000806 + 0
 *   DEF_LEBUF_VAR_KB_TO_FIELD(0, int16_t, 0.000806, 0, voltage);
 *
 *   // 解码压力传感器数据：buf中uint16_t值转换为实际压力(kPa)
 *   // 传感器输出0-65535对应0-1000kPa，零点偏移100kPa
 *   // pressure = raw * 0.01526 + 100
 *   DEF_LEBUF_VAR_KB_TO_FIELD(2, uint16_t, 0.01526, 100, pressure);
 * @endcode
 */
#define DEF_LEBUF_VAR_KB_TO_FIELD(buf_offset, buf_var_type, k, b, field) \
  do                                                                     \
  {                                                                      \
    float fk = k == 0 ? 1 : k;                                           \
    if (is_encode)                                                       \
    {                                                                    \
      DISABLE_WARNING_4244_BEGIN;                                        \
      buf_var_type temp_val = ((inst->field - b) / fk);                  \
      DISABLE_WARNING_4244_END;                                          \
      memcpy(buf + buf_offset, &temp_val,                                \
             min(sizeof(inst->field), sizeof(buf_var_type)));            \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      buf_var_type temp_val = 0;                                         \
      memcpy(&temp_val, buf + buf_offset,                                \
             min(sizeof(inst->field), sizeof(buf_var_type)));            \
      DISABLE_WARNING_4244_BEGIN;                                        \
      inst->field = ((temp_val * fk) + b);                               \
      DISABLE_WARNING_4244_END;                                          \
    }                                                                    \
  } while (0)

#define END_DEF_MSG() }

/* 在函数定义之前，临时禁用"未使用参数"的警告 */
#if defined(_MSC_VER) // 判断为 Microsoft Visual C++ 编译器
#define DISABLE_WARNING_4244_BEGIN \
  __pragma(warning(push)) __pragma(warning(disable : 4244))
#define DISABLE_WARNING_4244_END __pragma(warning(pop))

#elif defined(__GNUC__) || defined(__clang__) // 判断为 GCC 或 Clang 编译器
// GCC/Clang 中与 C4244 (可能丢失数据的隐式转换) 类似的警告是 -Wconversion 和
// -Wsign-conversion
#define DISABLE_WARNING_4244_BEGIN                       \
  _Pragma("GCC diagnostic push")                         \
      _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
          _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"")
#define DISABLE_WARNING_4244_END _Pragma("GCC diagnostic pop")

#else                              // 其他不支持的编译器
#define DISABLE_WARNING_4244_BEGIN // 定义为空
#define DISABLE_WARNING_4244_END   // 定义为空
#warning "DISABLE_WARNING_4244 macros are not supported for this compiler."
#endif

#endif

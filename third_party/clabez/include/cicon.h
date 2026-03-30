#ifndef C_CORE_ICON_H
#define C_CORE_ICON_H
#ifdef  __cplusplus  
extern "C" {
#endif  

#if defined(_WIN32) || defined(_WIN64)
#  ifndef CC_EXPORTS
#   ifdef CC_STATIC
#    define CC_API
#   else
#    define CC_API __declspec(dllimport)
#   endif
#  else
#    define CC_API __declspec(dllexport)
#  endif
#  define CC_CALL __cdecl
#elif defined(__unix) || defined(__linux)
#ifndef CC_API
#define CC_API __attribute__((visibility("default")))
#endif
#  define CC_CALL
#endif
    /** \addtogroup LabEZ_Icon ICO图标
     * @{
     */

    /**
     * @brief 图标尺寸
     * 
     */
    typedef struct cicon_Size
    {
        int w;  /*!< 宽度 */
        int h;  /*!< 高度 */
    }cicon_Size;

    /**
     * @brief 通过ico文件创建位图
     * 
     * @param icon 图标文件
     * @return 位图
     */
    CC_API void* CC_CALL cicon_create_bitmap(void* icon);

    /**
     * @brief 销毁位图
     * 
     * @param pbitmap 位图指针的指针
     * @return 空
     */
    CC_API void CC_CALL cicon_destroy_bitmap(void** pbitmap);

    /**
     * @brief 获取图标尺寸
     * 
     * @param icon 图标文件
     * @return 尺寸
     */
    CC_API cicon_Size CC_CALL cicon_get_size(void* icon);
    /** @}*/

#ifdef  __cplusplus  
}
#endif  /* end of __cplusplus */  
#endif
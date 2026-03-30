#ifndef C_CORE_DPI_H
#define C_CORE_DPI_H
#ifdef __cplusplus
extern "C"
{
#endif

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
#endif
    /** \addtogroup LabEZ_DPI 高分屏支持
     * @{
     */

    /**
     * @brief 使能dpi
     *
     * @return bool
     */
    CC_API int CC_CALL cdpi_enable();

    /**
     * @brief 获取系统dpi
     *
     * @return CC_API
     */
    CC_API float CC_CALL cdpi_get_system_dpi();

    /**
     * @brief 通过宽高像素描述，获取dpi
     *
     * @param w 宽度
     * @param h 高度
     * @return 比例
     */
    CC_API float CC_CALL cdpi_get_system_dpi_by_wh(int w, int h);

    /**
     * @brief 获取dc的dpi
     *
     * @param dc windows dc
     * @return 比例
     */
    CC_API float CC_CALL cdpi_get_dc_dpi(void *dc);

    /**
     * @brief 通过窗口句柄获取dpi
     *
     * @param hwnd windows hwnd
     * @return 比例
     */
    CC_API float CC_CALL cdpi_get_wnd_dpi(void *hwnd);

    /** @}*/
#ifdef __cplusplus
}
#endif /* end of __cplusplus */
#endif
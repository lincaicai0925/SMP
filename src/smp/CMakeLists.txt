cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)
string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})
PROJECT(${CURRENT_FOLDER} C)

file(GLOB HDR_FILES ${PROJECT_SOURCE_DIR}/*.h)
message("-- Project Source Dir --  " ${PROJECT_SOURCE_DIR})
aux_source_directory(. SRC_FILES)

add_executable(${PROJECT_NAME}
    ${HDR_FILES}
    ${SRC_FILES}
    ${SMP_SOURCES}
)

add_definitions(
    -DUSE_STD_INT_T
    -DCC_STATIC)

# c_lib_demo
if(BUILD_SHARED_LIBS)
else()
    add_definitions(-DHL_STATIC)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../third_party/clabez/include)

#  Windows (MSVC + MinGW 共用 LVGL / SDL2 配置)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_DIR x64)
else()
    set(ARCH_DIR x86)
endif()

if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../out)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # ---------- SDL2 路径 ----------
    if(MSVC)
        set(SDL2_ROOT ${PROJECT_SOURCE_DIR}/../../third_party/SDL2-2.32.10)
        set(SDL2_INCLUDE_DIR ${SDL2_ROOT}/MSVC/include)
        set(SDL2_LIBRARIES
            "${SDL2_ROOT}/MSVC/lib/${ARCH_DIR}/SDL2.lib"
            "${SDL2_ROOT}/MSVC/lib/${ARCH_DIR}/SDL2main.lib"
            winmm imm32 version setupapi cfgmgr32
        )
    else()
        # MinGW
        set(SDL2_ROOT ${PROJECT_SOURCE_DIR}/../../third_party/SDL2-2.32.10/x86_64-w64-mingw32)
        set(SDL2_INCLUDE_DIR ${SDL2_ROOT}/include/)
        set(SDL2_LIBRARIES
            mingw32
            "${SDL2_ROOT}/lib/libSDL2.a"
            "${SDL2_ROOT}/lib/libSDL2main.a"
            winmm imm32 version setupapi cfgmgr32
        )
    endif()
    # ---------- 公共 include ----------
    include_directories(${PROJECT_SOURCE_DIR}/../lvgl_ui/include)
    include_directories(${PROJECT_SOURCE_DIR}/../lvgl_ui/)
    include_directories(${SDL2_INCLUDE_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/../../third_party/)
    include_directories(${PROJECT_SOURCE_DIR}/../rtf/include)
    include_directories(${PROJECT_SOURCE_DIR}/../../third_party/miniz)
    # ---------- LVGL ----------
    set(LV_CONF_PATH ${PROJECT_SOURCE_DIR}/../lvgl_ui/include/lv_conf.h CACHE STRING "" FORCE)
    set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "Disable building lvgl examples" FORCE)
    set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "Disable building lvgl demos" FORCE)

    add_subdirectory(
        ${PROJECT_SOURCE_DIR}/../../third_party/lvgl-9.1.0
        ${CMAKE_BINARY_DIR}/lvgl
    )

    target_include_directories(lvgl PUBLIC ${SDL2_INCLUDE_DIR})

    if(MSVC)
        target_compile_options(lvgl PRIVATE /W0)
    endif()
    # ---------- SMP 源文件 ----------
    file(GLOB_RECURSE SMP_SOURCES
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/*.c
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/hal/hal.c
        ${PROJECT_SOURCE_DIR}/../../fonts/*.c
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/pages/*.c
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/modules/*.c
        ${PROJECT_SOURCE_DIR}/../../third_party/cjson/*.c
        ${PROJECT_SOURCE_DIR}/../rtf/*.c
        ${PROJECT_SOURCE_DIR}/../../third_party/miniz/*.c
    )
    
    target_sources(${PROJECT_NAME} PRIVATE ${SMP_SOURCES})

    # ---------- clabez ----------
    if(MSVC)
        set(CLABEZ_LIB
            "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/clabez/lib/${ARCH_DIR}/clabez-win-${ARCH_DIR}-msvc1900-md-3.2.lib")
    else()
        set(CLABEZ_LIB
            "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/clabez/lib/${ARCH_DIR}/libclabez-win-${ARCH_DIR}-mingw13-seh-3.2.a")
    endif()

    # ---------- 链接 ----------
    target_link_libraries(${PROJECT_NAME} PRIVATE
        lvgl
        lvgl::thorvg
        ${SDL2_LIBRARIES}
        ${CLABEZ_LIB}
        ws2_32
    )

    # ---------- 链接选项 ----------
    if(MSVC)
        target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(${PROJECT_NAME} PRIVATE -mconsole)
    endif()

    # ---------- Debug 模式编译警告 (GCC/MinGW) ----------
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
        message(STATUS "Debug mode enabled")
        target_compile_options(lvgl PRIVATE
            -pedantic-errors
            -Wall
            -Wclobbered
            -Wdeprecated
            -Wdouble-promotion
            -Wempty-body
            -Wextra
            -Wformat-security
            -Wmaybe-uninitialized
            # -Wmissing-prototypes
            -Wpointer-arith
            -Wmultichar
            -Wno-pedantic
            -Wreturn-type
            -Wshadow
            -Wshift-negative-value
            -Wsizeof-pointer-memaccess
            -Wtype-limits
            -Wundef
            -Wuninitialized
            -Wunreachable-code
            -Wfloat-conversion
            -Wstrict-aliasing
        )
    endif()

# ============================================================
#  Linux
# ============================================================
elseif(UNIX AND NOT APPLE)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../out)

    include_directories(${PROJECT_SOURCE_DIR}/../lvgl_ui/include)
    include_directories(${PROJECT_SOURCE_DIR}/../lvgl_ui/)
    include_directories(${PROJECT_SOURCE_DIR}/../../third_party/)

    # ---------- SDL2 (系统安装, 通过 pkg-config 查找) ----------
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})

    # ---------- LVGL ----------
    set(LV_CONF_PATH ${PROJECT_SOURCE_DIR}/../lvgl_ui/include/lv_conf.h CACHE STRING "" FORCE)
    set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
    set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "" FORCE)

    add_subdirectory(
        ${PROJECT_SOURCE_DIR}/../../third_party/lvgl-9.1.0
        ${CMAKE_BINARY_DIR}/lvgl
    )

    # ---------- SMP 源文件 ----------
    file(GLOB_RECURSE SMP_SOURCES
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/*.c
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/hal/hal.c
        ${PROJECT_SOURCE_DIR}/../../fonts/*.c
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/pages/*.c
        ${PROJECT_SOURCE_DIR}/../lvgl_ui/modules/*.c
        ${PROJECT_SOURCE_DIR}/../../third_party/cjson/*.c
    )
    target_sources(${PROJECT_NAME} PRIVATE ${SMP_SOURCES})

    # ---------- 链接 ----------
    target_link_libraries(${PROJECT_NAME} PRIVATE
        lvgl
        lvgl::thorvg
        ${SDL2_LIBRARIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/clabez/lib/x64/libclabez-linux-x64-gcc9-none-3.2.a
        X11
        dl
        m
        pthread
    )

    # ---------- Debug 警告 ----------
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(lvgl PRIVATE
            -pedantic-errors -Wall -Wextra
            -Wdouble-promotion -Wformat-security
            -Wshadow -Wundef -Wuninitialized
            -Wno-pedantic
        )
    endif()

    # 复制报告模板到 exe 输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/template"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/../template/report_template_v2.docx"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/template/report_template_v2.docx"
    COMMENT "Copying report template"
)
#  其他平台
else()
    target_link_libraries(${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/clabez/lib/x64/libclabez-linux-x64-gcc9-none-3.2.a
    )
endif()

# update version
if(MSVC)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        PRE_BUILD
        COMMAND ../../labez_devl prebuild -t $<CONFIG> -r ${CMAKE_CURRENT_BINARY_DIR}/hl_version_info.res
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_CURRENT_BINARY_DIR}/hl_version_info.res")
endif()


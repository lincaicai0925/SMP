<!-- GSD:project-start source:PROJECT.md -->
## Project

**SMP — 数据字典页面 Linear 风格改造**

SMP 是一个基于 LVGL v9.1 的嵌入式监控管理平台，通过 Modbus 协议与设备通信，提供仪表板、数据字典、日志管理、拓扑显示等功能页面。本次项目聚焦于将数据字典页面的视觉风格从当前设计全面改造为 Linear 设计语言。

**Core Value:** 数据字典页面在视觉上完全符合 Linear 设计语言——克制、高密度、工程感，同时不破坏任何现有功能。

### Constraints

- **Tech Stack**: LVGL v9.1 + C 语言，不能升级 LVGL 版本
- **No Hover**: LVGL v9.1 无 hover 事件支持，用 pressed 态替代
- **Input Radius**: 输入框圆角 = 0px（用户明确要求），其他元素按 Linear 规范 6px
- **Visual Only**: 不改动任何功能逻辑，只改样式相关代码
- **Scope**: 仅数据字典页面 + 其配置弹窗
<!-- GSD:project-end -->

<!-- GSD:stack-start source:codebase/STACK.md -->
## Technology Stack

## Languages
- C (C89/C99) - All application code, UI, modules, pages, HAL
- Python - Test data generation script (`out/conf/generate_test.py`)
## Runtime
- Desktop application (Windows primary, Linux secondary)
- Single-threaded LVGL event loop with auxiliary worker threads (via clabez `cthread`)
- None (all dependencies vendored in `third_party/`)
## Build System
- Top-level: `src/CMakeLists.txt` - project root, iterates subdirectories
- Application: `src/smp/CMakeLists.txt` - main executable build, links all libraries
- LVGL: `third_party/lvgl-9.1.0/CMakeLists.txt` - built as subdirectory target
- MinGW-w64 GCC (primary, Windows) - path: `C:\cjl\dev_tools\mingw64\`
- MSVC 2015+ (alternative, Windows)
- GCC 9+ (Linux)
## Frameworks
- LVGL 9.1.0 - Embedded graphics library for UI rendering (`third_party/lvgl-9.1.0/`)
- SDL2 2.32.10 - Display backend and input handling (`third_party/SDL2-2.32.10/`)
- CMake 3.31 - Build system generator
- labez_devl - Custom pre-build tool for version info (`labez_devl.exe` at project root)
## Key Dependencies
- Graphics framework, the entire UI is built on LVGL widgets
- Configuration: `src/lvgl_ui/include/lv_conf.h`
- Key settings:
- Window creation, mouse/keyboard input, display rendering
- Windows: pre-built static libraries (MinGW and MSVC variants)
- Linux: system-installed via pkg-config
- LVGL SDL driver enabled (`LV_USE_SDL 1`)
- Proprietary C utility library by hanlandtech (headers + static lib, no source)
- Pre-built static libraries for: Windows x64 MinGW, Windows x64 MSVC, Linux x64 GCC
- Used modules (by include):
- Also provides: TCP/UDP clients, IPC, logging, timers, memory pool, serialization, etc. (headers present but not all used)
- Lightweight JSON parser (source included: `cJSON.c`, `cJSON.h`)
- Used for configuration file I/O (dashboard layout, data dictionary config)
- Modbus RTU/TCP communication library (headers + `libmodbus.so`)
- Headers present: `modbus.h`, `modbus-rtu.h`, `modbus-tcp.h`
- Currently NOT linked in CMakeLists.txt - only Linux .so available
- Data dictionary stores `modbus_addr` fields but actual Modbus communication not yet integrated
- Custom math library (headers + static lib)
## Custom Fonts
- FangZheng KaiTi Simplified Chinese: 12px, 16px, 24px, 36px (`lv_font_founder_kaiti_simplified_*.c`)
- Digital-7 numeric display font: 16px, 24px, 36px (`lv_font_digital_7_*.c`)
- Source TTFs: `digital-7.ttf`, `fangzheng_kaiti.ttf`, `simkai.ttf`
## Configuration
- `out/conf/tabview.json` - Dashboard widget layout (positions, chart series, LED/slider/button configs)
- `out/conf/data_dict_conf.json` - Data dictionary entries (bus addresses, device names, data types)
- `out/conf/data.json` - Additional data configuration
- `src/CMakeLists.txt` - Top-level project config
- `src/smp/CMakeLists.txt` - Executable build config with platform-specific paths
- `src/lvgl_ui/include/lv_conf.h` - LVGL feature/driver configuration
- `src/set_local_env.cmake` - Optional local environment overrides (not committed)
- `USE_STD_INT_T` - Use standard integer types
- `CC_STATIC` / `HL_STATIC` - Static linking mode
- `_CRT_SECURE_NO_WARNINGS` - Windows CRT warning suppression
## Platform Requirements
- MinGW-w64 GCC or MSVC 2015+
- CMake 3.5.2+
- SDL2 (vendored)
- Resolution: 1920x1080 default window
- GCC 9+
- CMake 3.5.2+
- SDL2 (system-installed via pkg-config)
- X11, pthread, dl, m
- Windows desktop application (`smp.exe`)
- Inno Setup installer for distribution
- Ships with: config JSON files, image assets (`out/img/`), fonts
## System Libraries Linked
- `ws2_32` - Winsock2 (network support, linked but minimal current usage)
- `winmm`, `imm32`, `version`, `setupapi`, `cfgmgr32` - SDL2 dependencies
- `mingw32` - MinGW runtime (MinGW builds only)
- `X11` - X Window System
- `dl` - Dynamic loading
- `m` - Math library
- `pthread` - POSIX threads
<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->
## Conventions

## Naming Patterns
- Source files: `snake_case.c` (e.g., `main_window.c`, `data_distribution.c`, `chart_setting_pop.c`)
- Header files: `snake_case.h` matching the source file name
- Module headers collected in `include/` subdirectory (e.g., `modules/include/button.h`, `pages/include/dashboard.h`)
- Public/API functions: `Pascal_Case_With_Underscores` (e.g., `Create_Button()`, `Chart_Delete()`, `Main_Window_Create()`, `Data_Map_Init()`)
- Static/private functions: mixed convention -- some use `Pascal_Case_With_Underscores` (e.g., `Arrange_Dialog_Close()`), some use `snake_case` (e.g., `init_btn_style()`, `generate_signal()`, `btn_config_confirmed()`)
- Event callbacks: suffix `_Cb` or `_cb` (e.g., `Led_Delete_Cb()`, `Palette_Event_Cb()`, `checkbox_cb()`, `prev_cb()`)
- LVGL HAL functions: `snake_case` following LVGL convention (e.g., `sdl_hal_init()`)
- Prescriptive rule: **Use `Pascal_Case_With_Underscores` for all public functions. Use `snake_case` for file-static helpers. Use `_Cb` suffix (Pascal_Case) for event callbacks.**
- Local variables: `snake_case` (e.g., `sleep_time_ms`, `chart_param`, `base_obj`)
- Static module-level: `s_` prefix with `PascalCase` or `snake_case` (e.g., `s_btn_trans`, `s_btn_style`, `s_Arrange_Dialog`, `s_Ta_Rows`, `s_Last_Rows`)
- Global variables: `g_` prefix (e.g., `g_smp_ctx`)
- Struct members: `snake_case` (e.g., `point_cnt`, `chart_bg_color`, `series_color`)
- Prescriptive rule: **Use `s_` prefix for file-static variables. Use `g_` prefix for globals. Use `snake_case` for all variable names.**
- Structs/typedefs: `snake_case_t` suffix (e.g., `smp_ctx_t`, `chart_dsc_t`, `btn_conf_t`, `chart_param_dsc_t`, `led_param_dsc_t`)
- Enums: `UPPER_SNAKE_CASE` for values (e.g., `NAV_PAGE_DASHBOARD`, `WIDGET_TYPE_CHART`, `CHART_TYPE_LINE`)
- Named enum types: `PascalCase_t` (e.g., `Nav_Page_t`, `Palette_Item_t`) or `snake_case_t` (e.g., `scatter_marker_type_t`, `chart_type_t`)
- Callback typedefs: `snake_case` with `_fun` or `_Fn` suffix (e.g., `series_bind_fun`, `Widget_Save_Fn`, `Arrange_Cb_t`)
- Prescriptive rule: **All type names end with `_t`. Enum values use `UPPER_SNAKE_CASE`. Struct typedefs use `snake_case_t`.**
- `UPPER_SNAKE_CASE` (e.g., `CHART_SERIES_NUM`, `BTN_NAME_MAX_LEN`, `NAVIGATION_BAR_WIDTH`, `HANDLE_SIZE`)
- Color macros: descriptive name with context (e.g., `DIALOG_BG_COLOR`, `STYLE_ACCENT`, `STYLE_BTN_DANGER`)
- Feature toggles: `_ENABLE` suffix with 0/1 (e.g., `CHART_FPS_ENABLE`, `THEME_CHANGE_ENABLE`)
## Code Style
- No `.clang-format` or `.editorconfig` detected -- formatting is manual
- Indentation: 4 spaces (consistent across all files)
- Brace style: opening brace on same line for `if`/`for`/`while`, next line for function definitions
- Single-line bodies sometimes omit braces: `if(dsc) lv_free(dsc);`
- Prescriptive rule: **Use 4-space indentation. Place opening brace on next line for function definitions, same line for control flow. Always use braces for multi-line blocks.**
- GCC/MinGW with extensive warnings in Debug mode (see `src/smp/CMakeLists.txt` lines 118-143)
- Includes `-Wall -Wextra -Wshadow -Wundef -Wdouble-promotion -Wfloat-conversion`
- No static analysis tool (no clang-tidy config)
## Import Organization
- Always use relative paths from the source file's location or from include directories
- Include directories configured in CMake: `src/lvgl_ui/include/`, `src/lvgl_ui/`, `third_party/`
- Prescriptive rule: **Include `"lvgl.h"` first in headers. In source files, include the corresponding header first, then standard libs, then third-party, then project headers.**
- None. All includes use relative filesystem paths.
## Header Guard Pattern
- `MAIN_WINDOWS_H` for `main_window.h`
- `_CHART_H_` for `chart.h` (leading/trailing underscore)
- `BUTTON_H` for `button.h`
- `SMP_PRIVATE_H` for `smp_private.h`
## Error Handling
- NULL-check on function parameters at entry: `if (!parent || !param) return NULL;`
- NULL-check on memory allocation: `if (!dsc) return NULL;`
- Use `LV_ASSERT_NULL()` for critical assertions (from LVGL)
- Custom assert handler in `src/smp/main.c`: `My_Assert_Handler()` prints file/line/function and calls `exit(2)`
- No error codes or error propagation system -- functions return `NULL` on failure or `bool` for success/failure
## Logging
- Direct `printf()` for assert messages and debug output
- No structured logging framework
- Performance profiling via custom `My_Profiler_Init()` in `src/lvgl_ui/modules/profiler.c`
## Comments
- Chinese (Simplified) comments are the primary documentation language
- Block comments (`/* */`) for struct member documentation, inline with member declaration
- Line comments (`//`) for TODOs, quick notes, and disabled code
- Doxygen-style `@brief`, `@param`, `@return`, `@note`, `@example` used in well-documented modules (see `src/lvgl_ui/modules/include/chart.h`, `src/lvgl_ui/modules/include/profiler.h`)
- Section headers use decorative box comments in LVGL-style:
## Function Design
- Creation functions return pointer to descriptor struct (e.g., `chart_dsc_t*`, `led_dsc_t*`, `btn_dsc_t*`) or `NULL` on failure
- Simple widget creation returns `lv_obj_t*`
- Boolean return for success/failure on setter functions
## Module Design
- Public functions and types declared in headers under `modules/include/` or `pages/include/`
- No barrel files -- each module has its own header
- Use `lv_malloc()` / `lv_free()` / `lv_memzero()` (LVGL allocator) for UI-related allocations
- Use standard `malloc()` / `free()` for non-UI data only
- Register `LV_EVENT_DELETE` callback on parent LVGL object to ensure cleanup
- Use `static bool s_xxx_inited` flag pattern for one-time initialization of shared resources (styles, transitions)
## Page Module Pattern
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->
## Architecture

## Pattern Overview
- Monolithic C application with LVGL as the UI framework, SDL2 as the display/input backend
- Page-based navigation: a left sidebar selects one of 7 pages, each page owns its own content area
- Address-mapped data distribution: widgets subscribe to virtual addresses (`vaddr`) to receive real-time data from a background thread
- Multi-threaded: a data injection thread produces values, chart modules each spawn their own data-processing thread, UI updates happen on the main LVGL timer thread
- JSON-based persistence for widget layouts and data dictionary configuration
## Layers
- Purpose: Initialize display, mouse, and keyboard input devices via SDL2
- Location: `src/lvgl_ui/hal/hal.c`, `src/lvgl_ui/hal/hal.h`
- Contains: `sdl_hal_init()` - creates SDL2 window, mouse, keyboard input devices
- Depends on: LVGL SDL2 driver (`src/drivers/sdl/lv_sdl_window.h`), SDL2 library
- Used by: `src/smp/main.c`
- Purpose: Application shell - creates the navigation bar, top bar with window controls, and content area; manages page switching
- Location: `src/lvgl_ui/main_window.c`, `src/lvgl_ui/include/main_window.h`
- Contains: `Main_Window_Create()`, navigation bar creation, image preloading, page container creation, window control buttons (minimize/maximize/close)
- Depends on: HAL layer, all page modules, data distribution, `smp_private.h` (global context `smp_ctx_t`)
- Used by: `src/smp/main.c`
- Key global: `smp_ctx_t *g_smp_ctx` - the central application context holding all UI state
- Purpose: Individual application pages, each implementing a specific feature
- Location: `src/lvgl_ui/pages/`
- Contains: Page initialization functions following the `Xxx_Weight(smp_ctx_t *ctx)` pattern
- Depends on: Modules layer, data distribution, global context
- Used by: Main window (called during `Main_Window_Create()`)
- Purpose: Self-contained UI widget components that can be placed on the dashboard canvas
- Location: `src/lvgl_ui/modules/`
- Contains: Widget creation functions (`Create_Xxx`), save/load functions for JSON persistence, configuration dialogs
- Depends on: LVGL, data distribution, cJSON, clabez containers
- Used by: Dashboard page primarily
- Purpose: Map virtual addresses to widgets, distribute real-time data from background threads to UI widgets
- Location: `src/lvgl_ui/data_distribution.c`, `src/lvgl_ui/include/data_distribution.h`
- Contains: Address-to-widget-list map (`cmap<uint32_t, clist<widget_data*>>`), test data injection thread
- Depends on: clabez containers (`cmap`, `clist`, `cthread`), MPMC queue
- Used by: All data-bound widgets (chart, LED, button, slider)
- Purpose: External libraries used by the application
- Location: `third_party/`
- Contains:
## Data Flow
- Global application state: `smp_ctx_t *g_smp_ctx` (heap-allocated, initialized in `SMP_Init`)
- Per-page state: `smp_page_t` structs in `g_smp_ctx->pages[]` array
- Widget state: each widget type has its own descriptor struct (`chart_dsc_t`, `led_dsc_t`, `btn_dsc_t`)
- Data binding state: global `cmap_addr_map_clist_t *addr_map`
- Dashboard editor state: static `s_Editor` struct + `clist_ewt *ew_list` linked list
## Key Abstractions
- Purpose: Central state container for the entire application
- Defined in: `src/lvgl_ui/include/smp_private.h`
- Pattern: Global singleton (`g_smp_ctx`), heap-allocated
- Contains: image descriptors, UI object pointers, page array, window data, theme state
- Purpose: Bridge between data distribution and UI widgets; each bound widget allocates one
- Defined in: `src/lvgl_ui/include/data_distribution.h`
- Pattern: Allocated per-binding, stored in the address map; contains either a float value or an MPMC queue pair
- Two modes: `VALUE_TYPE_QUEUE` (for charts needing buffered streaming) and `VALUE_TYPE_OTHER` (for simple float display)
- Purpose: Wraps any widget type on the dashboard canvas with drag/resize/delete handles and responsive layout
- Defined in: `src/lvgl_ui/pages/dashboard/dashboard.c`
- Pattern: Stored in a linked list (`ew_list`); holds percentage-based position/size for responsive layout
- Contains: LVGL wrapper object, resize handle, delete handle, widget type ID, type-specific descriptor pointer
- Purpose: Dispatch table for widget save/load operations and default parameters
- Defined in: `src/lvgl_ui/pages/dashboard/dashboard.c`
- Pattern: Static const array indexed by `WIDGET_TYPE_*` enum; enables polymorphic behavior in C
- Contains: save function, load function, default parameter pointer, parameter struct size
- Purpose: Complete runtime state for a chart instance including threads, buffers, series data
- Defined in: `src/lvgl_ui/modules/include/chart.h`
- Pattern: Heap-allocated by `Create_Chart()`, owns its own data-processing thread and synchronization primitives
- Contains: double-buffered pixel arrays, series data with circular buffers, LTTB downsampling arrays, thread handles, mutexes, semaphores
## Entry Points
- Location: `src/smp/main.c` - `main()`
- Triggers: OS process start
- Responsibilities: Initialize LVGL, initialize SDL2 HAL (1920x1080), create main window, run the LVGL event loop (`lv_timer_handler()`)
- Each page has a `Xxx_Weight(smp_ctx_t *ctx)` function called once from `Main_Window_Create()`
- `Dashboard_Weight()` at `src/lvgl_ui/pages/dashboard/dashboard.c`
- `Data_Dictionary_Weight()` at `src/lvgl_ui/pages/data_dictionary/data_dictionary.c`
- `Topology_Display_Weight()` at `src/lvgl_ui/pages/topology_display.c`
- `Experimental_Data_Weight()` at `src/lvgl_ui/pages/experimental_data.c`
- `Log_Managment_Weight()` at `src/lvgl_ui/pages/log_manage.c`
- `Data_Map_Init()` at `src/lvgl_ui/data_distribution.c` - called from `Main_Window_Create()`, starts the test injection thread
## Error Handling
- `LV_ASSERT_MALLOC(ptr)` after every `lv_malloc` call - triggers `My_Assert_Handler` which prints and calls `exit(2)`
- `LV_ASSERT_NULL(ptr)` for null pointer checks
- Factory functions (`Create_Chart`, `Create_Led`, etc.) return `NULL` on allocation failure
- Resource cleanup via `LV_EVENT_DELETE` callbacks registered on LVGL objects - when an object is deleted, its callback frees associated heap memory
## Cross-Cutting Concerns
- Main thread: LVGL event loop + UI rendering (single-threaded LVGL requirement)
- Data injection thread: `Test_Inject_Thread` in `data_distribution.c` - produces test data
- Per-chart data threads: each `chart_dsc_t` spawns `Handle_Data` thread for pixel rendering
- Synchronization: MPMC queues for chart data flow; timed mutexes and counting semaphores for chart buffer management; no synchronization on `addr_map` access (known race condition)
- Dashboard widget layout: `./conf/tabview.json` (loaded/saved by dashboard page)
- Data dictionary: `./data_dict_conf.json` or user-selected JSON file
- Images: PNG files loaded from `./img/` directory relative to executable
<!-- GSD:architecture-end -->

<!-- GSD:workflow-start source:GSD defaults -->
## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd:quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd:debug` for investigation and bug fixing
- `/gsd:execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->



<!-- GSD:profile-start -->
## Developer Profile

> Profile not yet configured. Run `/gsd:profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->

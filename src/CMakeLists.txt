cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)
string(REGEX REPLACE "(/$)|(/src$)|(/src/)" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" PROJ_NAME ${CURRENT_FOLDER_ABSOLUTE})
project(${PROJ_NAME}  C)
message(STATUS "www.hanlandtech.com")

option(BUILD_MSVC_WITH_MT "Build with mt api in msvc" OFF)
option(BUILD_SHARED_LIBS "build shared libs" OFF)

# if(POLICY CMP0177)
#     cmake_policy(SET CMP0177 NEW)
# endif()
#------为VS设置全局参数
if(MSVC)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MD")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MD")

    foreach(CompilerFlag ${CompilerFlags})
        if(BUILD_MSVC_WITH_MT)
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        else()
            string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
        endif()
        set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "set compiler flags" FORCE)
        message("compiler flags: ${CompilerFlag}:${${CompilerFlag}}")
    endforeach()
endif()

#包含本地环境变量文件
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/set_local_env.cmake")
	MESSAGE(STATUS "Set local env by set_local_env.cmake")
    include(set_local_env.cmake)
endif()

option(USE_DIST_DIR_AS_INSTALL "set install dir" ON)
option(USE_OUT_DIR_AS_OUTPUT "set out dir" ON)

if(USE_DIST_DIR_AS_INSTALL)
    set(CMAKE_INSTALL_PREFIX ../dist/${LABEZ_ARCH})
endif()

if(USE_OUT_DIR_AS_OUTPUT)
    set(OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../out)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUT_DIR}/${LABEZ_ARCH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUT_DIR}/${LABEZ_ARCH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}/${LABEZ_ARCH}")
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY:" ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY:" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY:" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB_RECURSE sub_projects ${CMAKE_CURRENT_LIST_DIR}/*/CMakeLists.txt)
foreach(proj ${sub_projects})
  string(REGEX REPLACE ".*/(.*)/CMakeLists.txt" "\\1" sub_project ${proj})
  add_subdirectory(${sub_project})
  
  if(${CMAKE_VERSION} VERSION_GREATER "3.7")
    set_target_properties(${sub_project} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dist")
  endif()
endforeach()

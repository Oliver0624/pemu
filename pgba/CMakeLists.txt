cmake_minimum_required(VERSION 3.0)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(pgba)

##################
# mGBA
##################
set(LIBMGBA_ONLY ON CACHE BOOL "")
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(${CMAKE_SOURCE_DIR}/cores/mgba ${CMAKE_CURRENT_BINARY_DIR}/mgba-build)
set_property(TARGET mgba APPEND PROPERTY COMPILE_DEFINITIONS
        "_GNU_SOURCE;IOAPI_NO_64;USE_LIBZIP;HAVE_CRC32;COLOR_16_BIT;COLOR_5_6_5;DISABLE_THREADING"
)
if (PLATFORM_VITA)
    set_property(TARGET mgba APPEND PROPERTY COMPILE_DEFINITIONS "PSP2")
endif ()

##################
# pgba
##################
set(PGBA_INC sources)
file(GLOB PGBA_SRC sources/*.c*)
list(APPEND PGBA_SRC
        ${CMAKE_SOURCE_DIR}/cores/mgba/src/util/vfs/vfs-fd.c
        ${CMAKE_SOURCE_DIR}/cores/mgba/src/util/vfs/vfs-dirent.c
        ${CMAKE_SOURCE_DIR}/cores/mgba/src/util/vfs/vfs-zip.c
)
set(PGBA_CFLAGS
        -D__PGBA__ -D__PGBA_VERSION_MAJOR__=${VERSION_MAJOR} -D__PGBA_VERSION_MINOR__=${VERSION_MINOR}
)

#####################
# PLATFORM SPECIFIC
#####################
if (PLATFORM_VITA)
    set(TITLE_ID "PGBA00001" CACHE STRING "" FORCE)
    list(REMOVE_ITEM PGBA_SRC ${CMAKE_SOURCE_DIR}/cores/mgba/src/util/vfs/vfs-dirent.c)
    list(APPEND PGBA_SRC ${CMAKE_SOURCE_DIR}/cores/mgba/src/platform/psp2/sce-vfs.c)
    list(APPEND PGBA_CFLAGS -DPSP2 -D__HAIKU__)
elseif (PLATFORM_PS4)
    set(PS4_PKG_TITLE "pGBA - GBA Simulator with mGBA core")
    set(PS4_PKG_TITLE_ID "PGBA00001")
    set(PS4_PKG_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
    list(APPEND PGBA_CFLAGS -DHAVE_SYS_PARAM_H)
elseif (PLATFORM_SWITCH)
    set_property(TARGET mgba PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET mgba APPEND PROPERTY INCLUDE_DIRECTORIES ${DEVKITPRO}/portlibs/switch/include)
elseif (PLATFORM_WINDOWS)
    list(APPEND PGBA_INC ${CMAKE_CURRENT_SOURCE_DIR}/sources/windows)
elseif (PLATFORM_ANDROID)
    if (ANDROID_ABI STREQUAL armeabi-v7a)
        list(APPEND PGBA_CFLAGS -marm -mcpu=cortex-a7 -mfpu=neon -ffunction-sections -fomit-frame-pointer)
    endif ()
    list(APPEND PGBA_CFLAGS -DHAVE_SYS_PARAM_H)
endif ()

#####################
# pgba executable
#####################
add_executable(${PROJECT_NAME} ${PGBA_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${PGBA_INC})
target_compile_options(${PROJECT_NAME} PRIVATE ${PGBA_CFLAGS})
target_link_libraries(${PROJECT_NAME} cross2dui mgba)

include(${CMAKE_SOURCE_DIR}/libcross2d/cmake/targets.cmake)

add_custom_target(${PROJECT_NAME}.data.skin
        COMMAND ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/../data/common/datadir -D DST=${CMAKE_CURRENT_BINARY_DIR}/data_datadir -P ${CMAKE_CURRENT_LIST_DIR}/../libcross2d/cmake/copy_directory_custom.cmake
        COMMAND ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/../data/${TARGET_PLATFORM}/datadir -D DST=${CMAKE_CURRENT_BINARY_DIR}/data_datadir -P ${CMAKE_CURRENT_LIST_DIR}/../libcross2d/cmake/copy_directory_custom.cmake
        COMMAND ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/../data/common/romfs -D DST=${CMAKE_CURRENT_BINARY_DIR}/data_romfs -P ${CMAKE_CURRENT_LIST_DIR}/../libcross2d/cmake/copy_directory_custom.cmake
        COMMAND ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/../data/${TARGET_PLATFORM}/romfs -D DST=${CMAKE_CURRENT_BINARY_DIR}/data_romfs -P ${CMAKE_CURRENT_LIST_DIR}/../libcross2d/cmake/copy_directory_custom.cmake
)
add_dependencies(${PROJECT_NAME}.data.skin ${PROJECT_NAME}.data)
if (PLATFORM_LINUX)
    add_dependencies(${PROJECT_NAME}-romfs ${PROJECT_NAME}.data.skin)
else ()
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data.skin)
endif ()

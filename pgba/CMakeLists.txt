cmake_minimum_required(VERSION 3.0)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(pgba)

##############
# mGBA CFLAGS
# #############
set(MGBA_CFLAGS
        -DMGBA_VERSION="0.10.3"  # mGBA version
        -D__PGBA__ -D__PGBA_VERSION_MAJOR__=${VERSION_MAJOR} -D__PGBA_VERSION_MINOR__=${VERSION_MINOR} # pgba version
        # -Wno-unused-definition -Wno-unused-local-typedefs
        # -Wno-sign-compare -Wno-unused-variable
        # -Wno-delete-non-virtual-dtor -Wno-reorder
        # -Wno-parentheses -Wno-unknown-pragmas
        # -Wno-format-truncation -Wno-unused-but-set-variable
        # -Wno-format-zero-length
        )

#####################
# PLATORM SPECIFIC
#####################
if (PLATFORM_VITA) # PS Vita
    set(TITLE_ID "PGBA00001" CACHE STRING "" FORCE)
    list(APPEND MGBA_CFLAGS -Dstrcasecmp=strcmp)
    set(MGBA_LDFLAGS ScePower_stub)
elseif (PLATFORM_PS4) # PlayStation 4
    set(PS4_PKG_TITLE "pGBA - GBA Simulator with mGBA core")
    set(PS4_PKG_TITLE_ID "PGBA00001")
    set(PS4_PKG_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
elseif (PLATFORM_SWITCH) # Switch
elseif (PLATFORM_3DS) # 3DS
    list(APPEND MGBA_CFLAGS -Dstrcasecmp=strcmp)
elseif (PLATFORM_LINUX) # Linux
elseif (PLATFORM_WINDOWS)
elseif (PLATFORM_ANDROID)
endif (PLATFORM_VITA)

#####################
# mGBA libretro static library
#####################
set(MGBA_LIB mgba_libretro_static)

# Create build directory
set(MGBA_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/mgba_libretro)
file(MAKE_DIRECTORY ${MGBA_BIN_DIR})

# set DEFINES to build libretro only
set(MGBA_LIBRETRO_DEFINES
    -DBUILD_LIBRETRO=ON
    -DBUILD_STATIC=ON
    -DBUILD_SHARED=OFF
    -DBUILD_QT=OFF
    -DBUILD_SDL=OFF
    -DBUILD_GL=OFF
    -DBUILD_GLES2=OFF
    -DBUILD_GLES3=OFF
    -DUSE_SQLITE3=OFF
    -DUSE_FFMPEG=OFF
    -DUSE_PNG=OFF
    -DENABLE_SCRIPTING=OFF
    -DUSE_DISCORD_RPC=OFF
    -DUSE_DEBUGGERS=OFF
    -DUSE_GDB_STUB=OFF
    -DUSE_LZMA=ON
    -DUSE_ZLIB=ON
    -DUSE_MINIZIP=OFF
    -DUSE_LIBZIP=OFF
)

# Use platform specific toolchain to build mGBA core
if(PLATFORM_SWITCH)
    list(APPEND MGBA_LIBRETRO_DEFINES -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cores/mgba/src/platform/switch/CMakeToolchain.txt)
elseif(PLATFORM_3DS)
    list(APPEND MGBA_LIBRETRO_DEFINES -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cores/mgba/src/platform/3ds/CMakeToolchain.txt)
elseif(PLATFORM_VITA)
    list(APPEND MGBA_LIBRETRO_DEFINES -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cores/mgba/src/platform/psp2/CMakeToolchain.vitasdk)
endif()

# backup mgba CMakeLists.txt
set(MGBA_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cores/mgba)
configure_file("${MGBA_CMAKE_DIR}/CMakeLists.txt" "${MGBA_CMAKE_DIR}/CMakeLists.txt.bak" COPYONLY)

# append our cmake to mgba CMakeLists.txt to add static libretro target
file(READ "sources/libretro/mgba_retro_static.cmake" SOURCE_CONTENT)
file(APPEND "${MGBA_CMAKE_DIR}/CMakeLists.txt" "\n\n${SOURCE_CONTENT}")

# call cmake command to generate makefile for mGBA core
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${MGBA_LIBRETRO_DEFINES} ${CMAKE_SOURCE_DIR}/cores/mgba
    WORKING_DIRECTORY ${MGBA_BIN_DIR}
)

# restore mgba orign cmake
file(REMOVE "${MGBA_CMAKE_DIR}/CMakeLists.txt")
file(RENAME "${MGBA_CMAKE_DIR}/CMakeLists.txt.bak" "${MGBA_CMAKE_DIR}/CMakeLists.txt")

# Add a custom target for building mGBA core and libretro static lib
add_custom_target(MGBA_LIBRETRO ALL
    COMMAND ${CMAKE_COMMAND} --build . --target ${MGBA_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy ${MGBA_LIB}.a ${CMAKE_CURRENT_BINARY_DIR}/lib${MGBA_LIB}.a
    WORKING_DIRECTORY ${MGBA_BIN_DIR}
    COMMENT "Building mGBA libretro static library"
)

#####################
# pgba executable
#####################
# add main executable (elf)
set(PGBA_INC
    sources
    sources/libretro
    ${CMAKE_SOURCE_DIR}/cores/mgba/include
)
file(GLOB PGBA_SRC sources/*.c*)
add_executable(${PROJECT_NAME} ${PGBA_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${PGBA_INC})
target_compile_options(${PROJECT_NAME} PRIVATE ${MGBA_CFLAGS})
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${MGBA_BIN_DIR})
target_link_libraries(${PROJECT_NAME} cross2dui ${MGBA_LIB} ${MGBA_LDFLAGS})
add_dependencies(${PROJECT_NAME} MGBA_LIBRETRO)

# include libcross2d targets
include(${CMAKE_SOURCE_DIR}/libcross2d/cmake/targets.cmake)

# add common skin data
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
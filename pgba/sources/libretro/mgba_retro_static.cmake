# This file will be appended to CMakeLists.txt of mGBA repository
# to add our libretro static library
# It will be executed seperately before build pgba program

# set home dir of our sources
set(PGBA_LIBRETRO_DIR ${CMAKE_SOURCE_DIR}/../../pgba/sources/libretro)

# use our libretro headers and sources
include_directories(BEFORE ${PGBA_LIBRETRO_DIR})
file(GLOB RETRO_SRC ${PGBA_LIBRETRO_DIR}/*.c)

# add static libretro target using our codes, should define USE_LZMA and USE_MINIZIP
set(MGBA_RETRO_STATIC mgba_libretro_static)
add_library(${MGBA_RETRO_STATIC} STATIC ${CORE_SRC} ${VFS_SRC} ${RETRO_SRC})
add_dependencies(${MGBA_RETRO_STATIC} ${BINARY_NAME}-version-info)
set_target_properties(${MGBA_RETRO_STATIC} PROPERTIES PREFIX "" COMPILE_DEFINITIONS "__LIBRETRO__;USE_LZMA;USE_MINIZIP;COLOR_16_BIT;COLOR_5_6_5;DISABLE_THREADING;MGBA_STANDALONE;${OS_DEFINES};${FUNCTION_DEFINES};MINIMAL_CORE=2")
target_link_libraries(${MGBA_RETRO_STATIC} ${OS_LIB} ${DEPENDENCY_LIB})

cmake_minimum_required(VERSION 3.0)
project(mm_gps)

# CMAKE_C_FLAGS
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE TARGET_PLATFORM)
string(REGEX MATCH i586-poky-linux IS_POKY_LINUX ${TARGET_PLATFORM})
if(IS_POKY_LINUX)
  message(STATUS "Building for i586-poky-linux (Edison) platform")
  add_definitions(-DPOKY_LINUX=1)
endif()

# FILES
set(SOURCE_DIR mm_gps_c)
include_directories(include)
file(GLOB LIB_SOURCES "${SOURCE_DIR}/mm_gps*.c")

# TARGETS
add_compile_options(-std=c11)

add_library(mm_gps_static STATIC ${LIB_SOURCES})
add_library(mm_gps SHARED ${LIB_SOURCES})
add_executable(mm_gps_test ${SOURCE_DIR}/main.c)
target_link_libraries(mm_gps_test mm_gps_static)
# install(TARGETS mm_gps_test DESTINATION bin)
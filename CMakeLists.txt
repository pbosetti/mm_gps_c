cmake_minimum_required(VERSION 3.0)
project(mm_gps)

# CMAKE_C_FLAGS
set(CMAKE_C_FLAGS "-std=c99")
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE TARGET_PLATFORM)
string(REGEX MATCH i586-poky-linux IS_POKY_LINUX ${TARGET_PLATFORM})
if(IS_POKY_LINUX)
  message(STATUS "Building for i586-poky-linux (Edison) platform")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPOKY_LINUX")
endif()

# FILES
set(SOURCE_DIR mm_gps_c)
file(GLOB SOURCES "${SOURCE_DIR}/mm_gps*.c")

# TARGETS
add_library(mm_gps_static STATIC ${SOURCES})
add_library(mm_gps SHARED ${SOURCES})
add_executable(mm_gps_test ${SOURCE_DIR}/main.c)
target_link_libraries(mm_gps_test mm_gps_static)
# install(TARGETS mm_gps_test DESTINATION bin)
cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED On)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

set (TARGET_NAME top-left-rule)
project (${TARGET_NAME})

include_directories(
  ${SDL2_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../bresenham-generator/
  ${CMAKE_CURRENT_SOURCE_DIR}/../scanline-rasterizer/
  ${CMAKE_CURRENT_SOURCE_DIR}/../scanline-rasterizer-dumb/
  ${CMAKE_CURRENT_SOURCE_DIR}/../pit-rasterizer/
)

add_executable(
  ${TARGET_NAME}
  main.cpp
  ../scanline-rasterizer/scanline-rasterizer.cpp
  ../scanline-rasterizer/scanline-rasterizer.h
  ../scanline-rasterizer/srtl.cpp
  ../scanline-rasterizer/srtl.h
  ../scanline-rasterizer-dumb/srd.cpp
  ../scanline-rasterizer-dumb/srd.h
  ../pit-rasterizer/pit-rasterizer.h
  ../pit-rasterizer/pit-rasterizer.cpp
  ../../types.cpp
  ../../sw3d.cpp
  ../bresenham-generator/blg.cpp
)

if (WIN32)
  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIRS})
  target_link_libraries(${TARGET_NAME} ${MINGW32_LIBRARY}
                                         ${SDL2MAIN_LIBRARY}
                                         ${SDL2_LIBRARY})
else()
  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIRS})
  target_link_libraries(${TARGET_NAME} SDL2)
endif()

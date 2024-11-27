cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED On)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

set (TARGET_NAME rasterizer-bugs)
project (${TARGET_NAME})

include_directories(
  ${SDL2_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../bresenham-generator/
  ${CMAKE_CURRENT_SOURCE_DIR}/../scanline-rasterizer/
  ${CMAKE_CURRENT_SOURCE_DIR}/../pit-rasterizer/
  ${CMAKE_CURRENT_SOURCE_DIR}/../pit-rasterizer-tlr/
  ${CMAKE_CURRENT_SOURCE_DIR}/../scanline-rasterizer-chili/
  ${CMAKE_CURRENT_SOURCE_DIR}/../scanline-rasterizer-dumb/
)

add_executable(
  ${TARGET_NAME}
  main.cpp
  ../scanline-rasterizer/scanline-rasterizer.cpp
  ../scanline-rasterizer/scanline-rasterizer.h
  ../scanline-rasterizer/srtl.cpp
  ../scanline-rasterizer/srtl.h
  ../pit-rasterizer/pit-rasterizer.h
  ../pit-rasterizer/pit-rasterizer.cpp
  ../pit-rasterizer-tlr/pit-rasterizer-tlr.h
  ../pit-rasterizer-tlr/pit-rasterizer-tlr.cpp
  ../scanline-rasterizer-chili/srtl-chili.h
  ../scanline-rasterizer-chili/srtl-chili.cpp
  ../scanline-rasterizer-dumb/srd.h
  ../scanline-rasterizer-dumb/srd.cpp
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

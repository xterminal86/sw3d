cmake_minimum_required(VERSION 2.6)
set(TARGET_NAME sw-3d)
project(${TARGET_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${TARGET_NAME} ${SOURCES})

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

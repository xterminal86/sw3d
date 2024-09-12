cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED On)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

set (TARGET_NAME bresenham-generator)
project (${TARGET_NAME})

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_executable(${TARGET_NAME}
  main.cpp
  ../../types.cpp
  ../../sw3d.cpp
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

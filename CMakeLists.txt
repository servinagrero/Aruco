cmake_minimum_required(VERSION 3.10)

project(Aruco)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenCV REQUIRED)

add_executable(Aruco src/main.cpp)
install(TARGETS Aruco DESTINATION bin)

target_link_libraries(Aruco ${OpenCV_LIBS})

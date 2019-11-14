cmake_minimum_required(VERSION 3.10)

project(Aruco)

find_package(OpenCV REQUIRED)

add_executable(Aruco src/main.cpp)

target_link_libraries(Aruco ${OpenCV_LIBS})

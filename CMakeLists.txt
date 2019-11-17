cmake_minimum_required(VERSION 3.10)

project(Aruco)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#[[
set(CMAKE_CXX_FLAGS_DEBUG "put your flags")
set(CMAKE_CXX_FLAGS_MINSIZEREL "put your flags")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "put your flags")
set(CMAKE_CXX_FLAGS_RELEASE "put your flags")
]]
find_package(OpenCV REQUIRED)

add_executable(Aruco src/main.cpp)
install(TARGETS Aruco DESTINATION bin)
target_compile_options(Aruco PRIVATE -Wall -Wextra -pedantic -Werror)


target_link_libraries(Aruco ${OpenCV_LIBS})

cmake_minimum_required(VERSION 3.10)
project(RpgAviacao CXX)
 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
add_compile_options(-Wall -Wextra)
 
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()
 
add_executable(meu_exe src/main.cpp)
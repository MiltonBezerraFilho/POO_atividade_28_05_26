cmake_minimum_required(VERSION 3.10)
project(RpgAviacao CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# Necessario para std::thread / std::async (Questao 3-C/D)
find_package(Threads REQUIRED)

add_executable(meu_exe src/main.cpp)
target_link_libraries(meu_exe PRIVATE Threads::Threads)
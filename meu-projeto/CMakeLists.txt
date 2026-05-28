cmake_minimum_required(VERSION 3.15)
project(EcommerceSystem LANGUAGES CXX)

#Para usar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Ativar avisos rigorosos 
add_compile_options(-Wall -Wextra)

#Sinaliza o local do código principal
add_executable(meu_projeto src/main.cpp)
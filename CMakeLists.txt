cmake_minimum_required(VERSION 3.17)
project(SO2-TP)

set(CMAKE_CXX_STANDARD 20)

# activa o unicode para o TCHAR
add_compile_definitions("UNICODE" "_UNICODE")

# faz com que o msvc leia todos os source files como utf-8
add_compile_options("/utf-8")#16?

add_executable(SO2-TP-aviao src/aviao.cpp)
add_executable(SO2-TP-control src/control.cpp)
add_executable(SO2-TP-passag src/passag.cpp)
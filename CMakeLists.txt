cmake_minimum_required(VERSION 3.17)
project(SO2-TP)

set(CMAKE_CXX_STANDARD 20)

# activa o unicode para o TCHAR
add_compile_definitions("UNICODE" "_UNICODE")

# faz com que o msvc leia todos os source files como utf-8
add_compile_options("/utf-8")


include_directories(src)

add_executable(SO2-TP-control 
                src/control/control.cpp
                src/control/startup.cpp)

add_executable(SO2-TP-aviao src/aviao.cpp)

add_executable(SO2-TP-passag src/passag.cpp)

add_executable(exemplo_consumer_producer src/exemplo_consumer_producer.c)
add_executable(exemplo1_consumer src/exemplo1_consumer.c)
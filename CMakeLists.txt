cmake_minimum_required(VERSION 3.17)
project(SO2-TP)

set(CMAKE_CXX_STANDARD 20)

# activa o unicode para o TCHAR
add_compile_definitions("UNICODE" "_UNICODE")
add_compile_definitions("_DEBUG")
# faz com que o msvc leia todos os source files como utf-8
add_compile_options("/utf-8")

include_directories(src)

set(control_core src/control/control.cpp
        src/control/startup.cpp)

set(control_menu src/control/menu.cpp)

add_executable(SO2-TP-control src/control/main.cpp
        ${control_core})

add_executable(SO2-TP-aviao src/aviao/aviao.cpp)

add_executable(SO2-TP-passag src/passag.cpp)

add_executable(exemplo_consumer src/exemplo_consumer.c)
add_executable(exemplo_producer src/exemplo_producer.c)
add_executable(exemplo_registry src/exemplo_registry.c)

#tests
add_executable(test src/test.cpp)
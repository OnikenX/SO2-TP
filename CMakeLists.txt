cmake_minimum_required(VERSION 3.17)
project(SO2-TP)

set(CMAKE_CXX_STANDARD 20)

# activa o unicode para o TCHAR
#add_compile_definitions("UNICODE" "_UNICODE")
# true unicode utf_8
#add_compile_definitions("UTF8_UNICODE")
#deifine for esperimental pings
add_compile_definitions("PINGS")
# define especial para debugging
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_definitions("_DEBUG")
endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")



# faz com que o msvc leia todos os source files como utf-8
add_compile_options("/utf-8")

include_directories(src)

set(control_core src/control/control.cpp
        src/control/startup.cpp)
set(control_menu src/control/menu.cpp)
set(utils src/utils.cpp)
add_executable(SO2-TP-control src/control/main.cpp
        ${control_core} ${control_menu} ${utils} src/shared_control_aviao.cpp src/shared_control_aviao.hpp)
set(aviao_core src/aviao/aviao.cpp src/aviao/main.cpp src/aviao/startup.cpp)
set(aviao_menu src/aviao/menu.cpp)
add_executable(SO2-TP-aviao  ${aviao_core} ${aviao_menu} ${utils} src/shared_control_aviao.cpp src/shared_control_aviao.hpp)

add_executable(SO2-TP-passag src/passag.cpp)

add_executable(exemplo_consumer src/exemplo_consumer.c)
add_executable(exemplo_producer src/exemplo_producer.c)

#tests
add_executable(test src/test.cpp)
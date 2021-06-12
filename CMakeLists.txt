cmake_minimum_required(VERSION 3.17)
project(SO2-TP)

set(CMAKE_CXX_STANDARD 20)

# activa o unicode para o TCHAR
add_compile_definitions("UNICODE" "_UNICODE")

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
set(control_gui src/control/ControlerGrafico.cpp)
set(shared_control_aviao src/shared_control_aviao.cpp)
add_executable(SO2-TP-control src/control/main.cpp
        ${control_core} ${control_menu} ${utils} src/shared_control_aviao.cpp src/shared_control_aviao.hpp)
add_executable(SO2-TP-control-GUI ${control_gui})
target_sources(SO2-TP-control-GUI PRIVATE src/control/ControlerGrafico.rc src/control/arrow.cur src/control/small.ico)
#set_property(TARGET SO2-TP-control-GUI PROPERTY WIN32_EXECUTABLE)

set(aviao_core src/aviao/aviao.cpp src/aviao/main.cpp src/aviao/startup.cpp)
set(aviao_menu src/aviao/menu.cpp)
add_executable(SO2-TP-aviao ${aviao_core} ${aviao_menu} ${utils} ${shared_control_aviao})

add_executable(SO2-TP-passag src/passageiro/passag.cpp src/passageiro/main.cpp)

add_executable(exemplo_consumer src/exemplo_consumer.c)
add_executable(exemplo_producer src/exemplo_producer.c)

#tests
add_executable(test src/test.cpp)
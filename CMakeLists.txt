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
#    add_compile_definitions("_DEBUG_PINGS")
endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")


# faz com que o msvc leia todos os source files como utf-8
add_compile_options("/utf-8")

include_directories(src)

#grupos gerais
set(utils src/utils.cpp)

#grupos do control
set(control_core src/control/control.cpp
        src/control/startup.cpp)
set(control_menu src/control/menu.cpp )
set(control_gui src/control/gui/ControlerGrafico.cpp)
set(shared_control_aviao src/shared_control_aviao.cpp)
set(control_shared_components ${shared_control_aviao} ${utils} ${control_core}
        src/shared_control_aviao.cpp src/shared_control_passageiro.cpp src/control/pipeserver.cpp)
#executaveis do control
add_executable(SO2-TP-control src/control/main.cpp  ${control_menu}  ${control_shared_components})
add_executable(SO2-TP-control-GUI ${control_shared_components} ${control_gui})
target_sources(SO2-TP-control-GUI PRIVATE src/control/gui/control.rc src/control/gui/arrow.cur src/control/gui/control.ico src/control/gui/small.ico)
#set_property(TARGET SO2-TP-control-GUI PROPERTY WIN32_EXECUTABLE)

#cenas do aviao
set(aviao_core src/aviao/aviao.cpp src/aviao/main.cpp src/aviao/startup.cpp)
set(aviao_menu src/aviao/menu.cpp src/shared_control_passageiro.cpp src/shared_control_passageiro.hpp)
add_executable(SO2-TP-aviao ${aviao_core} ${aviao_menu} ${utils} ${shared_control_aviao})

#cenas do passageiro
set(passag_core src/passageiro/passag.cpp src/passageiro/main.cpp src/passageiro/startup.cpp src/passageiro/pipeclient.cpp)
add_executable(SO2-TP-passag ${passag_core} ${utils})

#executaveis dos exemplos
add_executable(example_client src/example_client.cpp)
add_executable(example_server src/example_server.cpp)

#tests
add_executable(test src/test.cpp)
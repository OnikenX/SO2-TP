//
// Created by OnikenX on 5/15/2021.
//
#pragma once

#include <utils.hpp>
#include "aviao.hpp"

struct AviaoInstance {
    SharedMemoryMap_control *sharedMemoryMap;
    HANDLE hMapFile;
    void *dllHandle;
    FARPROC ptr_move_func;
    unsigned long id_do_aeroporto;

    bool verifica_criacao_com_control();
    bool em_andamento;
    AviaoShare aviao;

    AviaoInstance(const AviaoInstance &) = delete; // non construction-copyable
    AviaoInstance &operator=(const AviaoInstance &) = delete; // non copyable
    AviaoInstance(HANDLE hMapFile, SharedMemoryMap_control *sharedMemoryMap, void *dllHandle,
                  AviaoInstance aviao);
    static std::optional<std::unique_ptr<AviaoInstance>>
    create(AviaoInstance aviao);
    int move(int cur_x, int cur_y, int final_dest_x, int final_dest_y, int *next_x, int *next_y);
    int run();
    ~AviaoInstance();
};


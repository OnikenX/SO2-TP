//
// Created by OnikenX on 5/15/2021.
//
#pragma once

#include <utils.hpp>
#include "aviao.hpp"

struct AviaoInstance {
    SharedMemoryMap *sharedMemoryMap;
    HANDLE hMapFile;
    void *dllHandle;
    FARPROC ptr_move_func;
    TCHAR *nome_do_aeroporto;

    bool verifica_criacao_com_control();

    int velocidade;
    int cap_max;
    unsigned long id;

    Semaforos( const Semaforos& ) = delete; // non construction-copyable
    Semaforos& operator=( const Semaforos& ) = delete; // non copyable

    AviaoInstance(HANDLE hMapFile, SharedMemoryMap *sharedMemoryMap, void *dllHandle,
                  TCHAR *nome_do_aeroporto, int velocidade, int cap_max);

    static std::optional<std::unique_ptr<AviaoInstance>>
    create(TCHAR *nome_do_aeroporto, int velocidade, int cap_max);

    int move(int cur_x, int cur_y, int final_dest_x, int final_dest_y, int *next_x, int *next_y);

    ~AviaoInstance();
};



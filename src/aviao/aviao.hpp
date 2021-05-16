//
// Created by OnikenX on 5/15/2021.
//
#pragma once

#include <utils.hpp>
#include "aviao.hpp"

//strutura de assistencia
struct AviaoSharedObjects_aviao {
    AviaoSharedObjects_aviao(HANDLE mutex_mensagens, HANDLE mutex_produtor,
                             HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                             Mensagem_Aviao *sharedMensagemAviao);

    static std::unique_ptr<AviaoSharedObjects_aviao> create();

    HANDLE mutex_mensagens, mutex_produtor, semaforo_write, semaforo_read, filemap, evento_morte;
    Mensagem_Aviao *sharedMensagemAviao;

    ~AviaoSharedObjects_aviao();
};

struct AviaoInstance {
    //memoria partilhada do control
    SharedMemoryMap_control *sharedMemoryMap;
    //handle da memoria partinhada
    HANDLE hMapFile;
    //handle da dll
    void *dllHandle;
    FARPROC ptr_move_func;
    unsigned long id_do_aeroporto;

    bool verifica_criacao_com_control();

    bool em_andamento;
    AviaoShare aviao;

    AviaoInstance(const AviaoInstance &) = delete; // non construction-copyable
    AviaoInstance &operator=(const AviaoInstance &) = delete; // non copyable
    AviaoInstance(HANDLE hMapFile, SharedMemoryMap_control *sharedMemoryMap, void *dllHandle,
                  AviaoShare av, std::unique_ptr<AviaoSharedObjects_aviao> sharedComs);

    std::unique_ptr<AviaoSharedObjects_aviao> sharedComs;

    static std::unique_ptr<AviaoInstance> create(AviaoShare av);

    int move(int cur_x, int cur_y, int final_dest_x, int final_dest_y, int *next_x, int *next_y);

    int run();

    ~AviaoInstance();

    std::unique_ptr<Mensagem_Aviao> sendMessage(bool recebeResposta, Mensagem_Control &mensagemControl);

private:

};


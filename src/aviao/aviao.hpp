//
// Created by OnikenX on 5/15/2021.
//
#pragma once

#include <utils.hpp>
#include <shared_control_aviao.hpp>
//strutura dos handles de partinha de informação com o control(exclusivo do aviaoInfo)
struct AviaoSharedObjects_aviao {
    AviaoSharedObjects_aviao(HANDLE mutex_mensagens, HANDLE mutex_produtor,
                             HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                             Mensagem_Aviao *sharedMensagemAviao, HANDLE mutex_em_andamento);

    static std::unique_ptr<AviaoSharedObjects_aviao> create(unsigned long id_aviao);
    HANDLE mutex_mensagens, mutex_produtor, semaforo_write, semaforo_read, filemap, mutex_em_andamento;
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
    HANDLE Wait_to_Die;
    FARPROC ptr_move_func;
    unsigned long id_do_aeroporto;
    AviaoInfo aviaoInfo;
    bool verifica_criacao_com_control();
    bool em_andamento;
    AviaoInstance(const AviaoInstance &) = delete; // non construction-copyable
    AviaoInstance &operator=(const AviaoInstance &) = delete; // non copyable
    AviaoInstance(HANDLE hMapFile, SharedMemoryMap_control *sharedMemoryMap, void *dllHandle,
                  AviaoInfo av, std::unique_ptr<AviaoSharedObjects_aviao> sharedComs,
                  unsigned long id_do_aeroporto);
    std::unique_ptr<AviaoSharedObjects_aviao> sharedComs;
    static std::unique_ptr<AviaoInstance> create(AviaoInfo av);
    int move(int cur_x, int cur_y, int final_dest_x, int final_dest_y, int *next_x, int *next_y) const;
    int run();
    ~AviaoInstance();
    std::unique_ptr<Mensagem_Aviao> sendMessage(bool recebeResposta, Mensagem_Control_aviao &mensagemControl);
    void suicidio();
};


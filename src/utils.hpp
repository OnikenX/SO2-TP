
#pragma once

//standard c++
#include <memory>
#include <sstream>
#include <iostream>
#include <sstream>
#include <optional>//RUST Team

//windows and tchar
#include <windows.h>
#include <tchar.h>

//set mode for the terminal
#include <io.h>
#include <fcntl.h>

//typedef for tchar to std types
#ifdef UNICODE
#define tcout std::wcout
#define tcerr std::wcerr
#define tcin std::wcin
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcerr std::cerr
#define tcin std::cin
#define tstring std::string
#define tstringstream std::stringstream
#endif

//defines

// shortener for the TEXT macro
#define t(quote) TEXT(quote)
#define SHARED_MEMORY_NAME t("SO2_TP_SHARED_MEMORY")
#define SEMAFORO_READ_CONTROL_AVIAO t("SO2_TP_SEMAFORO_READ_CONTROL_AVIAO")
#define SEMAFORO_WRITE_CONTROL_AVIAO t("SO2_TP_SEMAFORO_WRITE_CONTROL_AVIAO")
#define SEMAFORO_READ_AVIAO_CONTROL t("SO2_TP_SEMAFORO_READ_AVIAO_CONTROL")
#define SEMAFORO_WRITE_AVIAO_CONTROL t("SO2_TP_SEMAFORO_WRITE_AVIAO_CONTROL")
#define MUTEX_PARTILHADO t("SO2_TP_MUTEX_PARTINHADO_BUFFER")
#define EVENT_KILLER t("Nos_Vamos_Todos_Falecer")

struct SharedLocks {
    HANDLE semaforo_read_control_aviao;
    HANDLE semaforo_write_control_aviao;
    HANDLE mutex_partilhado;
    static SharedLocks *get();

    //define se o singleton acabou de ser criado
    bool firsttime;
    bool erros;

    SharedLocks(const SharedLocks &) = delete; // non construction-copyable
    SharedLocks &operator=(const SharedLocks &) = delete; // non copyable
    ~SharedLocks();

    SharedLocks();

private:
    void closeall();
};

#define CIRCULAR_BUFFERS_SIZE 50

#define MAX_LENGTH_NAME_AEROPORTO 30
//erros comuns

#define ERRO_ARGUMENTOS 1
#define ERRO_CONTROL_EXISTE 2
#define ERRO_CONTROL_NAO_EXISTE 3
#define ERRO_AVIAO_NAO_FOI_CRIADO 4

//structs partilhadas

struct Cords {
    int x;
    int y;
    bool isEqual(Cords &outro);
};

struct AviaoShare {
    unsigned long IDAv;
    int CapMax;
    int velocidade;
    Cords PosA;
    Cords PosDest;
};

struct Aeroporto {
    Cords pos;
    TCHAR nome[MAX_LENGTH_NAME_AEROPORTO];
    unsigned long IDAero;
};
/*
struct Buffer_Circular{
    AviaoInstance av[10];
    int posE; //proxima posicao de escrita
    int posL; //proxima posicao de leitura
    int nProdutores;
    int nConsumidores;

};

struct Passa_Thread{
    Buffer_Circular* bc;
    HANDLE hWrite;
    HANDLE hRead;
    int terminar;
};

*/


enum Mensagem_types{
    confirmar_novo_aviao,
    alterar_coords,
    novo_destino,
    suicidio

};

struct Pedido_mover{
    unsigned int x;
    unsigned int y;
};

struct Pedido_info_aeroporto{
    unsigned long id_aeroporto;
};


//union dos possiveis dados a enviar
union Mensagem_Control_union{
    Pedido_info_aeroporto pedidoConfirmarNovoAviao;
    Pedido_mover pedidoConfirmarMovimento;


};


//as mensagens que sao enviadas para o control pelo aviaoInstance
struct Mensagem_Control {
    Mensagem_types type;
    unsigned long id_aviao;
    Mensagem_Control_union mensagem;
};

struct Resposta_Mover{
    bool permissao;
};

struct Resposta_Novas_Coordenadas{
    unsigned int x;
    unsigned int y;
};


union Mensagem_Aviao_union{
    Resposta_Mover respostaMover;
    Resposta_Novas_Coordenadas respostaNovasCoordenadas;

};
enum Mensagem_resposta{
    ok,
    movimento_sucess,
    movimento_fail,
    kill_me,
    aeroporto_nao_existe
    //aviao_existe
};



struct Mensagem_Aviao {
//    Mensagem_types type;
    Mensagem_resposta resposta;
    Mensagem_Aviao_union msg;
};



//memoria partinha do control
struct SharedMemoryMap_control {
    bool se_pode_criar_mais_avioes;
    //numero de avioes em execução
    int nAvioes;
    bool terminar;
    //buffer circular das mensagens que vao do aviaoInstance para o control
    Mensagem_Control buffer_mensagens_control[CIRCULAR_BUFFERS_SIZE];
    int posReader, posWriter;
    SharedMemoryMap_control();
};


//struct DadosThreads {
//    SharedMemoryMap_control *memPar; //ponteiro para a memoria partilhada
//    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
//    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
//    HANDLE hMutex;
//    int terminar; // 1 para sair, 0 em caso contr�rio
//    int id;
//};


struct GuardLock {
    ~GuardLock();
    HANDLE mutex;
    GuardLock(HANDLE _mutex);
    GuardLock(GuardLock&& guard);
};






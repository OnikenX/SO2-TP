
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
};

struct Aviao {
    unsigned long IDAv;
    int CapMax;
    int velocidade;
    Cords PosA;
    Cords PosDest;
};

struct Aeroporto {
    Cords pos;
    TCHAR nome[MAX_LENGTH_NAME_AEROPORTO];
    int IDAero;
};
/*
struct Buffer_Circular{
    Aviao av[10];
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
enum MenuActionsToControl {
    EncerrarSistema,
    CriarAeroporto,
    ToggleAceitarAvioes
};

struct Mensagem_Control {
    MenuActionsToControl controlaction;
};

struct Mensagem_Aviao {

};


//memoria partinha do control
struct SharedMemoryMap_control {
    bool se_pode_criar_mais_avioes;
    //numero de avioes em execução
    int nAvioes;
    bool terminar;
    //buffer circular das mensagens que vao do aviao para o control
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






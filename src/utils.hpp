
#pragma once

//standard c++
#include <memory>
#include <sstream>
#include <iostream>
#include <sstream>
#include <optional>//RUST Team
#include <chrono>
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
#define MUTEX_PARTILHADO t("SO2_TP_MUTEX_PARTINHADO_BUFFER")
#define EVENT_KILLER t("Nos_Vamos_Todos_Falecer")
#define FM_AVIAO t("S02_TP_FM_%lu")
#define SW_AVIAO t("S02_TP_SW_%lu")
#define SR_AVIAO t("S02_TP_SR_%lu")
#define MT_AVIAO t("S02_TP_MT_%lu")

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
    bool isEqual(Cords &outro) const;
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

enum Mensagem_types{
    confirmar_novo_aviao,
    alterar_coords,
    novo_destino,
    suicidio,
    ping

};

struct Pedido_mover{
    unsigned int x;
    unsigned int y;
};

struct Pedido_info_aeroporto{
    unsigned long id_aeroporto;
    AviaoShare av;
};




//union dos possiveis dados a enviar
union Mensagem_Control_union{
    Pedido_info_aeroporto pedidoConfirmarNovoAviao;
    Pedido_mover pedidoConfirmarMovimento;

};


//as mensagens que sao enviadas para o control pelo aviaoInstance
struct Mensagem_Control {//o aviao envia isto
    Mensagem_Control() = default;

    Mensagem_types type;
    unsigned long id_aviao;
    Mensagem_Control_union mensagem;
};


struct Resposta_Novas_Coordenadas{
    unsigned int x;
    unsigned int y;
};


union Mensagem_Aviao_union{
    Resposta_Novas_Coordenadas respostaNovasCoordenadas;

};
enum Mensagem_resposta{
    lol_ok,//mensagem de confirmação/OK
    movimento_fail,// nao pode fazer movimento x
    kill_me,//suicinho quentinho
    aeroporto_nao_existe,//precisas de um dicionario?
    MAX_Atingido,//max power
    Porta_Fechada//cant enter, cant go
};



struct Mensagem_Aviao { //aviao recebe
//    Mensagem_types type;
    Mensagem_resposta resposta_type;
    Mensagem_Aviao_union msg_content;
};



//memoria partinha do control
struct SharedMemoryMap_control {
//    bool se_pode_criar_mais_avioes;
    //numero de avioes em execução
//    int nAvioes;
//    bool terminar;
    //buffer circular das mensagens que vao do aviaoInstance para o control
    Mensagem_Control buffer_mensagens_control[CIRCULAR_BUFFERS_SIZE];
    int posReader, posWriter;
    SharedMemoryMap_control();
};

struct GuardLock {
    ~GuardLock();
    HANDLE mutex;
    explicit GuardLock(HANDLE _mutex);
    GuardLock(GuardLock&& guard) noexcept ;
};

struct CriticalSectionGuard{
    ~CriticalSectionGuard();
    CRITICAL_SECTION  &criticalSection;
    explicit CriticalSectionGuard(CRITICAL_SECTION  &criticalSection);
};
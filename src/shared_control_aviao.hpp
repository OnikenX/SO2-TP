//
// Created by OnikenX on 6/2/2021.
//

#ifndef SO2_TP_SHARED_CONTROL_AVIAO_HPP
#define SO2_TP_SHARED_CONTROL_AVIAO_HPP

#include "utils.hpp"

struct shared_control_aviao {
    HANDLE semaforo_read_control_aviao;
    HANDLE semaforo_write_control_aviao;
    HANDLE mutex_partilhado;
    HANDLE evento_JackTheReaper;
    static shared_control_aviao *get();

    //define se o singleton acabou de ser criado
    bool firsttime;
    bool erros;

    shared_control_aviao(const shared_control_aviao &) = delete; // non construction-copyable
    shared_control_aviao &operator=(const shared_control_aviao &) = delete; // non copyable
    ~shared_control_aviao();

    shared_control_aviao();

private:
    void closeall();
};

#define SHARED_MEMORY_NAME t("SO2_TP_SHARED_MEMORY")
#define SEMAFORO_READ_CONTROL_AVIAO t("SO2_TP_SEMAFORO_READ_CONTROL_AVIAO")
#define SEMAFORO_WRITE_CONTROL_AVIAO t("SO2_TP_SEMAFORO_WRITE_CONTROL_AVIAO")
#define MUTEX_PARTILHADO t("SO2_TP_MUTEX_PARTINHADO_BUFFER")
#define FM_AVIAO t("S02_TP_FM_%lu")
#define SW_AVIAO t("S02_TP_SW_%lu")
#define SR_AVIAO t("S02_TP_SR_%lu")
#define MT_AVIAO t("S02_TP_MT_%lu")

#define CIRCULAR_BUFFERS_SIZE 50
#define MAX_LENGTH_NAME_AEROPORTO 30

//structs partilhadas
struct Cords {
    unsigned int x;
    unsigned int y;

    bool isEqual(Cords &outro) const;
};

struct AviaoInfo {
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

enum class Mensagem_aviao_types {
    confirmar_novo_aviao,
    alterar_coords,
    novo_destino,
    suicidio,
    ping
};

struct Pedido_mover {
    unsigned int x;
    unsigned int y;
};

struct Pedido_info_aeroporto {
    unsigned long id_aeroporto;
    AviaoInfo av;
};


//union dos possiveis dados a enviar do control para o aviao
union Mensagem_Control_aviao_union {
    Pedido_info_aeroporto pedidoConfirmarNovoAviao;
    Pedido_mover pedidoConfirmarMovimento;
};


//as mensagens que sao enviadas para o control pelo aviaoInstance
struct Mensagem_Control_aviao {//o aviaoInfo envia isto
    Mensagem_Control_aviao() = default;
    Mensagem_aviao_types type;
    unsigned long id_aviao;
    Mensagem_Control_aviao_union mensagem;
};


struct Resposta_Novas_Coordenadas {
    unsigned int x;
    unsigned int y;
};


union Mensagem_Aviao_union {
    Resposta_Novas_Coordenadas respostaNovasCoordenadas;
};

//respostas que o control envia
enum class Mensagem_aviao_resposta {
    lol_ok,//mensagem de confirmação/OK
    movimento_fail,// nao pode fazer movimento x
    kill_me,//suicinho quentinho
    aeroporto_nao_existe,//precisas de um dicionario?
    MAX_Atingido,//max power
    Porta_Fechada//cant enter, cant go
};


struct Mensagem_Aviao { //aviaoInfo recebe
//    Mensagem_aviao_types request_type;
    Mensagem_aviao_resposta resposta_type;
    Mensagem_Aviao_union msg_content;
};


//memoria partinha do control
struct SharedMemoryMap_control {
//    bool se_pode_criar_mais_avioes;
    //numero de avioes em execução
//    int nAvioes;
//    bool terminar;
    //buffer circular das mensagens que vao do aviaoInstance para o control
    Mensagem_Control_aviao buffer_mensagens_control[CIRCULAR_BUFFERS_SIZE];
    int posReader, posWriter;

    SharedMemoryMap_control();
};

#endif //SO2_TP_SHARED_CONTROL_AVIAO_HPP

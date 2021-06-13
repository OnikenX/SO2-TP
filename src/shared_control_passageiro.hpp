//
// Created by OnikenX on 6/12/2021.
//
#pragma once
#ifndef SO2_TP_SHARED_CONTROL_PASSAGEIRO_HPP
#define SO2_TP_SHARED_CONTROL_PASSAGEIRO_HPP

#include "utils.hpp"

#define NAMEDPIPE_NAME TEXT("\\\\.\\pipe\\SO2-TP-NAMEDPIPE")

//informação basica de um passageiro
struct PassageiroInfo {
    long id_aeroporto_origem;
    long id_aeroporto_destino;
    long tempo_para_embarcar;
};


union Mensagem_Passageiro_union{
    PassageiroInfo passageiroInfo;
};

enum class Mensagem_passageiro_request_type : uint8_t{
    confirmacao
};

enum class Mensagem_passageiro_response_type : uint8_t {
    lol_ok,//mensagem de confirmação/OK
    aeroporto_nao_existe,
    embarcado,
    timeout,
    desembarcado,
};
//mensagens que devem ser enviadas para o passageiro
struct Mensagem_Passageiro_response{
    Mensagem_passageiro_response_type resposta_type;
};
//mensagens enviadas para o control pelo passageiro
struct Mensagem_Passageiro_request{
    Mensagem_passageiro_request_type request_type;
    Mensagem_Passageiro_union msg_content;
};

#endif //SO2_TP_SHARED_CONTROL_PASSAGEIRO_HPP

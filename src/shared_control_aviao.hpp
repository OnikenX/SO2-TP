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


#endif //SO2_TP_SHARED_CONTROL_AVIAO_HPP

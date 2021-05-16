#pragma once

#include "control.hpp"

class Menu
{
public:
    Menu(Control &control);
    void run();
    Menu(const Menu &) = delete; // non construction-copyable
    Menu &operator=(const Menu &) = delete; // non copyable
private:
    int counter_avioes;
    int counter_aeroporto;
    Control& control;
    void cria_aeroporto();
    void consulta_aeroporto();
    void consultar_aviao();
    void mata_tudo();
    void desativa_novos_avioes();
};


struct AviaoSharedObjects_control {
    AviaoSharedObjects_control(HANDLE mutex, HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                               Mensagem_Aviao *sharedMensagemAviao);
    static std::unique_ptr<AviaoSharedObjects_control> AviaoSharedObjects_control::create(unsigned long id_aviao);

    HANDLE mutex, semaforo_write, semaforo_read, filemap;
    Mensagem_Aviao *sharedMensagemAviao;
    ~AviaoSharedObjects_control();
};

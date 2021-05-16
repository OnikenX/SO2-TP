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
#pragma once

#include "control.hpp"

class Menu
{
public:
    Menu(Control &control);
    void run();
    Menu(const Menu &) = delete; // non construction-copyable
    Menu &operator=(const Menu &) = delete; // non copyable
    Control& control;
    int counter_aeroporto;
private:
    void cria_aeroporto();
    void consulta_aeroporto();
    void consultar_aviao();
    void desativa_novos_avioes();
};




#pragma once

#include "control.hpp"

class Menu
{
public:
    Menu(Control &control);
    void run();
private:
    int counter_avioes;
    Control& control;
    void cria_aeroporto();
    void consulta_aeroporto();
    void consultar_aviao();
    void mata_tudo();
    void desativa_novos_avioes();
}
#pragma once

#include "aviao.hpp"

class Menu
{
public:
    Menu(AviaoInstance &aviao);
    void run();
    Menu(const Menu &) = delete; // non construction-copyable
    Menu &operator=(const Menu &) = delete; // non copyable
private:
    int counter_avioes;
    int counter_aeroporto;
    AviaoInstance& aviaoInstance;
    HANDLE mutex_em_andamento;
    void novas_cords();
    void inicia_voo();
    void suicidio();
};
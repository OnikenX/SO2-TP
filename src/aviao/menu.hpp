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
    int counter_aeroporto;
    AviaoInstance& aviaoInstance;
    void novas_cords();
    void inicia_voo();
    void suicidio();
    void embarcar_passageiros();
};
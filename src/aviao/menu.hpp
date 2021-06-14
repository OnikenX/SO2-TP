#pragma once

#include "aviao.hpp"

struct Menu
{
    Menu(AviaoInstance &aviao);
    void run();
    Menu(const Menu &) = delete; // non construction-copyable
    Menu &operator=(const Menu &) = delete; // non copyable
    int counter_aeroporto;
    AviaoInstance& aviaoInstance;
    void novo_destino();
    void inicia_voo();
    void suicidio();
    void embarcar_passageiros() const;
};
#pragma once

#include "./../utils.h"
#include <vector>

#define MAX_SQUARE_MAP 1000

struct Control {
    const DWORD MAX_AVIOES;
    const DWORD MAX_AEROPORTOS;

    // caso exista problemas a criar o control o optional não ira retornar um control
    static std::optional<std::unique_ptr<Control>> create(DWORD max_avioes = 50, DWORD max_aeroportos = 10);

    // main function do control
    int run();

    ~Control();

    static bool setup_do_registry(DWORD &max_avioes, DWORD &max_aeroportos);

    //variables to destroy
    HANDLE shared_memory_handle;
    LPTSTR view_of_file_pointer;

    void notifica_tudo();

    Control(DWORD max_avioes, DWORD max_aeroportos, HANDLE shared_memory_handle,
            LPTSTR view_of_file_pointer);

    std::vector<Aeroporto> aeroportos;
    std::vector<Aviao> avioes;
    bool aceita_avioes;

    void finalizar();
};

#pragma once

#include "./../utils.hpp"
#include <vector>
#include <unordered_map>



struct Control {
    const DWORD MAX_AVIOES;
    const DWORD MAX_AEROPORTOS;
    bool terminar;
    //mutex_produtor para mexer na data do control
    HANDLE mutex_interno;
    // caso exista problemas a criar o control o optional não ira retornar um control
    static std::optional<std::unique_ptr<Control>> create(DWORD max_avioes = 50, DWORD max_aeroportos = 10);

    // main function do control
    int run();

    //mata evento mandar todos dar um tiro nos cornos
    HANDLE evento_JackTheReaper;

    Control( const Control& ) = delete; // non construction-copyable
    Control& operator=( const Control& ) = delete; // non copyable

    ~Control();

    static bool setup_do_registry(DWORD &max_avioes, DWORD &max_aeroportos);

    //variables to destroy
    HANDLE shared_memory_handle;
    SharedMemoryMap_control* view_of_file_pointer;

    void liberta_o_jack();

    Control(DWORD max_avioes, DWORD max_aeroportos, HANDLE shared_memory_handle,
            SharedMemoryMap_control *view_of_file_pointer, HANDLE mutex_interno, HANDLE evento_JackTheReaper);

    std::vector<Aeroporto> aeroportos;
    std::unordered_map<unsigned long,AviaoShare> avioes;
    bool aceita_avioes;

    void finalizar();
};

//strutura de assistencia
struct AviaoSharedObjects_control {
    AviaoSharedObjects_control(HANDLE mutex, HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                               Mensagem_Aviao *sharedMensagemAviao);
    static std::unique_ptr<AviaoSharedObjects_control> AviaoSharedObjects_control::create(unsigned long id_aviao);

    HANDLE mutex, semaforo_write, semaforo_read, filemap;
    Mensagem_Aviao *sharedMensagemAviao;
    ~AviaoSharedObjects_control();
};
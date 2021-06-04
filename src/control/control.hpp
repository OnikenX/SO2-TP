#pragma once

#include "./../utils.hpp"
#include <vector>
#include <unordered_map>

//strutura de assistencia
struct AviaoSharedObjects_control {
    AviaoSharedObjects_control(HANDLE mutex, HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                               Mensagem_Aviao *sharedMensagemAviao);
    AviaoSharedObjects_control(AviaoSharedObjects_control&& aviaoSharedObjectsControl);
    static std::optional<AviaoSharedObjects_control> create(unsigned long id_aviao);

    AviaoSharedObjects_control(const AviaoSharedObjects_control &) = default; // non construction-copyable
    AviaoSharedObjects_control &operator=(const AviaoSharedObjects_control &) = delete; // non copyable
    HANDLE mutex, semaforo_write, semaforo_read, filemap;
    Mensagem_Aviao *sharedMensagemAviao;

    ~AviaoSharedObjects_control();

private:
    bool deleted;
};

struct aviao_in_controlstorage : AviaoShare{
    aviao_in_controlstorage(AviaoShare share, AviaoSharedObjects_control &&coms);
    AviaoSharedObjects_control coms;
    std::chrono::time_point<std::chrono::steady_clock> updated;
    void update_time();
};

struct Control {
    const DWORD MAX_AVIOES;
    const DWORD MAX_AEROPORTOS;
    bool terminar;
    //mutex_produtor para mexer na data do control
    HANDLE mutex_interno;

    // caso exista problemas a criar o control o optional não ira retornar um control
    static std::unique_ptr<Control> create(DWORD max_avioes = 50, DWORD max_aeroportos = 10);

    // main function do control
    int run();

    Control(const Control &) = delete; // non construction-copyable
    Control &operator=(const Control &) = delete; // non copyable

    ~Control();

    static bool setup_do_registry(DWORD &max_avioes, DWORD &max_aeroportos);

    //variables to destroy
    HANDLE shared_memory_handle;
    SharedMemoryMap_control *view_of_file_pointer;

    //mete o terminar a true e mete set ao evento
    void liberta_o_jack();

    //verifica se existe um aviao na mesma localizaao
    bool existeAlguem(Mensagem_Control &mensagemControl);

    bool verificaAeroporto_e_atualizaSeAviao(Mensagem_Control &mensagemControl, Mensagem_Aviao *mensagemAviao);

    Control(DWORD max_avioes, DWORD max_aeroportos, HANDLE shared_memory_handle,
            SharedMemoryMap_control *view_of_file_pointer, HANDLE mutex_interno);

    std::vector<Aeroporto> aeroportos;
    std::list<aviao_in_controlstorage> avioes;
    bool aceita_avioes;
};

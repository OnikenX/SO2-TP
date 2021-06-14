#pragma once


#include <vector>
#include <unordered_map>

#include "../utils.hpp"
#include "../shared_control_aviao.hpp"
#include "../shared_control_passageiro.hpp"

#include "resource.h"

//argumentos graphiocs



struct Control;

//strutura de assistencia
struct AviaoSharedObjects_control {
    AviaoSharedObjects_control(HANDLE mutex, HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                               Mensagem_Aviao *sharedMensagemAviao);

    AviaoSharedObjects_control(AviaoSharedObjects_control &&aviaoSharedObjectsControl) noexcept;

    AviaoSharedObjects_control() = default;

    static std::optional<AviaoSharedObjects_control> create(unsigned long id_aviao);

    AviaoSharedObjects_control(const AviaoSharedObjects_control &) = default; // non construction-copyable
    AviaoSharedObjects_control &operator=(const AviaoSharedObjects_control &) = delete; // non copyable
    HANDLE mutex, semaforo_write, semaforo_read, filemap;
    Mensagem_Aviao *sharedMensagemAviao;

    ~AviaoSharedObjects_control();

private:
    bool deleted;
};

struct Passageiro {
    Passageiro(const PassageiroInfo & info, const HANDLE& pipe);
    Passageiro(Passageiro &&passageiro) noexcept ;

    Passageiro(const Passageiro &) = default; // non construction-copyable
    Passageiro &operator=(const Passageiro &) = delete; // non copyable

    PassageiroInfo info;
    HANDLE pipe;

    ~Passageiro();

private:
    bool moved;
};

struct aviao_in_controlstorage : AviaoInfo {
    aviao_in_controlstorage(AviaoInfo share, AviaoSharedObjects_control &&coms);

    AviaoSharedObjects_control coms;
    std::chrono::time_point<std::chrono::steady_clock> updated;

    void update_time();

    //lista de passageiros que se encontram numa viagem do aviao
    std::list<Passageiro> passageiros_abordo;
};

struct Argumentos_deinit {
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPTSTR lpCmdLine;
    int nCmdShow;
    Aeroporto aeroporto;
};


struct Menu
{
    explicit Menu(Control &control);
    void run();
    Menu(const Menu &) = delete; // non construction-copyable
    Menu &operator=(const Menu &) = delete; // non copyable
    Control& control;
    int counter_aeroporto;
    bool cria_aeroporto();
    void consulta_aeroporto();
    void consultar_aviao();
    void desativa_novos_avioes();

    Argumentos_deinit componentes_graphicos{};
};
struct Control {
    const DWORD MAX_AVIOES;
    const DWORD MAX_AEROPORTOS;
    bool terminar;
    //mutex_produtor para mexer na data do control
    CRITICAL_SECTION critical_section_interno;

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

    //verifica se existe um aviaoInfo na mesma localizaao
    bool existeAlguem(Mensagem_Control_aviao &mensagemControl);

    bool verificaAeroporto_e_atualizaSeAviao(Mensagem_Control_aviao &mensagemControl, Mensagem_Aviao *mensagemAviao);

    Control(DWORD max_avioes, DWORD max_aeroportos, HANDLE shared_memory_handle,
            SharedMemoryMap_control *view_of_file_pointer, CRITICAL_SECTION critical_section_interno);
    Menu menu;
    std::vector<Aeroporto> aeroportos;
    //handles dos passageiros a espera
    std::list<Passageiro> passageiros;

    std::list<aviao_in_controlstorage> avioes;
    bool aceita_avioes;
};


#include "control.hpp"
#include "shared_control_aviao.hpp"
#include "menu.hpp"

DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;
}


void sendMessage(const AviaoSharedObjects_control &coms, Mensagem_Aviao &mensagemAviao) {
    WaitForSingleObject(coms.semaforo_write, INFINITE);
    WaitForSingleObject(coms.mutex, INFINITE);

    CopyMemory(coms.sharedMensagemAviao, &mensagemAviao, sizeof(Mensagem_Aviao));

    ReleaseMutex(coms.mutex);
    ReleaseSemaphore(coms.semaforo_read, 1, nullptr);
#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem enviada.") << std::endl;
#endif
}

void find_and_sendMessage(Control &control, unsigned long id_aviao, Mensagem_Aviao &mensagemAviao) {
    auto aviao = control.avioes.begin();
    {
        auto guard = CriticalSectionGuard(control.critical_section_interno);
        aviao = std::find_if(control.avioes.begin(), control.avioes.end(),
                             [&](aviao_in_controlstorage &aviaoShareWithComs) {
                                 return aviaoShareWithComs.IDAv == id_aviao;
                             });
        if (aviao == control.avioes.end()) {
            tcerr << "[ERROR]: O aviao " << id_aviao << " ainda não esta registado." << std::endl;
            return;
        }
        aviao->update_time();
    }
    sendMessage(aviao->coms, mensagemAviao);
}


bool
Control::verificaAeroporto_e_atualizaSeAviao(Mensagem_Control &mensagemControl, Mensagem_Aviao *mensagemAviao) {
    auto guard = CriticalSectionGuard(critical_section_interno);

    auto result = std::find_if(std::begin(aeroportos), std::end(aeroportos), [&](Aeroporto &a) {
        return a.IDAero == mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto;
    });
    if (result != std::end(aeroportos)) {
        if (mensagemAviao) {
            mensagemAviao->msg_content.respostaNovasCoordenadas.y = result->pos.y;
            mensagemAviao->msg_content.respostaNovasCoordenadas.x = result->pos.x;
        }
        return true;
    }
    return false;

}

void confirmarNovoAviao(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao{};
#ifdef _DEBUG
    tcout << t("[DEBUG]: Recebido pedido de novo aviao para o aeroporto ")
          << mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto << std::endl;
#endif
    auto coms = AviaoSharedObjects_control::create(mensagemControl.id_aviao);

    //nao encontrou o aeroporto
    if (!coms.has_value()) {
#ifdef _DEBUG
        tcerr << t("[DEBUG]: Não tenho acesso as comunicações do avião.") << mensagemControl.id_aviao << std::endl;
#endif
        HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, mensagemControl.id_aviao);
        if (process == nullptr) {
            tcerr << t("[ERROR]: Cant end process") << mensagemControl.id_aviao << std::endl;
        } else {
            TerminateProcess(process, 3);
            CloseHandle(process);
#ifdef _DEBUG
            tcerr << t("[DEBUG]: Terminated process") << mensagemControl.id_aviao;
#endif
            return;
        }
    }


    {//verifies values and takes actions
        auto guard = CriticalSectionGuard(control.critical_section_interno);

        if (!control.aceita_avioes) {
            mensagemAviao.resposta_type = Mensagem_resposta::Porta_Fechada;
        } else if (control.avioes.size() >= control.MAX_AVIOES) {
            mensagemAviao.resposta_type = Mensagem_resposta::MAX_Atingido;
        } else if (control.verificaAeroporto_e_atualizaSeAviao(mensagemControl, nullptr)) {
            control.avioes.emplace_back(mensagemControl.mensagem.pedidoConfirmarNovoAviao.av, std::move(coms.value()));
            mensagemAviao.resposta_type = Mensagem_resposta::lol_ok;
#ifdef _DEBUG
            tcout << t("[DEBUG]: Aviao com pid ") << mensagemControl.id_aviao << t(" aceite.") << std::endl;
#endif
        } else {
#ifdef _DEBUG
            tcout << t("[DEBUG]: Aviao com pid ") << mensagemControl.id_aviao << t(" receitado.") << std::endl;
#endif
            mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_nao_existe;
        }
    }
    sendMessage(coms.value(), mensagemAviao);
}

void novoDestino(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao{};

    if (control.verificaAeroporto_e_atualizaSeAviao(mensagemControl, &mensagemAviao)) {
        mensagemAviao.resposta_type = Mensagem_resposta::lol_ok;
    } else {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_nao_existe;
    }

    find_and_sendMessage(control, mensagemControl.id_aviao, mensagemAviao);
}


bool Control::existeAlguem(Mensagem_Control &mensagemControl) {
    auto guard = CriticalSectionGuard(critical_section_interno);
    for (auto &a: avioes) {
        if (a.PosA.y == mensagemControl.mensagem.pedidoConfirmarMovimento.y &&
            a.PosA.x == mensagemControl.mensagem.pedidoConfirmarMovimento.x) {
            return true;
        }
    }
    return false;
}

void alterarCoords(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao{};
    if (control.existeAlguem(mensagemControl)) {
        mensagemAviao.resposta_type = Mensagem_resposta::movimento_fail;
    } else {
        mensagemAviao.resposta_type = Mensagem_resposta::lol_ok;
    }
    find_and_sendMessage(control, mensagemControl.id_aviao, mensagemAviao);
}

void killMe(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao{};
    mensagemAviao.resposta_type = Mensagem_resposta::kill_me;
    find_and_sendMessage(control, mensagemControl.id_aviao, mensagemAviao);
}

#define WAIT_TIMELIMIT INFINITE

void mensagem_recebe(Mensagem_Control &mensagemControl,
                     SharedMemoryMap_control &sharedMem, shared_control_aviao &locks) {
    WaitForSingleObject(locks.semaforo_read_control_aviao, WAIT_TIMELIMIT);
    WaitForSingleObject(locks.mutex_partilhado, WAIT_TIMELIMIT);
    {
        CopyMemory(&mensagemControl, &sharedMem.buffer_mensagens_control[sharedMem.posReader],
                   sizeof(Mensagem_Control));
        sharedMem.posReader++;
        if (sharedMem.posReader == CIRCULAR_BUFFERS_SIZE)
            sharedMem.posReader = 0;
    }
    ReleaseMutex(locks.mutex_partilhado);
    ReleaseSemaphore(locks.semaforo_write_control_aviao, 1, nullptr);
}
void pingUpdate(Control &control, Mensagem_Control &mensagemControl){
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    auto aviao = std::find_if(control.avioes.begin(), control.avioes.end(), [&](aviao_in_controlstorage& aviao){
        return aviao.IDAv == mensagemControl.id_aviao;
    });
    if(aviao != control.avioes.end())
        aviao->update_time();
    else
        tcout << t("Aviao not found for update.\n");
}

void mensagem_trata(Control &control, Mensagem_Control &mensagemControl) {
    const TCHAR *type_string;
    switch (mensagemControl.type) {
        case confirmar_novo_aviao: {
            type_string = t("confirmar_novo_aviao");
            confirmarNovoAviao(control, mensagemControl);
            break;
        }
        case alterar_coords: {
            type_string = "alterar_coords";
            alterarCoords(control, mensagemControl);
            break;
        }
        case ping: {
            type_string = t("ping");
            pingUpdate(control, mensagemControl);
            break;
        }
        case novo_destino: {
            type_string = t("novo_destino\"");
            novoDestino(control, mensagemControl);
            break;
        }
        case suicidio: {
            type_string = t("suicidio");
            killMe(control, mensagemControl);
            break;
        }
    }
#ifdef _DEBUG
    tcout << t("[DEBUG]: Recebi msg_content \"") << type_string << t("\" por aviao com pid ")
          << mensagemControl.id_aviao
          << std::endl;
#endif
}

DWORD WINAPI ThreadReadBuffer(LPVOID param) {
    Control &control = *(Control *) param;
    Mensagem_Control mensagemControl;
    auto &sharedMem = *control.view_of_file_pointer;
    auto &locks = *shared_control_aviao::get();
    bool exit = false;
    while (!exit) {
        mensagem_recebe(mensagemControl, sharedMem, locks);
        mensagem_trata(control, mensagemControl);
        if (control.terminar)
            exit = true;
#ifdef _DEBUG
        tcerr << t("[DEBUG]: Buffer reading cycle done.\n");
#endif
    }
    return 1;
}


void limpaAntigos(Control &control) {
    using namespace std::chrono_literals;
    HANDLE process;
    auto now = std::chrono::high_resolution_clock::now();
    std::vector<DWORD> processes_to_delete;
    processes_to_delete.reserve(control.avioes.size());
    {//find processes to delete
        auto guard = CriticalSectionGuard(control.critical_section_interno);
        auto it = control.avioes.begin();
        long long durantion_in_milliseconds;
        while (it != control.avioes.end()) {
            durantion_in_milliseconds =
                    (std::chrono::duration_cast<std::chrono::milliseconds>(now - it->updated)).count();

            if (durantion_in_milliseconds >= 3000) {
#ifdef _DEBUG
                tcout << t("Vou matar o processo ") << it->IDAv << t(" pelo exceso de tempo de ")
                      << durantion_in_milliseconds << t("ms.") << std::endl;
#endif
                processes_to_delete.push_back(it->IDAv);
                auto temp = it++;
                control.avioes.erase(temp);
            }else{
                it++;
            }
        }
    }
    for (auto process_to_delete : processes_to_delete) {
        process = OpenProcess(PROCESS_TERMINATE, FALSE, process_to_delete);
        if (process == nullptr)
            tcout << t("[ERROR]: Cant end process ") << process_to_delete << std::endl;
        TerminateProcess(process, 3);
        CloseHandle(process);
    }
}

//verifica a cada 1 segundo se ouve timeouts
[[noreturn]] DWORD WINAPI VerifyValues(LPVOID param) {
    Control &control = *(Control *) param;
    while (true) {
        Sleep(3000);
        limpaAntigos(control);
#ifdef _DEBUG
        tcerr << t("[DEBUG]: Clean up cicle done.") << std::endl;
#endif
    }
}

//O Nuno Esteve Aqui as 6h30!!!
int Control::run() {
    auto menu = std::make_unique<Menu>(*this);
    //thread de leitura do buffer circular
    CreateThread(nullptr, 0, ThreadReadBuffer, this, 0, nullptr);

#ifdef PINGS
    //thread da
    CreateThread(nullptr, 0, VerifyValues, this, 0, nullptr);
#endif
    //menu do control
    WaitForSingleObject(CreateThread(nullptr, 0, ThreadMenu, menu.get(), 0, nullptr), INFINITE);

    return 0;
}


aviao_in_controlstorage::aviao_in_controlstorage(AviaoShare
                                                 share, AviaoSharedObjects_control &&coms)
        : AviaoShare(share), coms(std::move(coms)) {
    update_time();
}

void aviao_in_controlstorage::update_time() {
//    auto guard = CriticalSectionGuard(criticalSection);
    updated = std::chrono::high_resolution_clock::now();
}


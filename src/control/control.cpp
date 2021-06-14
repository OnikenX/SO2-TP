#include "control.hpp"
#include "../shared_control_aviao.hpp"

DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;
}


void sendMessage(const AviaoSharedObjects_control &coms, Mensagem_Aviao_response &mensagemAviao) {
    WaitForSingleObject(coms.semaforo_write, INFINITE);
    WaitForSingleObject(coms.mutex, INFINITE);

    CopyMemory(coms.sharedMensagemAviao, &mensagemAviao, sizeof(Mensagem_Aviao_response));

    ReleaseMutex(coms.mutex);
    ReleaseSemaphore(coms.semaforo_read, 1, nullptr);
#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem enviada.") << std::endl;
#endif
}

void find_and_sendMessage(Control &control, unsigned long id_aviao, Mensagem_Aviao_response &aviaoResponse) {
    AviaoSharedObjects_control *coms;
    {
        auto guard = CriticalSectionGuard(control.critical_section_interno);
        auto aviao = control.avioes.begin();
        aviao = std::find_if(control.avioes.begin(), control.avioes.end(),
                             [&](const aviao_in_controlstorage &aviaoShareWithComs) {
                                 return aviaoShareWithComs.IDAv == id_aviao;
                             });
        if (aviao == control.avioes.end()) {
            tcerr << "[ERROR]: O aviaoInfo " << id_aviao << " ainda não esta registado." << std::endl;
            return;
        }
        aviao->update_time();
        coms = &aviao->coms;
    }
    sendMessage(*coms, aviaoResponse);
}

bool
Control::verificaAeroporto_e_atualizaSeAviao(Mensagem_Aviao_request &aviaoRequest,
                                             Mensagem_Aviao_response *aviaoResponse) {
    auto guard = CriticalSectionGuard(critical_section_interno);
    auto result = std::find_if(std::begin(aeroportos), std::end(aeroportos), [&](Aeroporto &a) {
        return a.IDAero == aviaoRequest.mensagem.info_aeroportos.id_aeroporto;
    });

    if (result != std::end(aeroportos)) {
        if (aviaoResponse) {
            aviaoResponse->msg_content.respostaNovasCoordenadas.y = result->pos.y;
            aviaoResponse->msg_content.respostaNovasCoordenadas.x = result->pos.x;
        }
        return true;
    }
    return false;
}


bool Control::existeAlguem(Mensagem_Aviao_request &mensagemControl) {
    auto guard = CriticalSectionGuard(critical_section_interno);
    for (auto &a: avioes) {
        if (a.PosA.isEqual(mensagemControl.mensagem.coordenadas_movimento)) {
            a.PosA.x++;
            a.PosA.y++;
            return true;
        }
    }
    return false;
}


#define WAIT_TIMELIMIT INFINITE

class get_aeroporto_id_by_coords;

void mensagem_recebe(Mensagem_Aviao_request &mensagemControl,
                     SharedMemoryMap_control &sharedMem, shared_control_aviao &locks) {
    WaitForSingleObject(locks.semaforo_read_control_aviao, WAIT_TIMELIMIT);
    WaitForSingleObject(locks.mutex_partilhado, WAIT_TIMELIMIT);
    {
        CopyMemory(&mensagemControl, &sharedMem.buffer_mensagens_control[sharedMem.posReader],
                   sizeof(Mensagem_Aviao_request));
        sharedMem.posReader++;
        if (sharedMem.posReader == CIRCULAR_BUFFERS_SIZE)
            sharedMem.posReader = 0;
    }
    ReleaseMutex(locks.mutex_partilhado);
    ReleaseSemaphore(locks.semaforo_write_control_aviao, 1, nullptr);
}


namespace tratadores_de_mensagens {
    void novoDestino(Control &control, Mensagem_Aviao_request &aviaoRequest) {
        Mensagem_Aviao_response aviaoResponse{};

        if (control.verificaAeroporto_e_atualizaSeAviao(aviaoRequest, &aviaoResponse)) {
            aviaoResponse.resposta_type = Mensagem_Aviao_response_type::lol_ok;
        } else {
            aviaoResponse.resposta_type = Mensagem_Aviao_response_type::aeroporto_nao_existe;
        }
        auto id_aviao = aviaoRequest.id_aviao;
        AviaoSharedObjects_control *coms;
        std::list<Passageiro> tmp_passg;
        {
            auto guard = CriticalSectionGuard(control.critical_section_interno);
            auto aviao = control.avioes.begin();
            aviao = std::find_if(control.avioes.begin(), control.avioes.end(),
                                 [&](const aviao_in_controlstorage &aviaoShareWithComs) {
                                     return aviaoShareWithComs.IDAv == id_aviao;
                                 });
            if (aviao == control.avioes.end()) {
                tcerr << "[ERROR]: O aviaoInfo " << id_aviao << " ainda não esta registado." << std::endl;
                return;
            }
            aviao->update_time();
            coms = &aviao->coms;
            if (aviaoResponse.resposta_type == Mensagem_Aviao_response_type::lol_ok) {
                aviao->PosDest = aviaoResponse.msg_content.respostaNovasCoordenadas;
                tmp_passg.splice(tmp_passg.end(), aviao->passageiros_abordo,
                                     aviao->passageiros_abordo.begin(), aviao->passageiros_abordo.end());
            }
        }
        sendMessage(*coms, aviaoResponse);
#ifdef _DEBUG
        tcerr << t("[DEBUG]: A esvaziar os passageiros.\n");
#endif
        for(auto& passageiro : tmp_passg){
            Mensagem_Passageiro_response passageiroResponse{};
            passageiroResponse.resposta_type = Mensagem_passageiro_response_type::desembarcado;
            SendMessagePipe(passageiro.pipe, passageiroResponse);
        }
    }

    void confirmarNovoAviao(Control &control, Mensagem_Aviao_request &mensagemControl) {
        Mensagem_Aviao_response mensagemAviao{};
#ifdef _DEBUG
        tcout << t("[DEBUG]: Recebido pedido de novo aviaoInfo para o aeroporto ")
              << mensagemControl.mensagem.info_aeroportos.id_aeroporto << std::endl;
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
                mensagemAviao.resposta_type = Mensagem_Aviao_response_type::Porta_Fechada;
            } else if (control.avioes.size() >= control.MAX_AVIOES) {
                mensagemAviao.resposta_type = Mensagem_Aviao_response_type::MAX_Atingido;
            } else if (control.verificaAeroporto_e_atualizaSeAviao(mensagemControl, &mensagemAviao)) {
                mensagemControl.mensagem.info_aeroportos.aviaoInfo.PosA = mensagemAviao.msg_content.respostaNovasCoordenadas;
                mensagemControl.mensagem.info_aeroportos.aviaoInfo.PosDest = mensagemAviao.msg_content.respostaNovasCoordenadas;
                control.avioes.emplace_back(mensagemControl.mensagem.info_aeroportos.aviaoInfo,
                                            std::move(coms.value()));
                mensagemAviao.resposta_type = Mensagem_Aviao_response_type::lol_ok;
#ifdef _DEBUG
                tcout << t("[DEBUG]: Aviao com pid ") << mensagemControl.id_aviao << t(" aceite.") << std::endl;
#endif
            } else {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Aviao com pid ") << mensagemControl.id_aviao << t(" receitado.") << std::endl;
#endif
                mensagemAviao.resposta_type = Mensagem_Aviao_response_type::aeroporto_nao_existe;
            }
        }
        sendMessage(coms.value(), mensagemAviao);
    }

    void alterarCoords(Control &control, Mensagem_Aviao_request &mensagemControl) {
        Mensagem_Aviao_response aviaoResponse{};
        if (control.existeAlguem(mensagemControl)) {
            aviaoResponse.resposta_type = Mensagem_Aviao_response_type::movimento_fail;
        } else {
            aviaoResponse.resposta_type = Mensagem_Aviao_response_type::lol_ok;
        }
        auto id_aviao = mensagemControl.id_aviao;
        AviaoSharedObjects_control *coms;
        std::list<Passageiro> tmp_passg;
        {
            auto guard = CriticalSectionGuard(control.critical_section_interno);
            auto aviao = control.avioes.begin();
            aviao = std::find_if(control.avioes.begin(), control.avioes.end(),
                                 [&](const aviao_in_controlstorage &aviaoShareWithComs) {
                                     return aviaoShareWithComs.IDAv == id_aviao;
                                 });
            if (aviao == control.avioes.end()) {
                tcerr << "[ERROR]: O aviaoInfo " << id_aviao << " ainda não esta registado." << std::endl;
                return;
            }
            aviao->update_time();
            coms = &aviao->coms;
            if (aviaoResponse.resposta_type == Mensagem_Aviao_response_type::lol_ok) {
                aviao->PosA = mensagemControl.mensagem.coordenadas_movimento;
                if (aviao->PosA.isEqual(aviao->PosDest)) {
                    tmp_passg.splice(tmp_passg.end(), aviao->passageiros_abordo,
                                     aviao->passageiros_abordo.begin(), aviao->passageiros_abordo.end());
                }
            }

        }
        sendMessage(*coms, aviaoResponse);
        for(auto& passageiro : tmp_passg){
            Mensagem_Passageiro_response passageiroResponse{};
            passageiroResponse.resposta_type = Mensagem_passageiro_response_type::desembarcado_no_destino;
            SendMessagePipe(passageiro.pipe, passageiroResponse);
        }
    }

    void killMe(Control &control, Mensagem_Aviao_request &mensagemControl) {
        Mensagem_Aviao_response mensagemAviao{};
        mensagemAviao.resposta_type = Mensagem_Aviao_response_type::kill_me;
        find_and_sendMessage(control, mensagemControl.id_aviao, mensagemAviao);
    }

    void pingUpdate(Control &control, Mensagem_Aviao_request &aviaoRequest) {
        auto guard = CriticalSectionGuard(control.critical_section_interno);
        auto aviao = std::find_if(control.avioes.begin(), control.avioes.end(), [&](aviao_in_controlstorage &aviao) {
            return aviao.IDAv == aviaoRequest.id_aviao;
        });
        if (aviao != control.avioes.end())
            aviao->update_time();
        else
            tcout << t("Aviao not found for update.\n");
    }

    namespace embarcacao_funcs {
        void adiciona_lista(Control &control, std::list<Passageiro> &tmp_passg, Cords &actual, Cords &dest) {
            auto guard = CriticalSectionGuard(control.critical_section_interno);
            unsigned int actualID = 0;
            unsigned int destID = 0;
            for (const auto &aeroporto : control.aeroportos) {
                if (aeroporto.pos.isEqual(actual)) {
                    actualID = aeroporto.IDAero;
                }
                if (aeroporto.pos.isEqual(dest)) {
                    destID = aeroporto.IDAero;
                }
                if (actualID > 0 && destID > 0) {
                    break;
                }
            }
            if (actualID == 0 || destID == 0) {
#ifdef _DEBUG
                tcerr << t("[DEBUG]: Encontrado coordenadas que não aplicam...\n");
                DebugBreak();
#endif
                return;
            }
            auto it_pass = control.passageiros.begin();
            while (it_pass != control.passageiros.end()) {
                auto it_next = std::next(it_pass, 1);
                if (it_pass->info.id_aeroporto_origem == actualID && it_pass->info.id_aeroporto_destino == destID) {
                    tmp_passg.splice(tmp_passg.end(), control.passageiros, it_pass);
                }
                it_pass = it_next;
            }
        }

        void avisa_passageiros(Control &control, std::list<Passageiro> &tmp_passg) {
            auto it_pass = tmp_passg.begin();
            Mensagem_Passageiro_response passageiroResponse{};
            passageiroResponse.resposta_type = Mensagem_passageiro_response_type::embarcado;
            while (it_pass != tmp_passg.end()) {
                auto it_next = std::next(it_pass, 1);
                if (!SendMessagePipe(it_pass->pipe, passageiroResponse)) {
                    tmp_passg.erase(it_pass);
                }
                it_pass = it_next;
            }
        }

        void meter_passageiros_no_aviao(Control &control, std::list<Passageiro> &tmp_passg, unsigned long id_aviao) {
            auto guard = CriticalSectionGuard(control.critical_section_interno);
            auto aviao = std::find_if(control.avioes.begin(), control.avioes.end(),
                                      [&](auto &aviao) { return aviao.IDAv == id_aviao; });
            if (aviao == control.avioes.end()) {
                control.passageiros.splice(control.passageiros.end(), tmp_passg, tmp_passg.begin(), tmp_passg.end());
            } else {
                aviao->passageiros_abordo.splice(control.passageiros.end(), tmp_passg, tmp_passg.begin(),
                                                 tmp_passg.end());
            }
        }
    }

    void embarcacao(Control &control, Mensagem_Aviao_request &aviaoRequest) {
        std::list<Passageiro> tmp_passg;
        auto posA = aviaoRequest.mensagem.info_aeroportos.aviaoInfo.PosA;
        auto posDest = aviaoRequest.mensagem.info_aeroportos.aviaoInfo.PosDest;
        if (posA.isEqual(posDest)) {
            Mensagem_Aviao_response aviaoResponse{};
            aviaoResponse.resposta_type = Mensagem_Aviao_response_type::movimento_fail;
            find_and_sendMessage(control, aviaoRequest.id_aviao, aviaoResponse);
        } else {
            embarcacao_funcs::adiciona_lista(control, tmp_passg, posA, posDest);
            embarcacao_funcs::avisa_passageiros(control, tmp_passg);

        }
    }

}

void mensagem_trata(Control &control, Mensagem_Aviao_request &aviaoRequest) {
    const TCHAR *type_string;
    switch (aviaoRequest.type) {
        case Mensagem_aviao_request_types::confirmar_novo_aviao: {
            type_string = t("confirmar_novo_aviao");
            tratadores_de_mensagens::confirmarNovoAviao(control, aviaoRequest);
            break;
        }
        case Mensagem_aviao_request_types::alterar_coords: {
            type_string = t("alterar_coords");
            tratadores_de_mensagens::alterarCoords(control, aviaoRequest);
            break;
        }
        case Mensagem_aviao_request_types::ping: {
            type_string = t("ping");
            tratadores_de_mensagens::pingUpdate(control, aviaoRequest);
            break;
        }
        case Mensagem_aviao_request_types::novo_destino: {
            type_string = t("novo_destino\"");
            tratadores_de_mensagens::novoDestino(control, aviaoRequest);
            break;
        }
        case Mensagem_aviao_request_types::suicidio: {
            type_string = t("suicidio");
            tratadores_de_mensagens::killMe(control, aviaoRequest);
            break;
        }
        case Mensagem_aviao_request_types::embarcacao: {
            type_string = t("embarcacao");
            tratadores_de_mensagens::embarcacao(control, aviaoRequest);
            break;
        }
        default:
            type_string = t("undefined");
    }
#ifdef _DEBUG
    tcout << t("[DEBUG]: Recebi msg_content \"") << type_string << t("\" por aviaoInfo com pid ")
          << aviaoRequest.id_aviao
          << std::endl;
#endif
}

DWORD WINAPI ThreadReadBuffer(LPVOID param) {
    Control &control = *(Control *) param;
    Mensagem_Aviao_request mensagemControl;
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
            } else {
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
#ifdef _DEBUG_PINGS
        tcerr << t("[DEBUG]: Clean up cicle done.") << std::endl;
#endif
    }
}

DWORD WINAPI PipeServer(LPVOID param);

//O Nuno Esteve Aqui as 6h30!!!
int Control::run() {

    HANDLE threads[] = {
            //thread de leitura do buffer circular
            CreateThread(nullptr, 0, ThreadReadBuffer, this, 0, nullptr),
#ifdef PINGS
            //thread dos pings
            CreateThread(nullptr, 0, VerifyValues, this, 0, nullptr),
#endif
            //menu do control
            CreateThread(nullptr, 0, ThreadMenu, &menu, 0, nullptr),
            //thread de receção dos passageiros
            CreateThread(nullptr, 0, PipeServer, this, 0, nullptr),
    };
    WaitForMultipleObjects(sizeof(threads) / sizeof(HANDLE), threads, false, INFINITE);
    this->liberta_o_jack();
    return 0;
}


aviao_in_controlstorage::aviao_in_controlstorage(AviaoInfo share, AviaoSharedObjects_control &&coms)
        : AviaoInfo(share), coms(std::move(coms)) {
    update_time();
}

void aviao_in_controlstorage::update_time() {
    updated = std::chrono::high_resolution_clock::now();
}


Passageiro::~Passageiro() {
    if (!moved)
        terminate_pipe_handles(pipe);
}

Passageiro::Passageiro(Passageiro &&passageiro) noexcept
        : info(passageiro.info), pipe(passageiro.pipe) {
    passageiro.moved = true;
}

Passageiro::Passageiro(const PassageiroInfo &info, const HANDLE &pipe)
        : info(info), pipe(pipe), moved(false) {}

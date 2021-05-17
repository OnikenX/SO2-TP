#include "control.hpp"
#include "menu.hpp"


DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;
}

bool sendMessage(unsigned long id_aviao, Mensagem_Aviao &mensagemAviao) {
    auto coms = AviaoSharedObjects_control::create(id_aviao);
    if (!coms)
        return false;

    WaitForSingleObject(coms->semaforo_write, INFINITE);
    WaitForSingleObject(coms->mutex, INFINITE);

    CopyMemory(coms->sharedMensagemAviao, &mensagemAviao, sizeof(Mensagem_Aviao));

    ReleaseMutex(coms->mutex);
    ReleaseSemaphore(coms->semaforo_read, 1, nullptr);

#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem enviada.") << std::endl;
#endif
    return true;
}

bool
Control::verificaAeroporto_e_atualizaSeAviao(Mensagem_Control &mensagemControl, Mensagem_Aviao *mensagemAviao) {
    auto guard = GuardLock(mutex_interno);

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
    //nao encontrou o aeroporto
    if (!control.aceita_avioes) {
        mensagemAviao.resposta_type = Mensagem_resposta::Porta_Fechada;
    } else if (control.avioes.size() >= control.MAX_AVIOES) {
        mensagemAviao.resposta_type = Mensagem_resposta::MAX_Atingido;
    } else if (control.verificaAeroporto_e_atualizaSeAviao(mensagemControl, NULL)) {
        control.avioes.push_back(mensagemControl.mensagem.pedidoConfirmarNovoAviao.av);
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

    if (!sendMessage(mensagemControl.id_aviao, mensagemAviao))
        tcerr << t("Aviao n tem as suas cenas setadas") << std::endl;
}

void novoDestino(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao{};

    if (control.verificaAeroporto_e_atualizaSeAviao(mensagemControl, &mensagemAviao)) {
        mensagemAviao.resposta_type = Mensagem_resposta::lol_ok;
    } else {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_nao_existe;
    }
    sendMessage(mensagemControl.id_aviao, mensagemAviao);
}


bool Control::existeAlguem(Mensagem_Control &mensagemControl) {
    auto guard = GuardLock(mutex_interno);
    for (auto &a: avioes) {
        if (a.PosA.y == mensagemControl.mensagem.pedidoConfirmarMovimento.y &&
            a.PosA.x == mensagemControl.mensagem.pedidoConfirmarMovimento.x) {
            return true;
        }
    }
    return false;
}

void alterarCoords(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;
    if (control.existeAlguem(mensagemControl)) {
        mensagemAviao.resposta_type = Mensagem_resposta::movimento_fail;
    } else {
        mensagemAviao.resposta_type = Mensagem_resposta::lol_ok;
    }
    sendMessage(mensagemControl.id_aviao, mensagemAviao);
}

void killMe(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;
    mensagemAviao.resposta_type = Mensagem_resposta::kill_me;
    sendMessage(mensagemControl.id_aviao, mensagemAviao);
}

DWORD WINAPI ThreadReadBuffer(LPVOID param) {
    Control &control = *(Control *) param;
    Mensagem_Control mensagemControl;
    SharedLocks &locks = *SharedLocks::get();
    SharedMemoryMap_control &sharedMem = *control.view_of_file_pointer;
    Mensagem_Control *msgBuffer = sharedMem.buffer_mensagens_control;
    bool exit = false;
    while (!exit) {
        WaitForSingleObject(locks.semaforo_read_control_aviao, INFINITE);
        WaitForSingleObject(locks.mutex_partilhado, INFINITE);
        {
            CopyMemory(&mensagemControl, &msgBuffer[sharedMem.posReader], sizeof(Mensagem_Control));
            sharedMem.posReader++;
            if (sharedMem.posReader == CIRCULAR_BUFFERS_SIZE)
                sharedMem.posReader = 0;
        }
        ReleaseMutex(locks.mutex_partilhado);
        ReleaseSemaphore(locks.semaforo_write_control_aviao, 1, nullptr);

        switch (mensagemControl.type) {
            case confirmar_novo_aviao: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"confirmar_novo_aviao\" por aviao com pid ")
                      << mensagemControl.id_aviao
                      << std::endl;
#endif
                confirmarNovoAviao(control, mensagemControl);
                break;
            }
            case alterar_coords: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"alterar_coords\" por aviao com pid ")
                      << mensagemControl.id_aviao
                      << std::endl;
#endif
                alterarCoords(control, mensagemControl);
                break;
                case ping: {
#ifdef _DEBUG
                    tcout << t("[DEBUG]: Pong...") << std::endl;
#endif
                    break;
                }
            }
            case novo_destino: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"novo_destino\" por aviao com pid ")
                      << mensagemControl.id_aviao
                      << std::endl;
#endif
                novoDestino(control, mensagemControl);
                break;
            }
            case suicidio: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"suicidio\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
                killMe(control, mensagemControl);
                break;
#endif
            }
        }
        {
            WaitForSingleObject(control.mutex_interno, INFINITE);
            if (control.terminar) {
                exit = true;
            } else {
                auto result = std::find_if(control.avioes.begin(), control.avioes.end(), [&](AviaoShare &aviao) {
                    return aviao.IDAv == mensagemControl.id_aviao;
                });
                if (result != control.avioes.end())
                    result->update = std::chrono::system_clock::now().time_since_epoch().count();
            }
            ReleaseMutex(control.mutex_interno);
        }
    }
    return 1;
}


void limpaAntigos(Control &control) {
    using namespace std::chrono_literals;
    HANDLE process;
    long long now = std::chrono::system_clock::now().time_since_epoch().count();
    auto guard = GuardLock(control.mutex_interno);
    control.avioes.erase(
            std::remove_if(control.avioes.begin(), control.avioes.end(),
                           [&](const AviaoShare &aviao) {
                               if (now - aviao.update >= 3000) {
                                   process = OpenProcess(PROCESS_TERMINATE, FALSE, aviao.IDAv);
                                   if (process == nullptr)
                                       tcout << t("Cant end process ") << aviao.IDAv;
                                   TerminateProcess(process, 3);
                                   CloseHandle(process);
                                   return true;
                               }
                               return false;
                           }), control.avioes.end());
}

//verifica a cada 1 segundo se ouve timeouts
[[noreturn]] DWORD WINAPI VerifyValues(LPVOID param) {
    Control &control = *(Control *) param;
    while (true) {
        Sleep(3000);
        limpaAntigos(control);
    }
}

//O Nuno Esteve Aqui as 6h30!!!
int Control::run() {
    auto menu = std::make_unique<Menu>(*this);
    //thread de leitura do buffer circular
    CreateThread(NULL, 0, ThreadReadBuffer, this, 0, NULL);

#ifdef PINGS
    //thread da
    CreateThread(nullptr, 0, VerifyValues, this, 0, nullptr);
#endif
    //menu do control
    WaitForSingleObject(CreateThread(NULL, 0, ThreadMenu, menu.get(), 0, NULL), INFINITE);

    return 0;
}



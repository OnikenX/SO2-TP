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
    _tprintf(t("Boas.....\n"));
    WaitForSingleObject(coms->semaforo_write, INFINITE);
    WaitForSingleObject(coms->mutex, INFINITE);

    CopyMemory(&coms->sharedMensagemAviao, &mensagemAviao, sizeof(Mensagem_Aviao));

    ReleaseMutex(coms->mutex);
    ReleaseSemaphore(coms->semaforo_read, 1, nullptr);

#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem enviada.") << std::endl;
#endif
    return true;
}
/*
auto Control::existeAeroporto(Mensagem_Control &mensagemControl){
    auto guard = GuardLock(mutex_interno);
    auto result = std::find_if(std::begin(aeroportos), std::end(aeroportos), [&](Aeroporto& a) {
        if( a.IDAero == mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto){
            avioes.push_back(mensagemControl.mensagem.pedidoConfirmarNovoAviao.av);
            return true;
        }
    });
    return false;
}*/

//verifica se o aeroporto existe
//se é passado o argumento do mensagemAviao e metido o aviao da mensagem no vetor
bool
Control::verificaAeroporto_e_insereAviaSeExistir(Mensagem_Control &mensagemControl, Mensagem_Aviao *mensagemAviao) {
    auto guard = GuardLock(mutex_interno);
    auto result = std::find_if(std::begin(aeroportos), std::end(aeroportos), [&](Aeroporto &a) {
        return a.IDAero == mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto;
    });
    if (result != std::end(aeroportos)) {
        if (mensagemAviao) {
            avioes.push_back(mensagemControl.mensagem.pedidoConfirmarNovoAviao.av);
            mensagemAviao->msg_content.respostaNovasCoordenadas.y = result->pos.y;
            mensagemAviao->msg_content.respostaNovasCoordenadas.x = result->pos.x;
        }
        return true;
    }
    return false;

}

void confirmarNovoAviao(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;

    //nao encontrou o aeroporto
    if (control.verificaAeroporto_e_insereAviaSeExistir(mensagemControl, NULL)) {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_existe;


    } else {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_nao_existe;
    }
    if (!sendMessage(mensagemControl.id_aviao, mensagemAviao))
        tcerr << t("Aviao n tem as suas cenas setadas") << std::endl;
}

void novoDestino(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;

    if (control.verificaAeroporto_e_insereAviaSeExistir(mensagemControl, &mensagemAviao)) {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_existe;

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
    while (true) {

        WaitForSingleObject(locks.semaforo_read_control_aviao, INFINITE);
        WaitForSingleObject(locks.mutex_partilhado, INFINITE);

        CopyMemory(&mensagemControl, &msgBuffer[sharedMem.posReader], sizeof(Mensagem_Control));
        sharedMem.posReader++;
        if (sharedMem.posReader == CIRCULAR_BUFFERS_SIZE)
            sharedMem.posReader = 0;

        ReleaseMutex(locks.mutex_partilhado);
        ReleaseSemaphore(locks.semaforo_write_control_aviao, 1, nullptr);

        switch (mensagemControl.type) {
            case confirmar_novo_aviao: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"confirmar_novo_aviao\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
#endif
                confirmarNovoAviao(control, mensagemControl);
                break;
            }
            case alterar_coords: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"alterar_coords\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
#endif
                alterarCoords(control, mensagemControl);
                break;
            }
            case novo_destino: {
#ifdef _DEBUG
                tcout << t("[DEBUG]: Recebi msg_content \"novo_destino\" por aviao com pid ") << mensagemControl.id_aviao
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
            if (control.terminar)
                break;
            ReleaseMutex(control.mutex_interno);
        }
    }
    return 1;
}

int Control::run() {
    auto menu = std::make_unique<Menu>(*this);
    const int nthreads = 2;
    HANDLE threads[nthreads];
    threads[0] = CreateThread(NULL, 0, ThreadMenu, menu.get(), 0, NULL);
    threads[1] = CreateThread(NULL, 0, ThreadReadBuffer, this, 0, NULL);
    WaitForMultipleObjects(nthreads, threads, TRUE, INFINITE);
    return 0;
}



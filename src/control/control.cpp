#include "control.hpp"
#include "menu.hpp"


DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;
}

void sendMessage(unsigned long id_aviao, Mensagem_Aviao &mensagemAviao) {
    auto coms = AviaoSharedObjects_control::create(id_aviao);
    if (!coms)
        return;
    WaitForSingleObject(coms->semaforo_read, INFINITE);
    WaitForSingleObject(coms->mutex, INFINITE);
    CopyMemory(&coms->sharedMensagemAviao, &mensagemAviao, sizeof(Mensagem_Aviao));
    ReleaseMutex(coms->mutex);
    ReleaseSemaphore(coms->semaforo_read, 1, nullptr);
#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem enviada.") << std::endl;
#endif
}

auto Control::existeAeroporto(Mensagem_Control &mensagemControl){
    auto guard = GuardLock(mutex_interno);
    auto result = std::find_if(std::begin(aeroportos), std::end(aeroportos), [&](Aeroporto& a) {
        if( a.IDAero == mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto){
            avioes.push_back(mensagemControl.mensagem.pedidoConfirmarNovoAviao.av);
            return true;
        }
    });
    return false;
}

void confirmarNovoAviao(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;

    //nao encontrou o aeroporto
    if (control.existeAeroporto(mensagemControl)) {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_nao_existe;


    }else{
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_existe;
    }
        sendMessage(mensagemControl.id_aviao, mensagemAviao);
}
/*
void  novoDestino(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;
    auto guard = GuardLock(control.mutex_interno);

    auto result = std::find_if(std::begin(control.aeroportos), std::end(control.aeroportos), [&](Aeroporto& a) {
        return a.pos.y==mensagemControl.mensagem.pedidoConfirmarMovimento.y && a.pos.x==mensagemControl.mensagem.pedidoConfirmarMovimento.x;
    });

    if (result == std::end(control.aeroportos)) {
        mensagemAviao.resposta_type = Mensagem_resposta::aeroporto_nao_existe;
        sendMessage(mensagemControl.id_aviao, mensagemAviao);
    }

}
*/


bool Control::existeAlguem(Mensagem_Control &mensagemControl){
    auto guard = GuardLock(mutex_interno);
    for(auto& a: avioes){
        if(a.PosA.y==mensagemControl.mensagem.pedidoConfirmarMovimento.y && a.PosA.x==mensagemControl.mensagem.pedidoConfirmarMovimento.x){
            return true;
        }
    }
    return false;
}

void alterarCoords (Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;
    if (control.existeAlguem(mensagemControl)) {
        mensagemAviao.resposta_type = Mensagem_resposta::movimento_fail;
    }else{
        mensagemAviao.resposta_type = Mensagem_resposta::movimento_sucess;
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
        ReleaseSemaphore(locks.semaforo_read_control_aviao, 1, NULL);

        switch (mensagemControl.type) {
            case confirmar_novo_aviao: {
#ifdef _DEBUG
                tcout << t("Recebi msg_content \"confirmar_novo_aviao\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
#endif
                confirmarNovoAviao(control, mensagemControl);
                break;
            }
            case alterar_coords: {
#ifdef _DEBUG
                tcout << t("Recebi msg_content \"alterar_coords\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
#endif
                alterarCoords(control, mensagemControl);
                break;
            }
       /*     case novo_destino: {
#ifdef _DEBUG
                tcout << t("Recebi msg_content \"novo_destino\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
#endif
                novoDestino(control, mensagemControl);
                break;
            }*/
            case suicidio: {
#ifdef _DEBUG
                tcout << t("Recebi msg_content \"suicidio\" por aviao com pid ") << mensagemControl.id_aviao
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


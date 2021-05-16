
//#include "./../../SO2_TP_DLL/SO2_TP_DLL_2021.h"

#include "aviao.hpp"
#include "menu.hpp"

//novo destino
//iniciar voo
//suicidio
//TODO: corrigir esta thread
DWORD WINAPI Limbo(LPVOID param) {
#ifdef _DEBUG
    tcout << t("waiting for death ...") << std::endl;
#endif
    HANDLE evento = CreateEvent(nullptr, TRUE, FALSE, EVENT_KILLER);
    WaitForSingleObject(evento, INFINITE);
#ifdef _DEBUG
    tcout << t("BOOM !!X\\ RIP In Piece );") << std::endl;
#endif
    return 1;
}

DWORD WINAPI ThreadMenu(LPVOID param) {
    AviaoInstance &aviao = *(AviaoInstance *) param;
    Menu menu(aviao);
    menu.run();
    return 1;
}

std::unique_ptr<Mensagem_Aviao> AviaoInstance::sendMessage(bool recebeResposta, Mensagem_Control &mensagemControl) {
    //nao deixa ninguem enviar mensagens ao mesmo tempo na mesma thread
    auto guard = GuardLock(this->sharedComs->mutex_mensagens);

    WaitForSingleObject(SharedLocks::get()->semaforo_write_control_aviao, INFINITE);
    WaitForSingleObject(SharedLocks::get()->mutex_partilhado, INFINITE);

    CopyMemory(&this->sharedMemoryMap->buffer_mensagens_control[this->sharedMemoryMap->posWriter], &mensagemControl, sizeof(Mensagem_Control));
    this->sharedMemoryMap->posWriter++;

    if(this->sharedMemoryMap->posWriter == CIRCULAR_BUFFERS_SIZE)
        this->sharedMemoryMap->posWriter = 0;

    ReleaseMutex(SharedLocks::get()->mutex_partilhado);
    ReleaseSemaphore(SharedLocks::get()->semaforo_read_control_aviao, 1, nullptr);
#ifdef _DEBUG
    tcout << t("mensagem enviada") << std::endl;
#endif
    if(!recebeResposta)
        return nullptr;
    auto resposta = std::make_unique<Mensagem_Aviao>();
    WaitForSingleObject(this->sharedComs->semaforo_read, INFINITE);
    WaitForSingleObject(this->sharedComs->mutex_produtor, INFINITE);

    CopyMemory(resposta.get(), this->sharedComs->sharedMensagemAviao, sizeof(Mensagem_Aviao));

    ReleaseMutex(sharedComs->mutex_produtor);
    ReleaseSemaphore(this->sharedComs->semaforo_write, 1, nullptr);
    return resposta;
}

int AviaoInstance::run() {
    HANDLE Wait_to_Die = CreateThread(nullptr, 0, Limbo, nullptr, 0, nullptr);
    //o menu é uma dangling thread porque ele esta sempre a espera
    CreateThread(nullptr, 0, ThreadMenu, this, 0, nullptr);

    HANDLE threads[] = {Wait_to_Die};
    const int nThreads = sizeof(threads) / sizeof(HANDLE);
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    return 0;
}




//#include "./../../SO2_TP_DLL/SO2_TP_DLL_2021.h"

#include "aviao.hpp"
#include "menu.hpp"
#include "shared_control_aviao.hpp"

#ifndef _WIN64
#pragma message("[WARNING]: A compilação não está a ser feita em 64bit Windows,\
a dll que vem com este programa é só para 64bit windows.")
#pragma message("[WARNING]: Porfavor modifique as configurações de compilação para 64bit\
ou use utilize uma dll de 32bit se quer usar o programa em 32bits.")
#endif

//novo destino
//iniciar voo
//suicidio
//TODO: corrigir esta thread
DWORD WINAPI Limbo(LPVOID param) {
#ifdef _DEBUG
    tcout << t("[DEBUG]: waiting for death ...") << std::endl;
#endif
    HANDLE evento = CreateEvent(nullptr, TRUE, FALSE, EVENT_KILLER);
    WaitForSingleObject(evento, INFINITE);
#ifdef _DEBUG
    tcout << t("[DEBUG]: BOOM !!X\\ RIP In Piece );") << std::endl;
#endif
    return 1;
}

DWORD WINAPI ThreadMenu(LPVOID param) {
    AviaoInstance &aviao = *(AviaoInstance *) param;
    Menu menu(aviao);
    menu.run();
    TerminateThread(aviao.Wait_to_Die, 2);
    return 1;
}

std::unique_ptr<Mensagem_Aviao> AviaoInstance::sendMessage(bool recebeResposta, Mensagem_Control &mensagemControl) {
    //nao deixa ninguem enviar mensagens ao mesmo tempo na mesma thread
    auto guard = GuardLock(this->sharedComs->mutex_mensagens);

    WaitForSingleObject(shared_control_aviao::get()->semaforo_write_control_aviao, INFINITE);
    WaitForSingleObject(shared_control_aviao::get()->mutex_partilhado, INFINITE);

    CopyMemory(&this->sharedMemoryMap->buffer_mensagens_control[this->sharedMemoryMap->posWriter], &mensagemControl, sizeof(Mensagem_Control));
    this->sharedMemoryMap->posWriter++;

    if(this->sharedMemoryMap->posWriter == CIRCULAR_BUFFERS_SIZE)
        this->sharedMemoryMap->posWriter = 0;

    ReleaseMutex(shared_control_aviao::get()->mutex_partilhado);
    ReleaseSemaphore(shared_control_aviao::get()->semaforo_read_control_aviao, 1, nullptr);
#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem enviada") << std::endl;
#endif
    if(!recebeResposta)
        return nullptr;

    auto resposta = std::make_unique<Mensagem_Aviao>();

    WaitForSingleObject(this->sharedComs->semaforo_read, INFINITE);
    WaitForSingleObject(this->sharedComs->mutex_produtor, INFINITE);

    CopyMemory(resposta.get(), this->sharedComs->sharedMensagemAviao, sizeof(Mensagem_Aviao));

    ReleaseMutex(sharedComs->mutex_produtor);
    ReleaseSemaphore(this->sharedComs->semaforo_write, 1, nullptr);


#ifdef _DEBUG
    tcout << t("[DEBUG]: Mensagem recebida.") << std::endl;
#endif
    return resposta;
}

[[noreturn]] DWORD WINAPI ThreadUpdater(LPVOID param){
    AviaoInstance &aviao = *(AviaoInstance*)param;
    while(true){
        Mensagem_Control mensagemControl{};
        mensagemControl.id_aviao = aviao.aviao.IDAv;
        mensagemControl.type = ping;
        aviao.sendMessage(false, mensagemControl);
#ifdef _DEBUG
        tcout << t("[Debug]: Ping...") << std::endl;
#endif
        Sleep(1000);
    }
}

int AviaoInstance::run() {
    Wait_to_Die = CreateThread(nullptr, 0, Limbo, nullptr, 0, nullptr);
    //o menu é uma dangling thread porque ele esta sempre a espera
    CreateThread(nullptr, 0, ThreadMenu, this, 0, nullptr);
#ifdef PINGS
    CreateThread(nullptr, 0, ThreadUpdater, this, 0, nullptr);
#endif
    WaitForSingleObject(Wait_to_Die, INFINITE);

    return 0;
}

void AviaoInstance::suicidio() {
    Mensagem_Control mensagemControl{};
    mensagemControl.type = Mensagem_types::suicidio;
    sendMessage(false, mensagemControl);
}



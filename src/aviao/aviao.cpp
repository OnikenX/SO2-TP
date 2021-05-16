
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
    HANDLE evento = CreateEvent(NULL, TRUE, FALSE, EVENT_KILLER);
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
}

std::unique_ptr<Mensagem_Aviao> AviaoInstance::sendMessage(bool recebeResposta, Mensagem_Control &mensagemControl) {
    auto guard = GuardLock(this->sharedComs->mutex_mensagens);
}

int AviaoInstance::run() {

    HANDLE Wait_to_Die = CreateThread(NULL, 0, Limbo, NULL, 0, NULL);
    Menu menu(*this);
    HANDLE threadMenu = CreateThread(NULL, 0, ThreadMenu, this, 0, NULL);

    //a thread menu é ignorada
    HANDLE threads[] = {Wait_to_Die};
    const int nThreads = sizeof(threads) / sizeof(HANDLE);
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    return 0;
}


std::unique_ptr<AviaoSharedObjects_aviao> AviaoSharedObjects_aviao::create() {
    unsigned long id_aviao = GetProcessId(nullptr);
    TCHAR nome[30];
    //shared memory name
    _stprintf(nome, t("S02_TP_FM_%lu"), id_aviao);
    HANDLE filemap = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(Mensagem_Aviao), nome);
    if (!filemap)
        return nullptr;
    auto sharedMensagemAviao =
            (Mensagem_Aviao *) MapViewOfFile(filemap, FILE_MAP_ALL_ACCESS,
                                             0, 0, sizeof(Mensagem_Aviao));
    if (!sharedMensagemAviao) {
        CloseHandle(filemap);
        return nullptr;
    }

    HANDLE mutex_mensagens = CreateMutex(nullptr, FALSE, nullptr);
    _stprintf(nome, t("S02_TP_SR_%lu"), id_aviao);
    HANDLE semaforo_read = CreateSemaphore(nullptr, 0, 1, nome);
    _stprintf(nome, t("S02_TP_SW_%lu"), id_aviao);
    HANDLE semaforo_write = CreateSemaphore(nullptr, 1, 1, nome);
    _stprintf(nome, t("S02_TP_MT_%lu"), id_aviao);
    HANDLE mutex_produtores = CreateMutex(nullptr, FALSE, nome);
    if (!semaforo_read || !semaforo_write || !mutex_produtores || !mutex_mensagens) {
        UnmapViewOfFile(sharedMensagemAviao);
        CloseHandle(filemap);
        if (mutex_produtores)
            CloseHandle(mutex_produtores);
        if (mutex_mensagens)
            CloseHandle(mutex_mensagens);
        if (semaforo_read)
            CloseHandle(semaforo_read);
        if (semaforo_write)
            CloseHandle(semaforo_write);
        if (mutex_produtores)
            return nullptr;
    }

    return std::make_unique<AviaoSharedObjects_aviao>(mutex_mensagens, mutex_produtores, semaforo_write, semaforo_read,
                                                      filemap,
                                                      sharedMensagemAviao);
}

AviaoSharedObjects_aviao::AviaoSharedObjects_aviao(HANDLE mutex_mensagens, HANDLE mutex_produtor,
                                                   HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                                                   Mensagem_Aviao *sharedMensagemAviao)
        : mutex_produtor(mutex_produtor), semaforo_write(semaforo_write), semaforo_read(semaforo_read),
          filemap(filemap), sharedMensagemAviao(sharedMensagemAviao), mutex_mensagens(mutex_mensagens) {}

AviaoSharedObjects_aviao::~AviaoSharedObjects_aviao() {
    UnmapViewOfFile(sharedMensagemAviao);
    CloseHandle(filemap);
    if (mutex_produtor)
        CloseHandle(mutex_produtor);
    if (semaforo_read)
        CloseHandle(semaforo_read);
    if (semaforo_write)
        CloseHandle(semaforo_write);
}

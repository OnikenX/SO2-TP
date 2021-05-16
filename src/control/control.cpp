#include "control.hpp"
#include "menu.hpp"


DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;
}


std::unique_ptr<AviaoSharedObjects_control> AviaoSharedObjects_control::create(unsigned long id_aviao) {
    TCHAR nome[30];
    //shared memory name
    _stprintf(nome, t("FM_%lu"), id_aviao);
    HANDLE filemap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nome);
    if (!filemap)
        return nullptr;
    auto sharedMensagemAviao =
            (Mensagem_Aviao *) MapViewOfFile(filemap,FILE_MAP_ALL_ACCESS,
                                             0, 0, sizeof(Mensagem_Aviao));
    if (!sharedMensagemAviao) {
        CloseHandle(filemap);
        return nullptr;
    }

    _stprintf(nome, t("SR_%lu"), id_aviao);
    //shared semafore
    HANDLE semaforo_read = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, nome);
    _stprintf(nome, t("SW_%lu"), id_aviao);
    HANDLE semaforo_write = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, nome);
    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, nome);
    if (!semaforo_read || !semaforo_write || !mutex) {
        UnmapViewOfFile(sharedMensagemAviao);
        CloseHandle(filemap);
        if (mutex)
            CloseHandle(mutex);
        if (semaforo_read)
            CloseHandle(semaforo_read);
        if (semaforo_write)
            CloseHandle(semaforo_write);
        return nullptr;
    }
    return std::make_unique<AviaoSharedObjects_control>(mutex, semaforo_write, semaforo_read, filemap, sharedMensagemAviao);
}

AviaoSharedObjects_control::AviaoSharedObjects_control(HANDLE mutex, HANDLE semaforo_write, HANDLE semaforo_read,
                                                       HANDLE filemap, Mensagem_Aviao *sharedMensagemAviao)
        : mutex(mutex), semaforo_write(semaforo_write), semaforo_read(semaforo_read),
          filemap(filemap), sharedMensagemAviao(sharedMensagemAviao) {}

AviaoSharedObjects_control::~AviaoSharedObjects_control() {
    UnmapViewOfFile(sharedMensagemAviao);
    CloseHandle(filemap);
    if (mutex)
        CloseHandle(mutex);
    if (semaforo_read)
        CloseHandle(semaforo_read);
    if (semaforo_write)
        CloseHandle(semaforo_write);
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

void confirmarNovoAviao(Control &control, Mensagem_Control &mensagemControl) {
    Mensagem_Aviao mensagemAviao;
    auto guard = GuardLock(control.mutex_interno);

//    auto exists =
    auto result = std::find_if(std::begin(control.aeroportos), std::end(control.aeroportos), [&](Aeroporto &a) {
        return a.IDAero == mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto;
    });
    //nao encontrou o aeroporto
    if (result == std::end(control.aeroportos)) {
        mensagemAviao.resposta = Mensagem_resposta::aeroporto_nao_existe;
        sendMessage(mensagemControl.id_aviao, mensagemAviao);
    }

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
                tcout << t("Recebi msg \"confirmar_novo_aviao\" por aviao com pid ") << mensagemControl.id_aviao
                      << std::endl;
#endif
                confirmarNovoAviao(control, mensagemControl);
                break;
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

Control::~Control() {
    UnmapViewOfFile(view_of_file_pointer);
    CloseHandle(shared_memory_handle);
    this->liberta_o_jack();
}

void Control::finalizar() {
    SetEvent(evento_JackTheReaper);
    auto guard = GuardLock(mutex_interno);
    terminar = true;
}

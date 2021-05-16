#include "control.hpp"
#include "menu.hpp"


DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;

}

DWORD WINAPI ThreadReadBuffer(LPVOID param) {
    Control &control = *(Control *) param;
    Mensagem_Control mensagemControl;
    SharedLocks& locks = *SharedLocks::get();
    SharedMemoryMap_control& sharedMem = *control.view_of_file_pointer;
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

        mensagemControl.;
        {
            auto guard = GuardLock(control.mutex_interno);
            if (control.terminar)
                break;
        }
    }
    return 1;
}

int Control::run() {
    auto menu = std::make_unique<Menu>(*this);
    const int nthreads = 4;
    HANDLE threads[nthreads];
    threads[0] = CreateThread(NULL, 0, ThreadMenu, menu.get(), 0, NULL);
    threads[1] = CreateThread(NULL, 0, ThreadReadBuffer, this, 0, NULL);
    WaitForMultipleObjects(nthreads, threads, TRUE, INFINITE);
    return 0;
}

Control::~Control() {
    UnmapViewOfFile(view_of_file_pointer);
    CloseHandle(shared_memory_handle);
    this->notifica_tudo();
}

void Control::finalizar() {
    //faz cenas para dizer que vai fechar
    tcout << t("A espera de todos para mimir...");
}

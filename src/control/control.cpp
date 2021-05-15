#include "control.hpp"
#include "menu.hpp"

enum ControlActions {
    EncerrarSistema,
    CriarAeroporto,
    ToggleAceitarAvioes
};

DWORD WINAPI ThreadMenu(LPVOID param) {
    Menu &menu = *(Menu *) param;
    menu.run();
    return 1;
}

DWORD WINAPI ThreadBufferWrite(LPVOID param) {

    return 1;
}

DWORD WINAPI ThreadBufferRead(LPVOID param) {

    return 1;
}

DWORD WINAPI ThreadUpdateMap(LPVOID param) {

    return 1;
}

int Control::run() {
    //TODO: criar pipes para que o menu e control se comuniquem

    auto menu = std::make_unique<Menu>(*this);
    const int nthreads = 4;
    HANDLE threads[nthreads];
    threads[0] = CreateThread(NULL, 0, ThreadMenu, menu.get(), 0, NULL);
    threads[1] = CreateThread(NULL, 0, ThreadBufferWrite, this, 0, NULL);
    threads[2] = CreateThread(NULL, 0, ThreadBufferRead, this, 0, NULL);
    threads[3] = CreateThread(NULL, 0, ThreadUpdateMap, this, 0, NULL);
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

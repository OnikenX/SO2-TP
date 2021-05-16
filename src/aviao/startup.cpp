#include "aviao.hpp"
//funções do control que servem de inicialização


void shared_memory_verification(HANDLE &hMapFile, SharedMemoryMap_control *&pBuf) {

    //cria se um file maping
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
    if (hMapFile == nullptr) {
        tcout << t("O Control ainda não existe. Erro:") << GetLastError() << std::endl;
        return;
    }

    pBuf = (SharedMemoryMap_control *) MapViewOfFile(hMapFile,   // handle to map object
                                                     FILE_MAP_ALL_ACCESS, // read/write permission
                                                     0,
                                                     0,
                                                     sizeof(SharedMemoryMap_control)
    );

    if (pBuf == nullptr) {
        _tprintf(TEXT("Could not map view of file (%d).\n"),
                 GetLastError());
        CloseHandle(hMapFile);

    }
}


HMODULE getdll() {
    const int nSize = 255;
    TCHAR path[nSize] = t("");
    HMODULE hdll = LoadLibraryEx(t("SO2_TP_DLL_2021.dll"), nullptr, 0);
    if (hdll == nullptr) {
        GetModuleFileName(nullptr, path, nSize);
        tcerr <<
              t("Não encontrei a dll SO2_TP_DLL_2021.dll, por favor insira a em: ")
              << path << std::endl;
        return nullptr;
    }
    return hdll;
}

std::unique_ptr<AviaoInstance>
AviaoInstance::create(AviaoShare av) {
    auto coms = AviaoSharedObjects_aviao::create();
    if (!coms) {
        tcerr << t("Erro a criar a memoria partinhada para ser recebida por este aviao.") << std::endl;
        return nullptr;
    }

    if (!SharedLocks::get()) {
        tcerr << t("Erro a criar mutex_produtor e semaforos partilhados para o controller.") << std::endl;
        return nullptr;
    }

    HANDLE hMapFile = nullptr;
    SharedMemoryMap_control *sharedMemoryMap = nullptr;
    shared_memory_verification(hMapFile, sharedMemoryMap);
    if (hMapFile == nullptr || sharedMemoryMap == nullptr)
        return nullptr;

    HMODULE dllHandle = getdll();
    if (dllHandle == nullptr) {
        UnmapViewOfFile(sharedMemoryMap);
        CloseHandle(hMapFile);
        return nullptr;
    }

    auto aviao = std::make_unique<AviaoInstance>(hMapFile, sharedMemoryMap,
                                                 dllHandle, av, std::move(coms));

    if (!aviao->verifica_criacao_com_control())
        return nullptr;


    return aviao;
}


AviaoInstance::AviaoInstance(HANDLE hMapFile, SharedMemoryMap_control *sharedMemoryMap, void *dllHandle,
                             AviaoShare av, std::unique_ptr<AviaoSharedObjects_aviao> sharedComs)
        : hMapFile(hMapFile), sharedMemoryMap(sharedMemoryMap), dllHandle(dllHandle),
          id_do_aeroporto(av.IDAv), aviao(av), sharedComs(std::move(sharedComs)) {
    ptr_move_func = GetProcAddress((HMODULE) dllHandle, "move");
    av.IDAv = (int) GetProcessId(nullptr);
}

int AviaoInstance::move(int cur_x, int cur_y, int final_dest_x,
                        int final_dest_y, int *next_x, int *next_y) {
    auto move_fnc = (int (*)(int, int, int, int, int *, int *)) ptr_move_func;
    return move_fnc(cur_x, cur_y, final_dest_x, final_dest_y, next_x, next_y);
}

AviaoInstance::~AviaoInstance() {
    FreeLibrary((HMODULE) dllHandle);
    UnmapViewOfFile(sharedMemoryMap);
    CloseHandle(hMapFile);

#ifdef _DEBUG
    tcout << t("Avião destruido.") << std::endl;
#endif
}

bool AviaoInstance::verifica_criacao_com_control() {
    return true;
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

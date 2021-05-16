#include "aviao.hpp"
//funções do control que servem de inicialização


void shared_memory_verification(HANDLE &hMapFile, SharedMemoryMap_control *&pBuf) {

    //cria se um file maping
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
    if (hMapFile == NULL) {
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

std::optional<std::unique_ptr<AviaoInstance>>
AviaoInstance::create(int id_do_aeroporto, int velocidade, int cap_max) {

    if (!SharedLocks::get()){
        tcerr << t("Erro a criar mutex e semaforos partilhados.") << std::endl;
        return std::nullopt;
    }

    HANDLE hMapFile = nullptr;
    SharedMemoryMap_control *sharedMemoryMap = nullptr;
    shared_memory_verification(hMapFile, sharedMemoryMap);
    if (hMapFile == nullptr || sharedMemoryMap == nullptr)
        return std::nullopt;

    HMODULE dllHandle = getdll();
    if (dllHandle == nullptr) {
        UnmapViewOfFile(sharedMemoryMap);
        CloseHandle(hMapFile);
        return std::nullopt;
    }
    auto aviao = std::make_unique<AviaoInstance>(hMapFile, sharedMemoryMap, dllHandle,
                                                 id_do_aeroporto, velocidade, cap_max);
    if (!aviao->verifica_criacao_com_control())
        return std::nullopt;

    return std::optional(std::move(aviao));
}


AviaoInstance::AviaoInstance(HANDLE hMapFile, SharedMemoryMap_control *sharedMemoryMap,
                             void *dllHandle, TCHAR *nome_do_aeroporto, int velocidade, int cap_max)
        : hMapFile(hMapFile), sharedMemoryMap(sharedMemoryMap), dllHandle(dllHandle),
          id_do_aeroporto(id_do_aeroporto), velocidade(velocidade), cap_max(cap_max) {
    ptr_move_func = GetProcAddress((HMODULE) dllHandle, "move");
    id = (int) GetProcessId(nullptr);
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

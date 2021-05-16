#include "control.hpp"
//funções do control que servem de inicialização

Control::Control(DWORD max_avioes, DWORD max_aeroportos, HANDLE shared_memory_handle,
                 SharedMemoryMap_control *view_of_file_pointer, HANDLE mutex_interno, HANDLE evento_JackTheReaper)
        : MAX_AVIOES(max_avioes), MAX_AEROPORTOS(max_aeroportos), shared_memory_handle(shared_memory_handle),
          view_of_file_pointer(view_of_file_pointer), aceita_avioes(true), mutex_interno(mutex_interno),
          evento_JackTheReaper(evento_JackTheReaper) {}


// registry stuff
const TCHAR chave_nome[] = TEXT("SOFTWARE\\TP_SO2\\Control");
const TCHAR par_nome_max_avioes[] = TEXT("MaxAvioes");
const TCHAR par_nome_max_aeroportos[] = TEXT("MaxAeroportos");

bool registry_get_key(TCHAR par_nome[], DWORD &key_value) {
    DWORD resultado = 0;
    DWORD cbData = sizeof(resultado);
    //vai buscar o valor da chave
    auto return_value = RegGetValue(HKEY_CURRENT_USER,
                                    chave_nome,
                                    par_nome,
                                    RRF_RT_ANY,
                                    NULL,
                                    &resultado,
                                    &cbData
    );
    if (return_value == ERROR_SUCCESS && cbData == sizeof(DWORD)) {
        key_value = resultado;
        return true;
    }
    return false;
}

bool cria_chaves(DWORD &max_avioes, DWORD &max_aeroportos) {
    //cria as chaves
    HKEY chave;
    if (RegCreateKeyEx(
            HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &chave, NULL) != ERROR_SUCCESS) {
        tcerr << t("A chave não foi criada nem aberta.");
        return false;
    }
    bool return_value = true;
    if (RegSetValueEx(chave, par_nome_max_avioes, 0, RRF_RT_ANY,
                      (LPCBYTE) &max_avioes, sizeof(DWORD)) != ERROR_SUCCESS ||
        RegSetValueEx(chave, par_nome_max_aeroportos, 0, RRF_RT_ANY,
                      (LPCBYTE) &max_aeroportos, sizeof(DWORD)) != ERROR_SUCCESS) {
        tcerr << t("Erro a dar valores das chaves.");
        return_value = false;
    }
    RegCloseKey(chave);
    return return_value;
}

//se conseguir ler as chaves ou criar returna true se não returna false
bool Control::setup_do_registry(DWORD &max_avioes, DWORD &max_aeroportos) {
    if (registry_get_key((TCHAR *) par_nome_max_avioes, max_avioes))
        if (registry_get_key((TCHAR *) par_nome_max_aeroportos, max_aeroportos))
            return true;
    return cria_chaves(max_avioes, max_aeroportos);
}


std::optional<std::unique_ptr<Control>> Control::create(DWORD max_avioes, DWORD max_aeroportos) {

    if (!SharedLocks::get()) {
        tcerr << t("Erro a criar mutex e semaforos partilhados.") << std::endl;
        return std::nullopt;
    }

    //cria se um file maping
    HANDLE hMapFile =
            CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemoryMap_control),
                              SHARED_MEMORY_NAME);

    if (hMapFile == NULL) {
        DWORD getlasterror = GetLastError();
        if (getlasterror == ERROR_ALREADY_EXISTS) {
            tcout << t("O Control já existe.") << std::endl;
        } else {
            tcout << t("Não foi possivel criar o file mapping por uma razão desconhecida: ")
                  << getlasterror << std::endl;
        }
        return std::nullopt;
    }

    auto pBuf = (SharedMemoryMap_control *) MapViewOfFile(hMapFile,   // handle to map object
                                                          FILE_MAP_ALL_ACCESS, // read/write permission
                                                          0,
                                                          0,
                                                          sizeof(SharedMemoryMap_control)
    );

    if (pBuf == NULL) {
        _tprintf(TEXT("Could not map view of file (%d).\n"),
                 GetLastError());
        CloseHandle(hMapFile);
        return std::nullopt;
    }

    HANDLE mutex_interno = CreateMutex(nullptr, FALSE, nullptr);
    if (mutex_interno == nullptr) {
        CloseHandle(hMapFile);
        UnmapViewOfFile(pBuf);
    }

    if (!setup_do_registry(max_avioes, max_aeroportos)) {
        CloseHandle(hMapFile);
        UnmapViewOfFile(pBuf);
        CloseHandle(mutex_interno);
        return std::nullopt;
    }

    HANDLE evento = CreateEvent(nullptr, TRUE, FALSE, EVENT_KILLER);
    if (!evento) {
        CloseHandle(hMapFile);
        UnmapViewOfFile(pBuf);
        CloseHandle(mutex_interno);
        return std::nullopt;
    }

    // a razão para usar o unique pointer é para que o compilar n esteja a chamar o contrutor ou o descontrutor desnecessariamente
    // com um unique pointer certificamo nos que n existem copias descessarias do Control ou referencias para ele
    return std::optional(std::make_unique<Control>(max_avioes, max_aeroportos, hMapFile, pBuf, mutex_interno, evento));
}

//
void Control::liberta_o_jack() {
    SetEvent(evento_JackTheReaper);
    tcout << t("Isto é o Big Crunch deste universo, foi um prazer poder viajar consigo") << std::endl;
}

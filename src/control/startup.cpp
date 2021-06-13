#include "control.hpp"
#include "shared_control_aviao.hpp"
//funções do control que servem de inicialização

Control::Control(DWORD max_avioes, DWORD max_aeroportos, HANDLE shared_memory_handle,
                 SharedMemoryMap_control *view_of_file_pointer, CRITICAL_SECTION critical_section_interno)
        : MAX_AVIOES(max_avioes), MAX_AEROPORTOS(max_aeroportos),
          shared_memory_handle(shared_memory_handle), view_of_file_pointer(view_of_file_pointer),
          aceita_avioes(true), critical_section_interno(critical_section_interno), terminar(false) {}


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
        tcerr << t("Erro a dar valores das chaves.") << std::endl;
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


std::unique_ptr<Control> Control::create(DWORD max_avioes, DWORD max_aeroportos) {

    if (!shared_control_aviao::get()) {
        tcerr << t("Erro a criar mutex_produtor e semaforos partilhados.") << std::endl;
        return nullptr;
    }

    //cria se um file maping
    HANDLE hMapFile =
            CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(SharedMemoryMap_control),
                              SHARED_MEMORY_NAME);
    DWORD getlasterror = GetLastError();
    if (hMapFile == nullptr) {
        tcerr << t("Não foi possivel criar o file mapping por uma razão desconhecida: ")
              << getlasterror << std::endl;
        return nullptr;
    }
    if (getlasterror == ERROR_ALREADY_EXISTS) {
        tcerr << t("O Control já existe.") << std::endl;
        return nullptr;
    }
    auto pBuf = (SharedMemoryMap_control *) MapViewOfFile(hMapFile,   // handle to map object
                                                          FILE_MAP_ALL_ACCESS, // read/write permission
                                                          0,
                                                          0,
                                                          sizeof(SharedMemoryMap_control)
    );

    if (pBuf == nullptr) {
        _tprintf(TEXT("Could not map view of file (%d).\n"),
                 GetLastError());
        CloseHandle(hMapFile);
        return nullptr;
    }

    CRITICAL_SECTION criticalSection;
    if (!InitializeCriticalSectionAndSpinCount(&criticalSection, 0x0000400)) {
        CloseHandle(hMapFile);
        UnmapViewOfFile(pBuf);
        return nullptr;
    }

    if (!setup_do_registry(max_avioes, max_aeroportos)) {
        CloseHandle(hMapFile);
        UnmapViewOfFile(pBuf);
        LeaveCriticalSection(&criticalSection);
        return nullptr;
    }

    // a razão para usar o unique pointer é para que o compilar n esteja a chamar o contrutor ou o descontrutor desnecessariamente
    // com um unique pointer certificamo nos que n existem copias descessarias do Control ou referencias para ele
    return std::move(std::make_unique<Control>(max_avioes, max_aeroportos, hMapFile, pBuf, criticalSection));
}

void Control::liberta_o_jack() {
    SetEvent(shared_control_aviao::get()->evento_JackTheReaper);
    auto guard = CriticalSectionGuard(critical_section_interno);
    terminar = true;
    tcout << t("Isto é o Big Crunch deste universo, foi um prazer poder viajar consigo") << std::endl;
}

Control::~Control() {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing Control...") << std::endl;
#endif
    UnmapViewOfFile(view_of_file_pointer);
    CloseHandle(shared_memory_handle);
    this->liberta_o_jack();
}


std::optional<AviaoSharedObjects_control> AviaoSharedObjects_control::create(unsigned long id_aviao) {
    TCHAR nome[30];
    //shared memory name
    _stprintf(nome, FM_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;
    HANDLE filemap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nome);
    if (!filemap)
        return std::nullopt;
    auto sharedMensagemAviao =
            (Mensagem_Aviao *) MapViewOfFile(filemap, FILE_MAP_ALL_ACCESS,
                                             0, 0, sizeof(Mensagem_Aviao));
    if (!sharedMensagemAviao) {
        CloseHandle(filemap);
        return std::nullopt;
    }

    _stprintf(nome, SR_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;

    //shared semafores
    HANDLE semaforo_read = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, nome);
    _stprintf(nome, SW_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;

    HANDLE semaforo_write = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, nome);
    _stprintf(nome, MT_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;
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
        return std::nullopt;
    }

    return AviaoSharedObjects_control(mutex, semaforo_write, semaforo_read, filemap,
                                      sharedMensagemAviao);
}

AviaoSharedObjects_control::AviaoSharedObjects_control(HANDLE mutex, HANDLE semaforo_write, HANDLE semaforo_read,
                                                       HANDLE filemap, Mensagem_Aviao *sharedMensagemAviao)
        : mutex(mutex), semaforo_write(semaforo_write), semaforo_read(semaforo_read),
          filemap(filemap), sharedMensagemAviao(sharedMensagemAviao), deleted(false) {}

AviaoSharedObjects_control::~AviaoSharedObjects_control() {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing AviaoSharedObjects_control...") << std::endl;
#endif
    if (!deleted) {
        UnmapViewOfFile(sharedMensagemAviao);
        CloseHandle(filemap);
        if (mutex)
            CloseHandle(mutex);
        if (semaforo_read)
            CloseHandle(semaforo_read);
        if (semaforo_write)
            CloseHandle(semaforo_write);
    }
}

AviaoSharedObjects_control::AviaoSharedObjects_control(AviaoSharedObjects_control &&other)
 noexcept         : mutex(other.mutex), semaforo_write(other.semaforo_write), semaforo_read(other.semaforo_read),
          filemap(other.filemap), sharedMensagemAviao(other.sharedMensagemAviao), deleted(false) {
    other.deleted = true;
}

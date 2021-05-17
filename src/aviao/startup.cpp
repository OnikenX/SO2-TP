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
    unsigned long id_aeroporto = av.IDAv;
    HANDLE process = GetCurrentProcess();
    av.IDAv = (unsigned long) GetProcessId(process);
    CloseHandle(process);
    auto coms = AviaoSharedObjects_aviao::create(av.IDAv);
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
                                                 dllHandle, av, std::move(coms), id_aeroporto);

    if (!aviao->verifica_criacao_com_control())
        return nullptr;


    return aviao;
}


AviaoInstance::AviaoInstance(HANDLE hMapFile, SharedMemoryMap_control *sharedMemoryMap, void *dllHandle,
                             AviaoShare av, std::unique_ptr<AviaoSharedObjects_aviao> sharedComs,
                             unsigned long id_do_aeroporto)
        : hMapFile(hMapFile), sharedMemoryMap(sharedMemoryMap), dllHandle(dllHandle),
          id_do_aeroporto(id_do_aeroporto), aviao(av), sharedComs(std::move(sharedComs)) {
    ptr_move_func = GetProcAddress((HMODULE) dllHandle, "move");
}

int AviaoInstance::move(int cur_x, int cur_y, int final_dest_x,
                        int final_dest_y, int *next_x, int *next_y) {
    auto move_fnc = (int (*)(int, int, int, int, int *, int *)) ptr_move_func;
    return move_fnc(cur_x, cur_y, final_dest_x, final_dest_y, next_x, next_y);
}

AviaoInstance::~AviaoInstance() {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing AviaoInstance...") << std::endl;
#endif
    FreeLibrary((HMODULE) dllHandle);
    UnmapViewOfFile(sharedMemoryMap);
    CloseHandle(hMapFile);

#ifdef _DEBUG
    tcout << t("[DEBUG]: Avião destruido.") << std::endl;
#endif
}

bool AviaoInstance::verifica_criacao_com_control() {
    Mensagem_Control mensagemControl{};
    mensagemControl.type = Mensagem_types::confirmar_novo_aviao;
    mensagemControl.id_aviao = aviao.IDAv;
    mensagemControl.mensagem.pedidoConfirmarNovoAviao.av = aviao;
    mensagemControl.mensagem.pedidoConfirmarNovoAviao.id_aeroporto = this->id_do_aeroporto;
    _tprintf(t("fico a espera...\n"));
    auto resposta = this->sendMessage(true, mensagemControl);

    if (resposta->resposta_type == lol_ok) {
#ifdef _DEBUG
        tcout << t("[DEBUG]: Creação aceita com sucesso!") << std::endl;
#endif
        return true;
    } else {
        tstring causa = "";
        switch (resposta->resposta_type) {
            case aeroporto_nao_existe:
                causa = t("aeroporto_nao_existe");
                break;
            default:
                causa = t("nao_sabida");
                break;
        }
        tcerr << t("[ERRO]: Criação não foi aceite! Causa: ") << causa << std::endl;
        return false;
    }
}

std::unique_ptr<AviaoSharedObjects_aviao> AviaoSharedObjects_aviao::create(unsigned long id_aviao) {
    TCHAR nome[30]{};
    //shared memory name
    _stprintf(nome, FM_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;
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
    HANDLE mutex_em_andamento = CreateMutex(nullptr, FALSE, nullptr);
    HANDLE mutex_mensagens = CreateMutex(nullptr, FALSE, nullptr);
    _stprintf(nome, SR_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;
    HANDLE semaforo_read = CreateSemaphore(nullptr, 0, 1, nome);
    _stprintf(nome, SW_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;
    HANDLE semaforo_write = CreateSemaphore(nullptr, 1, 1, nome);
    _stprintf(nome, MT_AVIAO, id_aviao);
//    tcout << t("Nome: ") << nome << t(" ; id_aviao : ") << id_aviao << std::endl;
    HANDLE mutex_produtores = CreateMutex(nullptr, FALSE, nome);
    if (!semaforo_read || !semaforo_write || !mutex_produtores || !mutex_mensagens || !mutex_em_andamento) {
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
        if(mutex_em_andamento)
            CloseHandle(mutex_em_andamento);
        return nullptr;
    }

    return std::make_unique<AviaoSharedObjects_aviao>(mutex_mensagens, mutex_produtores, semaforo_write, semaforo_read,
                                                      filemap,
                                                      sharedMensagemAviao, mutex_em_andamento);
}

AviaoSharedObjects_aviao::AviaoSharedObjects_aviao(HANDLE mutex_mensagens, HANDLE mutex_produtor,
                                                   HANDLE semaforo_write, HANDLE semaforo_read, HANDLE filemap,
                                                   Mensagem_Aviao *sharedMensagemAviao, HANDLE mutex_em_andamento)
        : mutex_produtor(mutex_produtor), semaforo_write(semaforo_write), semaforo_read(semaforo_read),
          filemap(filemap), sharedMensagemAviao(sharedMensagemAviao), mutex_mensagens(mutex_mensagens), mutex_em_andamento(mutex_em_andamento) {}

AviaoSharedObjects_aviao::~AviaoSharedObjects_aviao() {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing AviaoSharedObjects_aviao...") << std::endl;
#endif
    UnmapViewOfFile(sharedMensagemAviao);
    CloseHandle(filemap);
    if (mutex_produtor)
        CloseHandle(mutex_produtor);
    if (semaforo_read)
        CloseHandle(semaforo_read);
    if (semaforo_write)
        CloseHandle(semaforo_write);
}

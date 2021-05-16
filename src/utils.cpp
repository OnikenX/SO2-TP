//
// Created by OnikenX on 5/15/2021.
//

#include <utils.hpp>

SharedLocks::SharedLocks() :
        semaforo_read_control_aviao(nullptr), semaforo_write_control_aviao(nullptr),
        mutex_partilhado(nullptr), firsttime(true), erros(false), evento_JackTheReaper(nullptr) {}

SharedLocks *SharedLocks::get() {
    static SharedLocks s;
    if (s.firsttime) {
        s.semaforo_read_control_aviao =
                CreateSemaphore(NULL, 0, CIRCULAR_BUFFERS_SIZE, SEMAFORO_READ_CONTROL_AVIAO);
        s.semaforo_write_control_aviao =
                CreateSemaphore(NULL, CIRCULAR_BUFFERS_SIZE, CIRCULAR_BUFFERS_SIZE, SEMAFORO_WRITE_CONTROL_AVIAO);
        s.evento_JackTheReaper = CreateEvent(nullptr, TRUE, FALSE, EVENT_KILLER);
        SetLastError(0);
        s.mutex_partilhado = CreateMutex(NULL, FALSE, MUTEX_PARTILHADO);
        s.firsttime = false;
        if (!s.semaforo_read_control_aviao || !s.semaforo_write_control_aviao
            || !s.mutex_partilhado || !s.evento_JackTheReaper) {
            s.closeall();
            s.erros = true;
            return nullptr;
        }
        if(GetLastError() != ERROR_ALREADY_EXISTS)
            ReleaseMutex(s.mutex_partilhado);
        return &s;
    } else {
        if (s.erros)
            return nullptr;
        else
            return &s;
    }
}

void SharedLocks::closeall() {
    if (this->semaforo_read_control_aviao)
        CloseHandle(this->semaforo_read_control_aviao);
    if (this->semaforo_write_control_aviao)
        CloseHandle(this->semaforo_write_control_aviao);
    if (this->evento_JackTheReaper)
        CloseHandle(this->evento_JackTheReaper);
    if (this->mutex_partilhado)
        CloseHandle(this->mutex_partilhado);
    this->semaforo_read_control_aviao = this->semaforo_write_control_aviao = nullptr;
}


SharedLocks::~SharedLocks() {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing SharedLocks...") << std::endl;
#endif
    closeall();
}


//implementacao do guard lock
GuardLock::GuardLock(HANDLE _mutex) : mutex(_mutex) {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Geting lock\n");
#endif
    auto switcher = WaitForSingleObject(mutex, INFINITE);
#ifdef _DEBUG
    tcout << t("[DEBUG]: waited\n");
#endif
    switch (switcher) {
        case WAIT_ABANDONED:
            throw std::exception("Problem in creating guard: WAIT_ABANDONED!");
            break;
        case WAIT_OBJECT_0:
            // SUCCESS
            break;
        case WAIT_TIMEOUT:
            throw std::exception("Problem in creating guard: WAIT_TIMEOUT!");
            break;
        case WAIT_FAILED:
            throw std::exception("Problem in creating guard: WAIT_FAILED!");
            break;
        default:
            std::ostringstream stream;
            stream << "Problem in creating guard: UNSPECIFIED" << switcher;
            throw std::exception(stream.str().c_str());
            break;
    }
}

GuardLock::~GuardLock() {
    ReleaseMutex(mutex);
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing GuardLock...") << std::endl;
#endif
}

GuardLock::GuardLock(GuardLock &&guard) {
    this->mutex = guard.mutex;
}


SharedMemoryMap_control::SharedMemoryMap_control() : se_pode_criar_mais_avioes(true), nAvioes(0), terminar(false),
                                                     posReader(0), posWriter(0) {
    memset(buffer_mensagens_control, 0, sizeof(Mensagem_Control) * CIRCULAR_BUFFERS_SIZE);
}

bool Cords::isEqual(Cords &outro) {
    return (this->x == outro.x && this->y == outro.y);
}

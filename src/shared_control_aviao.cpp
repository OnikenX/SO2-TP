//
// Created by OnikenX on 6/2/2021.
//

#include "shared_control_aviao.hpp"


shared_control_aviao::shared_control_aviao() :
        semaforo_read_control_aviao(nullptr), semaforo_write_control_aviao(nullptr),
        mutex_partilhado(nullptr), firsttime(true), erros(false), evento_JackTheReaper(nullptr) {}

shared_control_aviao *shared_control_aviao::get() {
    static shared_control_aviao s;
    if (s.firsttime) {
        s.semaforo_read_control_aviao =
                CreateSemaphore(nullptr, 0, CIRCULAR_BUFFERS_SIZE, SEMAFORO_READ_CONTROL_AVIAO);
        s.semaforo_write_control_aviao =
                CreateSemaphore(nullptr, CIRCULAR_BUFFERS_SIZE, CIRCULAR_BUFFERS_SIZE, SEMAFORO_WRITE_CONTROL_AVIAO);
        s.evento_JackTheReaper = CreateEvent(nullptr, TRUE, FALSE, EVENT_KILLER);
        SetLastError(0);
        s.mutex_partilhado = CreateMutex(nullptr, FALSE, MUTEX_PARTILHADO);
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

void shared_control_aviao::closeall() {
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


shared_control_aviao::~shared_control_aviao() {
#ifdef _DEBUG
    tcout << t("[DEBUG]: Destroing shared_control_aviao...") << std::endl;
#endif
    closeall();
}


SharedMemoryMap_control::SharedMemoryMap_control() : posReader(0), posWriter(0) {
    memset(buffer_mensagens_control, 0, sizeof(Mensagem_Control_aviao) * CIRCULAR_BUFFERS_SIZE);
}

bool Cords::isEqual(Cords &outro) const {
    return (this->x == outro.x && this->y == outro.y);
}


CriticalSectionGuard::CriticalSectionGuard(CRITICAL_SECTION &criticalSection)
        : criticalSection(criticalSection) {
    EnterCriticalSection(&this->criticalSection);
}

CriticalSectionGuard::~CriticalSectionGuard() {
    LeaveCriticalSection(&criticalSection);

}
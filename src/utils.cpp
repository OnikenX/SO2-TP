//
// Created by OnikenX on 5/15/2021.
//

#include <utils.hpp>


//implementacao do guard lock
GuardLock::GuardLock(HANDLE _mutex) : mutex(_mutex) {
#ifdef _DEBUG_LOCK
    tcout << t("[DEBUG]: Geting lock\n");
#endif
    auto switcher = WaitForSingleObject(mutex, INFINITE);
#ifdef _DEBUG_LOCK
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
#ifdef _DEBUG_LOCK
    tcout << t("[DEBUG]: Destroing GuardLock...") << std::endl;
#endif
}

GuardLock::GuardLock(GuardLock &&guard) noexcept {
    this->mutex = guard.mutex;
}


SharedMemoryMap_control::SharedMemoryMap_control() : posReader(0), posWriter(0) {
    memset(buffer_mensagens_control, 0, sizeof(Mensagem_Control) * CIRCULAR_BUFFERS_SIZE);
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


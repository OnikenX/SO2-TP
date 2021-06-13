//
// Created by OnikenX on 5/15/2021.
//

#include "utils.hpp"


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



//le um numero, returna o em resulting_value e returna se ouve um erro
bool ler_numero(const TCHAR *string_numero, long& resulting_value) {
    errno = 0;
    TCHAR* p_end;
    const long i = _tcstol(string_numero, &p_end, 10);
    const bool range_error = errno == ERANGE;
    if (range_error || i <= 0){
        tcerr << "[ERROR]: Range error ou invalied number occurred(only positive numbers).";
        return false;
    }
    resulting_value = i;
    return true;
}

DWORD WINAPI Limbo(LPVOID param) {
#ifdef _DEBUG
    tcout << t("[DEBUG]: waiting for death ...") << std::endl;
#endif
    HANDLE evento = CreateEvent(nullptr, TRUE, FALSE, EVENT_KILLER);
    WaitForSingleObject(evento, INFINITE);
#ifdef _DEBUG
    tcout << t("[DEBUG]: BOOM !!X\\ RIP In Piece );") << std::endl;
#endif
    return 1;
}

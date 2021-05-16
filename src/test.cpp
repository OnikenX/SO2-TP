//
// Created by OnikenX on 5/14/2021.
//
#include <Windows.h>
#include <utils.hpp>
DWORD WINAPI ThreadName(LPVOID param){
    for (int i=1;i< 10; ++i){
        tcout << i << t(" thread 2 antes...") << std::endl;
         WaitForSingleObject(SharedLocks::get()->mutex_partilhado, INFINITE);
        Sleep(500);
        tcout << i << t(" thread 2 depois ...") << std::endl;
        ReleaseMutex(SharedLocks::get()->mutex_partilhado);
    }
return 1;
}
int _tmain() {
    HANDLE thread = CreateThread(NULL, 0, ThreadName, NULL, 0, NULL);
    for (int i=1;i< 10; ++i){
        tcout << i << t(" thread 1 antes...") << std::endl;
        auto guard1 = GuardLock(SharedLocks::get()->mutex_partilhado);
        tcout << i << t(" thread 1 depois ...") << std::endl;
    }

    WaitForSingleObject(thread, INFINITE);
    return 0;
}
//
// Created by OnikenX on 5/14/2021.
//
#include <Windows.h>
#include <utils.hpp>

DWORD WINAPI ThreadName(LPVOID param) {
    tcout << t("waiting ...") << std::endl;
    HANDLE evento = CreateEvent(NULL, TRUE, FALSE, t("evento_test"));

    WaitForSingleObject(evento, INFINITE);
    tcout << t("waited!") << std::endl;
    return 1;
}

int _tmain() {
    SetLastError(ERROR_SUCCESS);
    HANDLE evento = CreateEvent(NULL, TRUE, FALSE, t("evento_test"));
    if (!evento) {
        tcout << t("fudeu!");
        return -1;
    }
    HANDLE processes[10];
    for (int i = 0; i < 10; i++)
        processes[i] = CreateThread(NULL, 0, ThreadName, NULL, 0, NULL);
    Sleep(2000);
    SetEvent(evento);
    WaitForMultipleObjects(10,processes, true, INFINITE);
}
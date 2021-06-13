
#pragma once

//standard c++
#include <memory>
#include <sstream>
#include <iostream>
#include <sstream>
#include <optional>//RUST Team
#include <chrono>
//windows and tchar
#include <windows.h>
#include <tchar.h>

//set mode for the terminal
#include <io.h>
#include <fcntl.h>

//typedef for tchar to std types
#ifdef UNICODE
#define tcout std::wcout
#define tcerr std::wcerr
#define tcin std::wcin
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcerr std::cerr
#define tcin std::cin
#define tstring std::string
#define tstringstream std::stringstream
#endif

//defines

// shortener for the TEXT macro
#define t(quote) TEXT(quote)
#define EVENT_KILLER t("Nos_Vamos_Todos_Falecer")

#define ERRO_ARGUMENTOS 1
#define ERRO_CONTROL_EXISTE 2
#define ERRO_CONTROL_NAO_EXISTE 3
#define ERRO_AVIAO_NAO_FOI_CRIADO 4



struct GuardLock {
    ~GuardLock();

    HANDLE mutex;

    explicit GuardLock(HANDLE _mutex);

    GuardLock(GuardLock &&guard) noexcept;
};

struct CriticalSectionGuard {
    ~CriticalSectionGuard();

    CRITICAL_SECTION &criticalSection;

    explicit CriticalSectionGuard(CRITICAL_SECTION &criticalSection);
};

bool ler_numero(const TCHAR *string_numero, long& resulting_value);


//uma thread que espera pelo sinal de morrer
DWORD WINAPI Limbo(LPVOID param);
//
// Created by OnikenX on 5/14/2021.
//
#include "./control/control.hpp"
int _tmain() {

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif


    DWORD max_avioes =10, max_aeroportos = 10;

    //cria se um file maping
    HANDLE hMapFile =
            CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(shared_memory_map),SHARED_MEMORY_NAME);

    if (hMapFile == NULL) {
        DWORD getlasterror = GetLastError();
        if (getlasterror == ERROR_ALREADY_EXISTS) {
            tcout << t("O Control já existe.") << std::endl;
        } else {
            tcout << t("Não foi possivel criar o file mapping por uma razão disconhecida: ") << getlasterror
                  << std::endl;
        }
        return 1;
    }

    auto pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                                       FILE_MAP_ALL_ACCESS, // read/write permission
                                       0,
                                       0,
                                       sizeof(shared_memory_map)
    );

    if (pBuf == NULL) {
        _tprintf(TEXT("Could not map view of file (%d).\n"),
                 GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

//    bool return_value = setup_do_registry(max_avioes, max_aeroportos);

    CloseHandle(hMapFile);
    UnmapViewOfFile(pBuf);

    return 0;
//    if (return_value)
//        return 0;
//    else
//        return 1;
}
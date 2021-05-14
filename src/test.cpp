//
// Created by OnikenX on 5/14/2021.
//

#include "./control/control.hpp"
#include <utils.h>
int _tmain() {

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
    return 0;
}
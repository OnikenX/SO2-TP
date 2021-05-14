#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>
#include "utils.h"




int _tmain(int argc, TCHAR *argv[]){
    if(argc!=4){
        return ERRO_ARGUMENTOS;
    }


    aviao a;
    a.CapMax = (int)argv[1];
    a.velocidade = (int)argv[2];
    a.IDAv = (int)argv[3];

//mandar para o control, por os IDs certos

    
auto ControlRunning = Wrappers::Handle<HANDLE>(CreateMutex(0, 0,  SHARED_MEMORY_NAME));
 
//verifica se 
if(GetLastError() == ERROR_SUCCESS){
    _tprintf(t("Control não iniciado."));
    return ERRO_CONTROL_NAO_EXISTE;
}


TCHAR* fileViewMap;
HANDLE hFileMap;
 hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 100 * sizeof(TCHAR), TEXT("SO2_MEMORIA_PARTILHADA"));
    if (hFileMap == NULL) {
        _tprintf(TEXT("Erro no CreateFileMapping\n"));
        return -1;
    }


fileViewMap = (TCHAR*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (fileViewMap == NULL) {
        _tprintf(TEXT("Erro no MapViewOfFile\n"));
        return -1;
    }

    //WaitForSingleObject(hSemaforo, INFINITE);
return 0;    
}
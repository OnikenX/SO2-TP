#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include "utils.h"


int _tmain(){
    
auto ControlRunning = Wrappers::Handle<HANDLE>(CreateMutex(0, 0,  MUnique));

//verifica se 
if(GetLastError() == ERROR_SUCCESS){
    _tprintf(t("Control não iniciado."));
    return 1;
}



    //WaitForSingleObject(hSemaforo, INFINITE);
return 0;    
}
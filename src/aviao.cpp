#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>


void _tmain(){
    
HANDLE ControlRunning = CreateMutex (0, 0,  (LPCWSTR)"Controlador_42");

if(GetLastError() == ERROR_SUCCESS){
    CloseHandle(ControlRunning);
    return;
}

    //WaitForSingleObject(hSemaforo, INFINITE);
    
}
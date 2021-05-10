#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

void _tmain(){
    HANDLE hFileMap;
    HANDLE hSemaforo;
    TCHAR par_nome[50] = TEXT("MaxAvioes");
    int MAXAv = 10; //Default
    TCHAR chave_nome[50] = TEXT("SOFTWARE\\TP_SO2\\Avioes");
    HKEY chave;
	DWORD resultado;
    HANDLE Unique = CreateMutex (0, 0, (LPCWSTR)"Controlador_42");

    if (GetLastError() == ERROR_ALREADY_EXISTS)  return;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &resultado) != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro a criar Chave ); \n"));
		return;
	}
    if (RegSetValueEx(chave, par_nome, 0, REG_SZ, (LPBYTE)MAXAv, sizeof(MAXAv)) != ERROR_SUCCESS) {
				_tprintf(TEXT("Erro a aceder a Chave\n"));
	}

    hSemaforo = CreateSemaphore(NULL, MAXAv, MAXAv, TEXT("SO2_SEMAFORO"));
    if (hSemaforo == NULL) {
        _tprintf(TEXT("Erro no CreateSemaphore\n"));
        return ;
    }





/*
    if (RegDeleteValue(chave, par_nome) != ERROR_SUCCESS) {
		_tprintf(TEXT("Esse Registro não existe...\n"));
	}
	else {
		_tprintf(TEXT("Registro Eliminado com sucesso(espero eu)\n"));
	}
*/


}
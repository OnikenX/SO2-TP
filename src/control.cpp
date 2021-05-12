#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include "utils.h"

std::optional<Wrappers::Handle<HANDLE>> verifica_se_o_control_ja_existe()
{
    auto Unique = Wrappers::Handle<HANDLE>{CreateMutex(0, 0, MUnique)};

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        tcerr << t("Control já existe.") << std::endl;
        return std::nullopt;
    }
    return std::optional(std::move(Unique));
}

int _tmain()
{
    HANDLE hFileMap;
    HANDLE hSemaforo;
    TCHAR par_nome[50] = TEXT("MaxAvioes");
    int MAXAv = 10; //Default
    TCHAR chave_nome[50] = TEXT("SOFTWARE\\TP_SO2\\Avioes");
    HKEY chave;
    DWORD resultado;

    // o flag do control guarda um optional, optional 
    auto flag_do_control = verifica_se_o_control_ja_existe();

        if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &resultado) != ERROR_SUCCESS)
        {
            tcerr << t("Erro a criar Chave ); \n") << std::endl;
            return 1;
        }
    if (RegSetValueEx(chave, par_nome, 0, REG_SZ, (LPBYTE)MAXAv, sizeof(MAXAv)) != ERROR_SUCCESS)
    {
        tcerr << t("Erro a aceder a Chave\n") << std::endl;
    }

    hSemaforo = CreateSemaphore(NULL, MAXAv, MAXAv, TEXT("SO2_SEMAFORO"));
    if (hSemaforo == NULL)
    {
        _tprintf(TEXT("Erro no CreateSemaphore\n"));
        return;
    }

    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 100 * sizeof(TCHAR), TEXT("SO2_MEMORIA_PARTILHADA"));
    if (hFileMap == NULL)
    {
        _tprintf(TEXT("Erro no CreateFileMapping\n"));
        return;
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
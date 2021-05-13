#include "control.hpp"
//funções do control que servem de inicialização



Control::Control(unsigned char max_avioes, HANDLE mutex_unico) : MAX_AVIOES(max_avioes), mutex_unico(mutex_unico)
{
	setup_do_registry();
}

std::optional<HANDLE> Control::verifica_se_o_control_ja_existe()
{
	// HANDLE Unique = CreateMutex(0, 0, MUnique);
	HANDLE semaforo_reads = CreateSemaphore(NULL, max_avioes, max_avioes, TEXT("Semaforo_Para_Ler"));
	HANDLE semaforo_writes =  CreateSemaphore(NULL, 0, max_avioes, TEXT("Semaforo_Para_Escrever"));

	//criar mutex para os produtores
    HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("Mutex_Para_Control"));

    if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL || dados.hMutex == NULL) {
        _tprintf(TEXT("Erro no CreateSemaphore ou no CreateMutex\n"));
        return std::nullopt;
    }
	
	HANDLE memoria_partilhada = 
	if (Unique == NULL)
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			tcerr << t("Control já existe.") << std::endl;
			return std::nullopt;
		}
	return std::optional(Unique);
}

int Control::setup_do_registry()
{
	HKEY chave;
	DWORD resultado;
	const TCHAR par_nome[] = TEXT("MaxAvioes");
	const TCHAR chave_nome[] = TEXT("SOFTWARE\\TP_SO2\\Avioes");

	if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &resultado) != ERROR_SUCCESS)
	{
		tcerr << t("Erro a criar Chave ); \n") << std::endl;
		return 1;
	}

	if (RegSetValueEx(chave, par_nome, 0, REG_SZ, &this->MAX_AVIOES, sizeof(this->MAX_AVIOES)) != ERROR_SUCCESS)
	{
		tcerr << t("Erro a aceder a Chave\n") << std::endl;
	}
}

std::optional<Control> Control::create(unsigned char MAX_AVIOES)
{
	auto flag_do_control = verifica_se_o_control_ja_existe();
	if (!flag_do_control.has_value())
	{
		return std::nullopt;
	}
}


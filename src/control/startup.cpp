#include "control.hpp"

Vars::Vars() : MAX_AVIOES(10) {}

std::optional<HANDLE> verifica_se_o_control_ja_existe()
{
	HANDLE Unique = CreateMutex(0, 0, MUnique);
	if (Unique == NULL)
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			tcerr << t("Control já existe.") << std::endl;
			return std::nullopt;
		}
	return std::optional(Unique);
}

int setup_do_registry(Vars &variaveis_globais)
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

	if (RegSetValueEx(chave, par_nome, 0, REG_SZ, &variaveis_globais.MAX_AVIOES, sizeof(variaveis_globais.MAX_AVIOES)) != ERROR_SUCCESS)
	{
		tcerr << t("Erro a aceder a Chave\n") << std::endl;
	}
}

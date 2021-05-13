#include "control.hpp"
//funções do control que servem de inicialização

Control::Control(unsigned char max_avioes, HANDLE mutex_unico) : MAX_AVIOES(max_avioes), mutex_unico(mutex_unico)
{
	setup_do_registry();
	aceita_avioes = true;
}

std::optional<HANDLE>
Control::get_map_file_handle()
{
	//cria se um file maping
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(shared_memory_map),
		szName);
	if (hMapFile == NULL)
	{
		DWORD getlasterror = GetLastError();
		if (getlasterror == ERROR_ALREADY_EXISTS)
		{
			tcout << t("O Control já existe.") << std::endl;
		}
		else
		{
			tcout << t("Não foi possivel criar o file mapping por uma razão disconhecida: ") << getlasterror << std::endl;
		}
		return std::nullopt;
	};
	return std::optional(hMapFile);
}

int Control::setup_do_registry()
{
	HKEY chave;
	DWORD resultado;
	const TCHAR par_nome[] = TEXT("MaxAvioes");
	const TCHAR chave_nome[] = TEXT("SOFTWARE\\TP_SO2\\Avioes");

	if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &resultado) != ERROR_SUCCESS)
	{
		tcerr << t("Erro a criar Chave") << std::endl;
		return 1;
	}

	if (RegSetValueEx(chave, par_nome, 0, REG_SZ, &this->MAX_AVIOES, sizeof(this->MAX_AVIOES)) != ERROR_SUCCESS)
	{
		tcerr << t("Erro a aceder a Chave") << std::endl;
	}
}

std::optional<Control> Control::create(unsigned char MAX_AVIOES)
{
	auto flag_do_control = verifica_se_o_control_ja_existe();
	if (!flag_do_control.has_value())
	{
		return std::nullopt;
	}
	return std::optional(Control(MAX_AVIOES, flag_do_control.value()));
}

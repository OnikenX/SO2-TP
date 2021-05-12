#include "control.hpp"

int main()
{
	Vars variaveis_globais;
	auto flag_do_control = verifica_se_o_control_ja_existe();
	if (!flag_do_control.has_value())
	{
		return 1;
	}

	setup_do_registry(variaveis_globais);

	//salva guarda que handle é fechado caso o programa é fechado em alguma altura
	auto mutex_unique = Wrappers::Handle<HANDLE>(flag_do_control.value());

	auto hSemaforo = Wrappers::Handle<HANDLE>{CreateSemaphore(NULL, variaveis_globais.MAX_AVIOES, variaveis_globais.MAX_AVIOES, TEXT("SO2_SEMAFORO"))};
	if (hSemaforo() == NULL)
	{
		tcerr << t("Erro no CreateSemaphore\n") << std::endl;
		return -1;
	}

	auto hFileMap = Wrappers::Handle<HANDLE>{CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 100 * sizeof(TCHAR), TEXT("SO2_MEMORIA_PARTILHADA"))};
	if (hFileMap() == NULL)
	{
		tcerr << t("Erro no CreateFileMapping\n") << std::endl;
		return -1;
	}
}
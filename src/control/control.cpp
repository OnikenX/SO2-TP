#include "control.hpp"

int Control::run()
{
	return 0;
}

int main()
{

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	Dados d;

	auto control_create = Control::create(d);
	if (!control_create.has_value())
		tcout << t("Control não pode ser criado.") << std::endl;
	Control control = std::move(control_create.value());

	int return_control = control.run(d);
	if (return_control != 0)
	{
		tcout << t("Erro ") << return_control << t(" a correr o control.") << std::endl;
	}
	return return_control;
}
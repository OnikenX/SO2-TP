#include "menu.hpp"

Menu::Menu(Control &control): control(control), counter_avioes(0){}


void Menu::run()
{
	TCHAR op[25];
	do
	{
		tcout << (TEXT("\nO que deseja fazer?\n"));
		tcout << (TEXT("*********************************\n"));
		tcout << (TEXT("*********************************\n"));
		tcout << (TEXT("**   1 - Cria Aeroporto        **\n"));
		tcout << (TEXT("**   2 - Consultar Aeroporto   **\n"));
		tcout << (TEXT("**   3 - Consultar Avião       **\n"));
		tcout << (TEXT("**   4 - Consultar Passageiros **\n"));
		tcout << (TEXT("**   5 - No More Planes        **\n"));
        tcout << (TEXT("**   6 - Shutdown total        **\n"));
		tcout << (TEXT("**                             **\n"));
		tcout << (TEXT("**   0 - Abandonar             **\n"));
		tcout << (TEXT("*********************************\n"));
		tcout << (TEXT("*********************************\n\n"));
		_fgetts(op, 25, stdin);
		op[_tcslen(op) - 1] = t('\0');
		if (_tcscmp(op, t("0")) == 0)
		{
			break;
		}
		else if (_tcscmp(op, t("1")) == 0)
		{
			cria_aeroporto();
		}
		else if (_tcscmp(op, t("2")) == 0)
		{
			consulta_aeroporto();
		}
		else if (_tcscmp(op, t("3")) == 0)
		{
			consultar_aviao();
		}
		else if (_tcscmp(op, t("4")) == 0)
		{
			//consultar_passageiros();
			tcout << t("Essa opção está indesponivel, compre a nova versão para desbloquear\n");
		}
        else if (_tcscmp(op, t("5")) == 0)
		{
			desativa_novos_avioes();
		}
		else if (_tcscmp(op, t("6")) == 0)
		{
			mata_tudo();
		}
		else
		{
			tcout << t("Essa opção quase que existe, não queres tentar algo que seja possivel?\n");
		}
	} while (1);
}

void Menu::cria_aeroporto(){
    Aeroporto a;
    tcout << t("Insira as Coordenadas do novo Aeroporto:") << std::endl;
    tcout << t("X:") << std::endl;
    std::cin >> a.pos.x;
    tcout << t("Y:") << std::endl;
    std::cin >> a.pos.y;
    a.IDAero = this->counter_avioes++;
}

void Menu::consulta_aeroporto(){
    for(int i=0;i<counter_avioes;i++){
        tcout << t("Avião nr ") << this->control.aeroportos[i].IDAero << std::endl;
        tcout << t("X-> ") << this->control.aeroportos[i].pos.x << t("Y-> ") << this->control.aeroportos[i].pos.y << std::endl;
    }
}
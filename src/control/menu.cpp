#include "menu.hpp"

Menu::Menu(Control &control) : control(control), counter_avioes(0), counter_aeroporto(0) {}


void Menu::run() {
    TCHAR op[25];
    bool exit = false;
    do {
        tcout << t("\nO que deseja fazer?\n");
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("**   1 - Cria Aeroporto        **") << std::endl;
        tcout << t("**   2 - Consultar Aeroporto   **") << std::endl;
        tcout << t("**   3 - Consultar Avião       **") << std::endl;
        tcout << t("**   4 - Consultar Passageiros **") << std::endl;
        if (this->control.aceita_avioes)
            tcout << t("**   5 - No More Planes        **") << std::endl;
        else
            tcout << t("**   5 - More Planes Please    **") << std::endl;
        tcout << t("**                             **") << std::endl;
        tcout << t("**   6 - Shutdown total        **") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl << std::endl;
//        _fgetts(op, 25, stdin);

        tcout << t("> ");
        tcout.flush();
        int input = -1;
        if(input == 0){
            tcout << t("Por favor insira um numero positivo entre 1 a 6 > ");
            tcout.flush();
        }
        try {
            do {
                tcin >> input;
            } while (input <= 0);
        } catch (std::exception e) {
            std::cout << e.what();
            break;
        }
//        op[_tcslen(op) - 1] = t('\0');
        tcout << t("Input: ") << input << std::endl;
        switch (input) {
            case 0:
                mata_tudo();
                exit = true;
                break;
            case 1:
                cria_aeroporto();
                break;
            case 2:
                consulta_aeroporto();
                break;
            case 3:
                consultar_aviao();
                break;
            case 4:
                tcout << t("Essa opção está indesponivel, compre a nova versão para desbloquear\n");
                break;
            case 5:
                desativa_novos_avioes();
                break;
            default:
                tcout << t("Essa opção quase que existe, não queres tentar algo que seja possivel?\n");
        }

    } while (!exit);
}

void Menu::cria_aeroporto() {
    Aeroporto a;
    tcout << t("Insira as Coordenadas do novo Aeroporto:") << std::endl;
    tcout << t("X:");
    tcout.flush();
    tcin >> a.pos.x;
    tcout << t("Y:");
    tcout.flush();
    tcin >> a.pos.y;
    a.IDAero = this->counter_aeroporto++;
}

void Menu::consulta_aeroporto() {
    for (int i = 0; i < counter_aeroporto; i++) {
        tcout << t("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@") << std::endl;
        tcout << t("Aeroporto nr ") << this->control.aeroportos[i].IDAero << std::endl;
        tcout << t("X-> ") << this->control.aeroportos[i].pos.x << t("Y-> ") << this->control.aeroportos[i].pos.y
              << std::endl;
    }
    tcout << t("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@") << std::endl;
}

void Menu::consultar_aviao() {
    for (int i = 0; i < counter_avioes; i++) {
        tcout << t("#############################################################################") << std::endl;
        tcout << t("Avião nr ") << this->control.avioes[i].IDAv << std::endl;
        tcout << t("Posição atual:") << std::endl;
        tcout << t("X-> ") << this->control.avioes[i].PosA.x << t("Y-> ") << control.avioes[i].PosA.y << std::endl;
        tcout << t("Destino:") << std::endl;
        tcout << t("X-> ") << this->control.avioes[i].PosDest.x << t("Y-> ") << control.avioes[i].PosDest.y
              << std::endl;
        tcout << t("Capacidade Maxima: ") << this->control.avioes[i].CapMax << std::endl;
        tcout << t("Velocidade: ") << this->control.avioes[i].velocidade << std::endl;
    }
    tcout << t("#############################################################################") << std::endl;
}

void Menu::desativa_novos_avioes() {
    this->control.aceita_avioes = !this->control.aceita_avioes;
}

void Menu::mata_tudo() {
    control.finalizar();
    //fechar tudo e mandar msg a avisar
}
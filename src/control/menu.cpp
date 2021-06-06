#include "menu.hpp"
#include <algorithm>

Menu::Menu(Control &control) : control(control), counter_aeroporto(0) {}

void Menu::run() {
    bool exit = false;
    int input = -1;
    tstring line_input;
    do {
        tcout << t("\nO que deseja fazer?\n");
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("**   1 - Cria Aeroporto        **") << std::endl;
        tcout << t("**   2 - Consultar Aeroporto   **") << std::endl;
        tcout << t("**   3 - Consultar Avião       **") << std::endl;
        tcout << t("**   4 - Consultar Passageiros **") << std::endl;
        tcout << t("**   5 - Toggle new Planes     **") << std::endl;
        tcout << t("**                             **") << std::endl;
        tcout << t("**   6 - Shutdown total        **") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl << std::endl;
//        _fgetts(op, 25, stdin);

        tcout << t("> ");
        tcout.flush();
        if (input == 0) {
            tcout << t("Por favor insira um numero positivo entre 1 a 6 > ");
            tcout.flush();
        }
        try {
            do {
                std::getline(tcin, line_input);
                std::basic_stringstream<TCHAR> sstream(line_input);
                sstream >> input;
            } while (input <= 0);
        } catch (std::exception &e) {
            tcout << e.what();
            break;
        }
//        op[_tcslen(op) - 1] = t('\0');
        tcout << t("Input: ") << input << std::endl;
        switch (input) {
            case 6:
//                mata_tudo();
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

bool verificaMaxAeroportos(Menu *isto) {
    auto guard = CriticalSectionGuard(isto->control.critical_section_interno);
    if (isto->control.MAX_AEROPORTOS <= isto->counter_aeroporto) {
        tcout << t("Já foi atingido o limite de Aeroportos possiveis") << std::endl;
        return false;
    }
    return true;
}

void verificaAeroporto(Aeroporto& a, Control& control, bool& exit_loop){
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    auto existe = std::find_if(std::begin(control.aeroportos), std::end(control.aeroportos),
                               [&](Aeroporto tmp) { return !_tcscmp(tmp.nome, a.nome); });
    if (existe != std::end(control.aeroportos)) {
        tcout << t("Tens de ser mais original, esse nome ja foi patentiado:");
    } else {
        exit_loop = true;
    }
}

void Menu::cria_aeroporto() {
    if (!verificaMaxAeroportos(this))
        return;
    Aeroporto a;
    tcout << t("Insira as Coordenadas do novo Aeroporto:") << std::endl;
    bool aeroporto_near;
    bool exit_loop = false;
    tstring line_input;
    do {
        aeroporto_near = false;
        do {
            tcout << t("Nome do Aeroporto:");
            tcout.flush();
            tcin >> a.nome;
            verificaAeroporto(a, control, exit_loop);
        } while (!exit_loop);
        do {
            tcout << t("X:");
            tcout.flush();
            std::getline(tcin, line_input);
            std::basic_stringstream<TCHAR> sstream(line_input);
            sstream >> a.pos.x;
        } while (a.pos.x < 0 || a.pos.x > 999);
        do {
            tcout << t("Y:");
            tcout.flush();
            std::getline(tcin, line_input);
            std::basic_stringstream<TCHAR> sstream(line_input);
            sstream >> a.pos.y;
        } while (a.pos.y < 0 || a.pos.y > 999);
        {
            auto guard = CriticalSectionGuard(control.critical_section_interno);
            for (auto &elem : this->control.aeroportos) {
                int tmpX = abs(long(elem.pos.x - a.pos.x));
                int tmpY = abs(long(elem.pos.y - a.pos.y));
                if (tmpX < 10 && tmpY < 10) {
                    tcout << t("Já existe um aeroporto nas redondezas, não me parece boa ideia construir um aqui")
                          << std::endl;
                    aeroporto_near = true;
                }
            }
        }
    } while (aeroporto_near);

    a.IDAero = ++this->counter_aeroporto;
    {
        auto guard = CriticalSectionGuard(control.critical_section_interno);
        this->control.aeroportos.insert(this->control.aeroportos.end(), a);
    }
}

void Menu::consulta_aeroporto() {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    for (int i = 0; i < counter_aeroporto; i++) {
        tcout << t("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@") << std::endl;
        tcout << t("Aeroporto nr ") << this->control.aeroportos[i].IDAero << std::endl;
        tcout << t("Aeroporto ") << this->control.aeroportos[i].nome << std::endl;
        tcout << t("X-> ") << this->control.aeroportos[i].pos.x << t("\tY-> ") << this->control.aeroportos[i].pos.y
              << std::endl;
    }
    tcout << t("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@") << std::endl;
}

void Menu::consultar_aviao() {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    for (auto &aviao : control.avioes) {
        tcout << t("#############################################################################") << std::endl;
        tcout << t("Avião nr ") << aviao.IDAv << std::endl;
        tcout << t("Posição atual:") << std::endl;
        tcout << t("X-> ") << aviao.PosA.x << t("\tY-> ") << aviao.PosA.y << std::endl;
        tcout << t("Destino:") << std::endl;
        tcout << t("X-> ") << aviao.PosDest.x << t("\tY-> ") << aviao.PosDest.y << std::endl;
        tcout << t("Capacidade Maxima: ") << aviao.CapMax << std::endl;
        tcout << t("Velocidade: ") << aviao.velocidade << std::endl;
    }
    tcout << t("#############################################################################") << std::endl;
}

void Menu::desativa_novos_avioes() {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    this->control.aceita_avioes = !this->control.aceita_avioes;
}


//void Menu::mata_tudo() {
//    control.liberta_o_jack();
//    //fechar tudo e mandar msg_content a avisar
//}
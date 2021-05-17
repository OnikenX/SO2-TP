#include "menu.hpp"
#include <algorithm>

Menu::Menu(AviaoInstance &aviao) : aviaoInstance(aviao), counter_aeroporto(0) {}


void Menu::run() {
    TCHAR op[25];
    //bool em_voo=false;
    aviaoInstance.em_andamento = false;
    bool exit = false;
    do {
        //O Nuno Esteve aqui!!!
        tcout << t("\nO que deseja fazer?\n");
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("**   1 - Novo Destino          **") << std::endl;
        tcout << t("**   2 - Iniciar Voo           **") << std::endl;
        tcout << t("**                             **") << std::endl;
        tcout << t("**   9 - commit Seppuku        **") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl << std::endl;

        tcout << t("> ");
        tcout.flush();
        int input = -1;
        if (input == 0) {
            tcout << t("Por favor insira uma das opções do Menu > ");
            tcout.flush();
        }
        try {
            do {
                tcin >> input;
            } while (input <= 0);
        } catch (std::exception e) {
            tcout << e.what();
            break;
        }
        tcout << t("Input: ") << input << std::endl;
        switch (input) {

            case 9:
                    suicidio();
                    exit = true;
                    break;

            case 1:

                    novas_cords();
                    break;


            case 2:

                    inicia_voo();
                    if(aviaoInstance.em_andamento)
                        exit = true;
                    break;


            default:
                tcout << t("Essa opção quase que existe, não queres tentar algo que seja possivel?\n");
        }

    } while (!exit);
}

//

void Menu::novas_cords() {
    int idAero;
    tcout << t("Insira o ID do Aeroporto distino:") << std::endl;
    tcin >> idAero;
    Mensagem_Control mc;
    mc.type = novo_destino;
    mc.id_aviao = aviaoInstance.aviao.IDAv;
    std::unique_ptr<Mensagem_Aviao> resposta = aviaoInstance.sendMessage(true, mc);
    if (resposta->resposta_type == aeroporto_existe) {
        aviaoInstance.aviao.PosDest.x = resposta->msg_content.respostaNovasCoordenadas.x;
        aviaoInstance.aviao.PosDest.y = resposta->msg_content.respostaNovasCoordenadas.y;
    } else {
        tcout << t("Esse Aeroporto não existe") << std::endl;
    }

    //Confirmar id e atualizar coords

}

DWORD WINAPI ThreadVoa(LPVOID param) {
    AviaoInstance &aviao = *(AviaoInstance *) param;
    int cond, newX, newY;
    Mensagem_Control mc;
    mc.type = alterar_coords;
    mc.id_aviao = aviao.aviao.IDAv;
    do {
        for (int i = 0; i < aviao.aviao.velocidade; i++) {
            cond = aviao.move(aviao.aviao.PosA.x, aviao.aviao.PosA.y, aviao.aviao.PosDest.x, aviao.aviao.PosDest.y,
                              &newX, &newY);
            mc.mensagem.pedidoConfirmarMovimento.x = newX;
            mc.mensagem.pedidoConfirmarMovimento.y = newY;
            std::unique_ptr<Mensagem_Aviao> resposta = aviao.sendMessage(true, mc);
            //recebe msg
            if (resposta->resposta_type == lol_ok) {
                aviao.aviao.PosA.x = newX;
                aviao.aviao.PosA.y = newY;
            } else {
                tcout << t("não houve movimento, ou havia algo a estrovar ou a piloto adormeceu") << std::endl;
            }

            tcout << t("X: ") << aviao.aviao.PosA.x << t("\tY: ") << aviao.aviao.PosA.y << t("\t\t X: ")
                  << aviao.aviao.PosDest.x << t("\tY: ") << aviao.aviao.PosDest.y << std::endl;


            if (cond == 0) {
                break;
            }
        }
        Sleep(1000);
    } while (cond != 0);

    return 1;

}

void Menu::inicia_voo() {

    if (aviaoInstance.aviao.PosA.isEqual(aviaoInstance.aviao.PosDest)) {
        tcout << t("Não é muito interessante voar para onde já esta") << std::endl;
        return;
    }
    aviaoInstance.em_andamento = true;

    HANDLE Voa_Passarinho = CreateThread(nullptr, 0, ThreadVoa, &aviaoInstance, 0, nullptr);
    tcin.get();
}


void Menu::suicidio() {
    aviaoInstance.suicidio();
    //fechar tudo e mandar msg_content a avisar
}
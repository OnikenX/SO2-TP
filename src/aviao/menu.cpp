#include "menu.hpp"
#include <algorithm>

Menu::Menu(AviaoInstance &aviao) : aviaoInstance(aviao), counter_aeroporto(0) {}


void Menu::run() {
    TCHAR op[25];
    //bool em_voo=false;
    aviaoInstance.em_andamento = false;
    bool exit = false;
    do {
        //O Nuno Esteve aqui as 5 da manhã!!!
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
                WaitForSingleObject(this->aviaoInstance.sharedComs->mutex_em_andamento, INFINITE);
                if (aviaoInstance.em_andamento)
                    exit = true;
                ReleaseMutex(this->aviaoInstance.sharedComs->mutex_em_andamento);
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
    mc.mensagem.pedidoConfirmarNovoAviao.id_aeroporto = idAero;
    std::unique_ptr<Mensagem_Aviao> resposta = aviaoInstance.sendMessage(true, mc);
    if (resposta->resposta_type == lol_ok) {
        aviaoInstance.aviao.PosDest.x = resposta->msg_content.respostaNovasCoordenadas.x;
        aviaoInstance.aviao.PosDest.y = resposta->msg_content.respostaNovasCoordenadas.y;
    } else if (resposta->resposta_type == MAX_Atingido) {
        tcout << t("O Maximo de Aviões foi atingido, e não temos mais copões para MALLOCS, logo azar") << std::endl;
    } else if (resposta->resposta_type == Porta_Fechada) {
        tcout << t("Desculpe informar, mas neste Aeroporto trabalham funcionarios publicos") << std::endl;
    } else {
        tcout << t("Esse Aeroporto não existe, mas se quiseres eu posso ser o teu Aeroporto XD") << std::endl;
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
    WaitForSingleObject(aviao.sharedComs->mutex_em_andamento, INFINITE);
    aviao.em_andamento = false;
    ReleaseMutex(aviao.sharedComs->mutex_em_andamento);
    tcout << t("Chegou vivo ao seu destino, clique em qualquer botão para voltar ao menu inicial.") << std::endl;

    return 1;

}

void Menu::inicia_voo() {

    if (aviaoInstance.aviao.PosA.isEqual(aviaoInstance.aviao.PosDest)) {
        tcout << t("Não é muito interessante voar para onde já esta") << std::endl;
        return;
    }
    WaitForSingleObject(this->aviaoInstance.sharedComs->mutex_em_andamento, INFINITE);
    aviaoInstance.em_andamento = true;
    ReleaseMutex(this->aviaoInstance.sharedComs->mutex_em_andamento);
    tcout << t("A voar, clique em qualquer tecla para !viver.");
    CreateThread(nullptr, 0, ThreadVoa, &aviaoInstance, 0, nullptr);
    while(tcin.get()!=t(" ")[0]);
#ifdef _DEBUG
    tcout << t("[DEBUG]: O botao foi clicado.(se estiveres em voo, es um ganda nabo).")<< std::endl;
#endif

}


void Menu::suicidio() {
    aviaoInstance.suicidio();
    //fechar tudo e mandar msg_content a avisar
}
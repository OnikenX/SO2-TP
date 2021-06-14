#include "menu.hpp"
#include <algorithm>

Menu::Menu(AviaoInstance &aviao) : aviaoInstance(aviao), counter_aeroporto(0) {}


void Menu::run() {
    TCHAR op[25];
    //bool em_voo=false;
    aviaoInstance.em_andamento = false;
    tstring line_input;
    bool exit = false;
    do {
        //O Nuno Esteve aqui as 5 da manhã!!!
        tcout << t("\nO que deseja fazer?\n");
        tcout << t("*********************************") << std::endl;
        tcout << t("*********************************") << std::endl;
        tcout << t("**   1 - Novo Destino          **") << std::endl;
        tcout << t("**   2 - Iniciar Voo           **") << std::endl;
        tcout << t("**   3 - Embarcar Passageiros  **") << std::endl;
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
                std::getline(tcin, line_input);
                std::basic_stringstream<TCHAR> sstream(line_input);
                sstream >> input;
            } while (input <= 0);
        } catch (std::exception &e) {
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


            case 2: {
                inicia_voo();
                WaitForSingleObject(this->aviaoInstance.sharedComs->mutex_em_andamento, INFINITE);
                if (aviaoInstance.em_andamento)
                    exit = true;
                ReleaseMutex(this->aviaoInstance.sharedComs->mutex_em_andamento);
                break;

            }


            case 3: {
                embarcar_passageiros();
            }


            default:
                tcout << t("Essa opção quase que existe, não queres tentar algo que seja possivel?\n");
        }

    } while (!exit);
}

//

void Menu::novas_cords() {
    int idAero;
    tstring line_input;
    tcout << t("Insira o ID do Aeroporto distino:") << std::endl;
    std::getline(tcin, line_input);
    std::basic_stringstream<TCHAR> sstream(line_input);
    sstream >> idAero;
    Mensagem_Aviao_request mc;
    mc.type = Mensagem_aviao_request_types::novo_destino;
    mc.id_aviao = aviaoInstance.aviaoInfo.IDAv;
    mc.mensagem.info_aeroportos.id_aeroporto = idAero;
    std::unique_ptr<Mensagem_Aviao_response> resposta = aviaoInstance.sendMessage(true, mc);
    if (resposta->resposta_type == Mensagem_Aviao_response_type::lol_ok) {
        aviaoInstance.aviaoInfo.PosDest.x = resposta->msg_content.respostaNovasCoordenadas.x;
        aviaoInstance.aviaoInfo.PosDest.y = resposta->msg_content.respostaNovasCoordenadas.y;
    } else if (resposta->resposta_type == Mensagem_Aviao_response_type::MAX_Atingido) {
        tcout << t("O Maximo de Aviões foi atingido, e não temos mais copões para MALLOCS, logo azar") << std::endl;
    } else if (resposta->resposta_type == Mensagem_Aviao_response_type::Porta_Fechada) {
        tcout << t("Desculpe informar, mas neste Aeroporto trabalham funcionarios publicos") << std::endl;
    } else {
        tcout << t("Esse Aeroporto não existe, mas se quiseres eu posso ser o teu Aeroporto XD") << std::endl;
    }

    //Confirmar id e atualizar coords

}

DWORD WINAPI ThreadVoa(LPVOID param) {
    AviaoInstance &aviao = *(AviaoInstance *) param;
    int cond, newX, newY;
    Mensagem_Aviao_request mc{};
    mc.type = Mensagem_aviao_request_types::alterar_coords;
    mc.id_aviao = aviao.aviaoInfo.IDAv;
    do {
        for (int i = 0; i < aviao.aviaoInfo.velocidade; i++) {
            cond = aviao.move(aviao.aviaoInfo.PosA.x, aviao.aviaoInfo.PosA.y, aviao.aviaoInfo.PosDest.x,
                              aviao.aviaoInfo.PosDest.y,
                              &newX, &newY);
            mc.mensagem.coordenadas_movimento.x = newX;
            mc.mensagem.coordenadas_movimento.y = newY;
            std::unique_ptr<Mensagem_Aviao_response> resposta = aviao.sendMessage(true, mc);
            //recebe msg
            if (resposta->resposta_type == Mensagem_Aviao_response_type::lol_ok) {
                aviao.aviaoInfo.PosA.x = newX;
                aviao.aviaoInfo.PosA.y = newY;
            } else {
                tcout << t("não houve movimento, ou havia algo a estrovar ou a piloto adormeceu") << std::endl;
            }
            tcout << t("X: ") << aviao.aviaoInfo.PosA.x << t("\tY: ") << aviao.aviaoInfo.PosA.y << t("\t\t X: ")
                  << aviao.aviaoInfo.PosDest.x << t("\tY: ") << aviao.aviaoInfo.PosDest.y << std::endl;
            if (cond == 0) {
                break;
            }
        }
        Sleep(1000);
    } while (cond != 0);
    WaitForSingleObject(aviao.sharedComs->mutex_em_andamento, INFINITE);
    aviao.em_andamento = false;
    ReleaseMutex(aviao.sharedComs->mutex_em_andamento);
    tcout << t("Chegou vivo ao seu destino, espaço e enter para voltar.") << std::endl;

    return 1;
}

void Menu::inicia_voo() {

    if (aviaoInstance.aviaoInfo.PosA.isEqual(aviaoInstance.aviaoInfo.PosDest)) {
        tcout << t("Não é muito interessante voar para onde já esta") << std::endl;
        return;
    }
    WaitForSingleObject(this->aviaoInstance.sharedComs->mutex_em_andamento, INFINITE);
    aviaoInstance.em_andamento = true;
    ReleaseMutex(this->aviaoInstance.sharedComs->mutex_em_andamento);
    tcout << t("A voar, digite espaço e enter para !viver.");
    CreateThread(nullptr, 0, ThreadVoa, &aviaoInstance, 0, nullptr);
    while (tcin.get() != t(" ")[0]);
#ifdef _DEBUG
    tcout << t("[DEBUG]: O botao foi clicado.(se estiveres em voo, es um ganda nabo).") << std::endl;
#endif

}


void Menu::suicidio() {
    aviaoInstance.suicidio();
    //fechar tudo e mandar msg_content a avisar
}

void Menu::embarcar_passageiros() const {
    Mensagem_Aviao_request aviaoRequest{};
    aviaoRequest.type = Mensagem_aviao_request_types::embarcacao;
    aviaoRequest.mensagem.info_aeroportos.aviaoInfo = aviaoInstance.aviaoInfo;
    aviaoRequest.id_aviao = aviaoInstance.aviaoInfo.IDAv;
    auto received = aviaoInstance.sendMessage(true, aviaoRequest);
    received->msg_content.passageiros_embarcados = aviaoInstance.embarcados;
}

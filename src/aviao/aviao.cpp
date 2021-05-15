#include "aviao.hpp"

int _tmain(int argc, TCHAR *argv[]){
//    if(argc!=4){
//        return ERRO_ARGUMENTOS;
//    }
//
//
//    auto a = std::make_unique<Aviao>();
//    a->CapMax = (int)argv[1];
//    a->velocidade = (int)argv[2];
//    a->IDAv = (int)argv[3];
    int posicao_x, posicao_y;
    move(1, 1, 100, 100, &posicao_x, &posicao_y);
    tcout << t("values: 1 1 100 100") << posicao_x << posicao_y << std::endl;
return 0;
}


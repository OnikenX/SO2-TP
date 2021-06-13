//
// Created by OnikenX on 5/15/2021.
//

#include "aviao.hpp"

int _tmain(int argc, TCHAR *argv[]) {

#ifdef UNICODE
#ifdef UTF8_UNICODE
    _setmode(_fileno(stdin), _O_U8TEXT);
        _setmode(_fileno(stdout), _O_U8TEXT);
        _setmode(_fileno(stderr), _O_U8TEXT);
#else
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
#endif

    if (argc != 4) {
        tcerr << t("ERRO: Por favor insira argumentos da seginte maneira:\n") <<
              t("\tSO2-TP-aviaoInfo.exe  <capacidade maxima> <velocidade> <ID do aeroporto>\n");
        return ERRO_ARGUMENTOS;
    }

    AviaoInfo a{};
    a.CapMax = _tstoi(argv[1]);
    a.velocidade = _tstoi(argv[2]);
    a.IDAv = _tstol(argv[3]);

    if (a.velocidade <= 0 || a.CapMax <= 0 || a.IDAv <= 0) {
        tcerr <<
              t("ERRO: A velocidade ou a capacidade maxima ou o Id do aeroporto não são numeros maiores que 0.")
              << std::endl;
        return ERRO_ARGUMENTOS;
    }

#ifdef _DEBUG
    tcout << t("[DEBUG]: Argumentos dados: ") <<
          t("\n\tcapacidade maxima: ") << a.CapMax <<
          t("\n\tvelocidade: ") << a.velocidade <<
          t("\n\tId do aeoroporto: ") << a.IDAv << std::endl;
#endif

    auto aviao = AviaoInstance::create(a);
    if (!aviao) {
        return 1;
    }

#ifdef _DEBUG
    tcout << t("[DEBUG]: Inicializado com sucesso!!") << std::endl;
#endif
    aviao->run();

    return 0;
}


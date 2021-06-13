//
// Created by OnikenX on 6/12/2021.
//

#include <shared_control_passageiro.hpp>
#include "passageiro.hpp"


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

    if (!(argc == 3 || argc == 4)) {
        tcerr << t("ERRO: Por favor insira argumentos da seginte maneira:\n") <<
              t("\tSO2-TP-passageiro.exe <ID do aeroporto de origem> <ID do aeroporto de destino> [tempo(segundos) para embarcar]\n");
        return ERRO_ARGUMENTOS;
    }
    errno = 0;
    PassageiroInfo p{};
    TCHAR *p_end;
    if (!ler_numero(argv[1], p.id_aeroporto_origem))
        return 2;
    if (!ler_numero(argv[2], p.id_aeroporto_destino))
        return 2;
    if (argc == 4) {
        if (!ler_numero(argv[3], p.tempo_para_embarcar))
            return 2;
    } else {
        p.tempo_para_embarcar = 0;
    }


    if (p.id_aeroporto_origem <= 0 || p.id_aeroporto_destino <= 0 || p.tempo_para_embarcar < 0) {
        tcerr <<
              t("ERRO: Os ids ou o tempo tem valores inferiores a 0.")
              << std::endl;
        return ERRO_ARGUMENTOS;
    }

#ifdef _DEBUG
    tcout << t("[DEBUG]: Argumentos dados: ") <<
          t("\n\tID do aeroporto de origem: ") << p.id_aeroporto_origem <<
          t("\n\tID do aeroporto de destino: ") << p.id_aeroporto_destino <<
          t("\n\ttempo_para_embarcar : ") << p.tempo_para_embarcar << std::endl;
#endif

    auto passageiro = PassageiroInstance::create(p);
    if (!passageiro) {
        return 1;
    }

    passageiro.value().run();
    return 0;
}


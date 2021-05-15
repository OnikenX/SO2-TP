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

    TCHAR aeroporto_do_aviao[MAX_LENGTH_NAME_AEROPORTO];

    if (argc != 4) {
        tcerr << t("ERRO: Por favor insira argumentos da seginte maneira:\n") <<
              t("\tSO2-TP-aviao.exe <capacidade maxima> <velocidade> <nome do aeroporto>\n");
        return ERRO_ARGUMENTOS;
    }

    if (_tcslen(argv[3]) > MAX_LENGTH_NAME_AEROPORTO) {
        tcerr
                << t("ERRO: O tamanho do nome do aeromorto é demasiado grande. Verifique se é esse mesmo o nome do aeroporto.\n");
        return ERRO_ARGUMENTOS;
    }

    int cap_max = _tstoi(argv[1]);
    int velocidade = _tstoi(argv[2]);
    if (cap_max == 0 || velocidade == 0) {
        tcerr << t("ERRO: A velocidade ou a capacidade maxima não são numeros maiores que 0.\n");
        return ERRO_ARGUMENTOS;
    }
//    _tccpy(aeroporto_do_aviao, argv[3]);

#ifdef _DEBUG
    tcout << t("Argumentos dados: ") <<
          t("\n\tcapacidade maxima: ") << cap_max <<
          t("\n\tvelocidade: ") << velocidade <<
          t("\n\tnome do aeoroporto: ") << argv[3] << std::endl;
#endif


    auto optional_aviao = AviaoInstance::create(argv[3], velocidade, cap_max);
    if (!optional_aviao.has_value()) {
        return 1;
    }
    std::unique_ptr<AviaoInstance> aviao = std::move(optional_aviao.value());

#ifdef _DEBUG
    tcout << t("fim com sucesso!!") << std::endl;
#endif
    return 0;
}

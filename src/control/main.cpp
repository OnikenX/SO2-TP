#include "control.hpp"

int _tmain() {
    try {
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
        auto control = Control::create();
        if (!control){
            tcout << t("Control não pode ser criado.") << std::endl;
            return ERRO_CONTROL_NAO_EXISTE;
        }
        int return_control = control->run();
        if (return_control != 0) {
            tcout << t("Erro ") << return_control << t(" a correr o control.") << std::endl;
        }
        return return_control;
    } catch (std::exception e) {
        std::cout << e.what();
        return 123;
    }
    return 124;
}
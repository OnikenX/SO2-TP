#include "control.hpp"

int _tmain() {

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    auto control_create = Control::create();
    if (!control_create.has_value())
        tcout << t("Control não pode ser criado.") << std::endl;
    auto control = std::move(control_create.value());

    int return_control = control->run();
    if (return_control != 0) {
        tcout << t("Erro ") << return_control << t(" a correr o control.") << std::endl;
    }
    return return_control;

}
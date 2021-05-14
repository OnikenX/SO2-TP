#include "control.hpp"
#include "menu.hpp"

int Control::run()
{
    auto menu = std::make_unique<Menu>(*this);
    menu->run();
	return 0;
}

Control::~Control() {
    UnmapViewOfFile(view_of_file_pointer);
    CloseHandle(shared_memory_handle);
    this->notifica_tudo();
}

void Control::finalizar() {
    //faz cenas para dizer que vai fechar
    tcout << t("A espera de todos para mimir...");
}

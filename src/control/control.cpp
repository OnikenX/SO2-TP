#include "control.hpp"

int Control::run()
{
	return 0;
}

Control::~Control() {
    UnmapViewOfFile(view_of_file_pointer);
    CloseHandle(shared_memory_handle);
    this->notifica_tudo();
}

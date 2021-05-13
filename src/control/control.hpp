#pragma once

#include "./../utils.h"

class Control
{
public:
	const unsigned char MAX_AVIOES;

	// caso exista problemas a criar o control o optional não ira retornar um control
	static std::optional<Control> create(unsigned char MAX_AVIOES = 10);

	// main function do control
	int run();

private:
	Control(unsigned char max_avioes, HANDLE mutex_unico);
	// startup functions
	int setup_do_registry();
	Wrappers::Handle<HANDLE> mutex_unico;

	// verfica se o control já existe,
	// se não existir returna o handle para o mutex deste
	static std::optional<HANDLE> verifica_se_o_control_ja_existe();
};

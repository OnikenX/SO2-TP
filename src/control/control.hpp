#pragma once
#include "./../utils.h"
#include <vector>

class Control
{
public:
	const unsigned char MAX_AVIOES;
	const unsigned char MAX_AEROPORTOS;
	// caso exista problemas a criar o control o optional não ira retornar um control
	static std::optional<Control> create(unsigned char MAX_AVIOES = 10, unsigned char MAX_AEROPORTOS = 10);

	// main function do control
	int run();

private:
	Control(unsigned char max_avioes, HANDLE mutex_unico);
	// startup functions
	static int setup_do_registry();
	Wrappers::Handle<HANDLE> mutex_unico;
	Dados d;
	std::vector<Aeroporto> aeroportos;
	std::vector<Aviao> avioes;
	bool aceita_avioes;
	
	char Mapa[1000][1000];

	// verfica se o control já existe,
	// se não existir returna o handle para o mutex deste
	static std::optional<HANDLE> get_map_file_handle();

	friend class Menu;
};

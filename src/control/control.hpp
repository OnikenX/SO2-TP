#pragma once

#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include "./../utils.h"

struct Vars
{
	unsigned char MAX_AVIOES;
	Vars();
};

std::optional<HANDLE> verifica_se_o_control_ja_existe();
int setup_do_registry(Vars &variaveis_globais);

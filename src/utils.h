
#pragma once

//standard c++
#include <memory>
#include <sstream>
#include <iostream>
#include <sstream>
#include <optional>//RUST Team

//windows and tchar
#include <windows.h>
#include <tchar.h>

//set mode for the terminal
#include <io.h>
#include <fcntl.h>

//typedef for tchar to std types
#ifdef UNICODE
#define tcout std::wcout
#define tcerr std::wcerr
#define tcin std::wcin
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcerr std::cerr
#define tcin std::cin
#define tstring std::string
#define tstringstream std::stringstream
#endif

//defines

// shortener for the TEXT macro
#define t(quote) TEXT(quote)
//shared memory name
#define SHARED_MEMORY_NAME t("Global\\SO2_TP_SHARED_MEMORY")
//erros comuns

#define ERRO_ARGUMENTOS 1
#define ERRO_CONTROL_EXISTE 2
#define ERRO_CONTROL_NAO_EXISTE 3

//structs partilhadas

struct Cords
{
	int x;
    int y;
};

struct Aviao
{
    int IDAv;
    int CapMax;
    int velocidade;
	Cords PosA;
    Cords PosDest;
};

struct Aeroporto{
    Cords pos;
    int IDAero;
};

class Mensagem_Aviao_Control{

};

class Mensagem_Control_Aviao{

};

struct shared_memory_map
{
	bool se_pode_criar_mais_avioes;
	//buffer circular das mensagens que vao do aviao para o control
	Mensagem_Aviao_Control mensagens_aviao_controler[100];
	//buffer circular das mensagens que vao do control para o aviao
	Mensagem_Control_Aviao mensagens_controler_aviao[100];
};



struct DadosThreads{
    shared_memory_map* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    int terminar; // 1 para sair, 0 em caso contr�rio
    int id;
};
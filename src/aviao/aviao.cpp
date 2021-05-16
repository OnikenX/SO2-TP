
//#include "./../../SO2_TP_DLL/SO2_TP_DLL_2021.h"

#include "aviao.hpp"

//novo destino
//iniciar voo
//suicidio


void receiveMessage(){

}

bool confirmMessage(){

}

DWORD WINAPI Limbo(LPVOID param) {
    tcout << t("waiting ...") << std::endl;
    HANDLE evento = CreateEvent(NULL, TRUE, FALSE, EVENT_KILLER);

    WaitForSingleObject(evento, INFINITE);
#ifdef _DEBUG
    tcout << t("BOOM !!X\\ RIP In Piece );") << std::endl;
#endif
    return 1;
}

bool sendMessage(){

}

int AviaoInstance::run(){
    auto Wait_to_Die = CreateThread(NULL, 0, Limbo, NULL, 0, NULL)

    if (em_andamento){
        
    }
    WaitForMultiple
    return 0;
}




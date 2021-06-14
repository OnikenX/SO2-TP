//
// Created by OnikenX on 5/14/2021.
//


//
//DWORD WINAPI ThreadName(LPVOID param) {
//    tcout << t("waiting ...") << std::endl;
//    HANDLE evento = CreateEvent(NULL, TRUE, FALSE, t("evento_test"));
//
//    WaitForSingleObject(evento, INFINITE);
//    tcout << t("waited!") << std::endl;
//    return 1;
//}
//
//void testevent(){
//    SetLastError(ERROR_SUCCESS);
//    HANDLE evento = CreateEvent(NULL, TRUE, FALSE, t("evento_test"));
//    if (!evento) {
//        tcout << t("fudeu!");
//        return -1;
//    }
//    HANDLE processes[10];
//    for (int i = 0; i < 10; i++)
//        processes[i] = CreateThread(NULL, 0, ThreadName, NULL, 0, NULL);
//    Sleep(2000);
//    SetEvent(evento);
//    WaitForMultipleObjects(10,processes, true, INFINITE);
//}
#include <memory>
#include <string>
#include <iostream>
#include <WINDOWS.h>
#include <utils.hpp>
#include <list>


struct tester{
    bool moved = false;
    tester(){tcout <<t("contructed") << std::endl;}
    tester(tester& other){
        tcout << t("reference constructor") << std::endl;;
    }

    tester(tester const &other){
        tcout << t("copy constructor") << std::endl;;
    }

    tester(tester&& other) noexcept {
        tcout << t("moved") << std::endl;;
        moved = true;
    }
    ~tester(){
        if(!moved){
            tcout << t("deleted stuff") << std::endl;;
        }else
            tcout << t("this was moved correctly") << std::endl;;
    };
};

int main() {
    tester inst;
    std::list<tester> insts1;
    std::list<tester> insts2;
    insts1.emplace_back(std::move(inst));
    insts2.splice(insts2.begin(), insts1, insts1.begin());
    tcin.get();
}
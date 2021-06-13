//
// Created by OnikenX on 6/12/2021.
//

#include "passageiro.hpp"

PassageiroInstance::PassageiroInstance(HANDLE waitToDie, const PassageiroInfo &passageiroInfo)
        : Wait_to_Die(waitToDie), passageiroInfo(passageiroInfo), moved(false) {}

std::optional<PassageiroInstance> PassageiroInstance::create(const PassageiroInfo &passageiroInfo) {
    return std::move(PassageiroInstance(nullptr, passageiroInfo));
}

PassageiroInstance::~PassageiroInstance() {
    if (!moved) {
        CloseHandle(Wait_to_Die);
    }
}

PassageiroInstance::PassageiroInstance(PassageiroInstance &&passageiroInstance) noexcept
        : Wait_to_Die(passageiroInstance.Wait_to_Die), moved(false), passageiroInfo(passageiroInstance.passageiroInfo) {
    passageiroInstance.moved = true;
    passageiroInstance.Wait_to_Die = nullptr;
}

DWORD WINAPI PipeClient(LPVOID param);
void PassageiroInstance::run() {
    PipeClient(this);
}

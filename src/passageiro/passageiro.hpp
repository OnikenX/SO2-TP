//
// Created by OnikenX on 6/12/2021.
//
#pragma once

#include <utils.hpp>
#include <shared_control_passageiro.hpp>

////strutura dos handles de partinha de informação com o control(exclusivo do passageiro)
//struct PassageiroComunicationSharedObjects_passageiro {
//    HANDLE shared_mutex;
//    HANDLE shared_pipe;
//};


struct PassageiroInstance {
    HANDLE Wait_to_Die;//HANDLE for the thread to kill itself
    const PassageiroInfo passageiroInfo;
    PassageiroInstance(const PassageiroInstance &) = delete; // non construction-copyable
    PassageiroInstance &operator=(const PassageiroInstance &) = delete; // non copyable
    PassageiroInstance() = delete;
    PassageiroInstance(HANDLE waitToDie, const PassageiroInfo &passageiroInfo);
    ~PassageiroInstance();
    PassageiroInstance(PassageiroInstance&& passageiroInstance) noexcept ;

    void run();
    static std::optional<PassageiroInstance> create(const PassageiroInfo & passageiroInfo);
private:
    bool moved;
};


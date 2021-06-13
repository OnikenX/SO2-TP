//
// Created by OnikenX on 6/12/2021.
//

#include "control.hpp"
#include <memory>

DWORD WINAPI InstanceThread(LPVOID);

bool verifica_avioes(Mensagem_Passageiro_request &passageiroRequest,
                     Mensagem_Passageiro_response &passageiroResponse, Control &control) {
    //incrementa se quando se encontra um destino, caso se encontra ambos os destinos(encontrou_id = 2)
    bool encontrou_destino = false, encontrou_origem = false;
    passageiroResponse.resposta_type = Mensagem_passageiro_response_type::aeroporto_nao_existe;
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    for (const auto &aviao : control.avioes) {
        if (aviao.IDAv == passageiroRequest.msg_content.passageiroInfo.id_aeroporto_destino)
            encontrou_destino = true;
        if (aviao.IDAv == passageiroRequest.msg_content.passageiroInfo.id_aeroporto_origem)
            encontrou_origem = true;
        if (encontrou_destino && encontrou_origem) {
            passageiroResponse.resposta_type = Mensagem_passageiro_response_type::lol_ok;
            return true;
        }
    }
    return false;
}

DWORD WINAPI PipeServer(LPVOID param) {
    Control &control = *(Control *) param;

    bool fConnected = false;
    DWORD dwThreadId = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = nullptr;
    const TCHAR *lpszPipename = NAMEDPIPE_NAME;

// The main loop creates an instance of the named pipe and
// then waits for a client to connect to it. When the client
// connects, a thread is created to handle communications
// with that client, and this loop is free to wait for the
// next client connect request. It is an infinite loop.

    for (;;) {
        _tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
        hPipe = CreateNamedPipe(
                lpszPipename,             // pipe name
                PIPE_ACCESS_DUPLEX,       // read/write access
                PIPE_TYPE_MESSAGE |       // message request_type pipe
                PIPE_READMODE_MESSAGE |   // message-read mode
                PIPE_WAIT,                // blocking mode
                PIPE_UNLIMITED_INSTANCES, // max. instances
                sizeof(Mensagem_Passageiro_response),                  // output buffer size
                sizeof(Mensagem_Passageiro_request),                  // input buffer size
                0,                        // client time-out
                nullptr);                 // default security attribute

        if (hPipe == INVALID_HANDLE_VALUE) {
            _tprintf(TEXT("CreateNamedPipe failed, GLE=%lu.\n"), GetLastError());
            return -1;
        }

        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

        fConnected = ConnectNamedPipe(hPipe, nullptr) ?
                     TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected) {
            printf("Client connected, creating a processing thread.\n");

            auto control_handle = new std::pair<Control &, HANDLE>(control, hPipe);
            // Create a thread for this client.
            hThread = CreateThread(
                    nullptr,              // no security attribute
                    0,                 // default stack size
                    InstanceThread,    // thread proc
                    (LPVOID) control_handle,    // thread parameter
                    0,                 // not suspended
                    &dwThreadId);      // returns thread ID

            if (hThread == nullptr) {
                delete control_handle;
                _tprintf(TEXT("CreateThread failed, GLE=%lu.\n"), GetLastError());
                return -1;
            } else CloseHandle(hThread);
        } else
            // The client could not connect, so close the pipe.
            CloseHandle(hPipe);
    }

    return 0;
}


void terminate_pipe_handles(HANDLE hPipe) {
    // Flush the pipe to allow the client to read the pipe's contents
    // before disconnecting. Then disconnect the pipe, and close the
    // handle to this pipe instance.
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
{

    if (lpvParam == nullptr) {
        tcerr << t("Param is null in InstanceThread\n");
    }
    const auto pair_param = std::unique_ptr<std::pair<Control &, HANDLE>>((std::pair<Control &, HANDLE> *) lpvParam);
    if (pair_param->second == nullptr) {
        tcerr << t("Pipe not available.\n");
        return -1;
    }

    Mensagem_Passageiro_request passageiroRequest{};
    Mensagem_Passageiro_response passageiroResponse{};
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe = pair_param->second;




    // Print verbose messages. In production code, this should be for debugging only.
    printf("InstanceThread created, receiving and processing messages.\n");

// The thread's parameter is a handle to a pipe object instance.

// Loop until done reading
    // Read client requests from the pipe. This simplistic code only allows messages
    // up to BUFSIZE characters in length.
    fSuccess = ReadFile(
            hPipe,        // handle to pipe
            &passageiroRequest,    // buffer to receive data
            sizeof(Mensagem_Passageiro_request), // size of buffer
            &cbBytesRead, // number of bytes read
            nullptr);        // not overlapped I/O

    if (!fSuccess || cbBytesRead == 0) {
        if (GetLastError() == ERROR_BROKEN_PIPE) {
            _tprintf(TEXT("InstanceThread: client disconnected.\n"));
        } else {
            DebugBreak();
            _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%lu.\n"), GetLastError());
        }
        terminate_pipe_handles(hPipe);
        return 0;
    }
    // Process the incoming message.
    verifica_avioes(passageiroRequest, passageiroResponse, pair_param->first);
    // Write the reply to the pipe.
    fSuccess = WriteFile(
            hPipe,        // handle to pipe
            &passageiroResponse,     // buffer to write from
            sizeof(Mensagem_Passageiro_response), // number of bytes to write
            &cbWritten,   // number of bytes written
            nullptr);        // not overlapped I/O

    if (!fSuccess || cbWritten == 0) {
        _tprintf(TEXT("InstanceThread WriteFile failed, GLE=%lu.\n"), GetLastError());
    }

    if (passageiroResponse.resposta_type != Mensagem_passageiro_response_type::lol_ok) {
        terminate_pipe_handles(hPipe);
    } else {
        auto guard = CriticalSectionGuard(pair_param->first.critical_section_interno);
        pair_param->first.passageiros.emplace_back(passageiroRequest.msg_content.passageiroInfo, hPipe);
    }

    printf("InstanceThread exiting.\n");
    return 1;
}


//envia uma mensagem ao passageiro,
// caso de erro(returnar false) deve se ignorar como se tivesse dado timeout ou saido
//  e remover o membro se assim for o caso
bool SendMessagePipe(HANDLE hPipe, Mensagem_Passageiro_response &passageiroResponse) {
    DWORD cbWritten;
    bool fSuccess = WriteFile(
            hPipe,        // handle to pipe
            &passageiroResponse,     // buffer to write from
            sizeof(Mensagem_Passageiro_response), // number of bytes to write
            &cbWritten,   // number of bytes written
            nullptr);        // not overlapped I/O
    auto lastError = GetLastError();
    if (!fSuccess || cbWritten == 0 || lastError == ERROR_BROKEN_PIPE) {
        _tprintf(TEXT("SendMessagePipe() => WriteFile() failed, GLE=%d.\n"), GetLastError());
        return false;
    }
    return true;
}
//
// Created by OnikenX on 6/13/2021.
//

#include "passageiro.hpp"

DWORD WINAPI InputWaiter(LPVOID param);

int8_t confirm_ids(HANDLE hPipe, PassageiroInstance &passageiro);

DWORD WINAPI GetMessages(LPVOID hPipe);

DWORD WINAPI PipeClient(LPVOID param) {
    auto &passageiro = *(PassageiroInstance *) param;
    HANDLE hPipe;
    const TCHAR *lpszPipename = NAMEDPIPE_NAME;
    bool fSuccess = false;
    DWORD cbRead, cbWritten, dwMode;


// Try to open a named pipe; wait for it, if necessary.
    while (true) {
        hPipe = CreateFile(
                lpszPipename,   // pipe name
                GENERIC_READ |  // read and write access
                GENERIC_WRITE,
                0,              // no sharing
                nullptr,        // default security attributes
                OPEN_EXISTING,  // opens existing pipe
                0,              // default attributes
                nullptr);       // no template file

// Break if the pipe handle is valid.

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

// Exit if an error other than ERROR_PIPE_BUSY occurs.

        if (GetLastError() != ERROR_PIPE_BUSY) {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
            tcerr << t("Control provavelmente não existe.");
            return -1;
        }

// All pipe instances are busy, so wait for 20 seconds.
        if (!WaitNamedPipe(lpszPipename, 20000)) {
            printf("Could not open pipe: 20 second wait timed out.");
            return -1;
        }
    }

// The pipe connected; change to message-read mode.

    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(
            hPipe,    // pipe handle
            &dwMode,  // new pipe mode
            nullptr,     // don't set maximum bytes
            nullptr);    // don't set maximum time
    if (!fSuccess) {
        _tprintf(TEXT("[ERROR]: SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
        return -1;
    }


    //verifies if the ids are right
    if (confirm_ids(hPipe, passageiro) != 0) {
        return -1;
    }
#ifdef _DEBUG
    tcerr<< t("[DEBUG]: Connected with sucess!!\n");
#endif
    HANDLE threads[] = {
            CreateThread(nullptr, 0, InputWaiter, nullptr, 0, nullptr),
            CreateThread(nullptr, 0, Limbo, nullptr, 0, nullptr),
            CreateThread(nullptr, 0, GetMessages, nullptr, 0, nullptr)
    };
    //if any of the threads fails, they are terminated
    WaitForMultipleObjects(sizeof(threads) / sizeof(HANDLE), threads, false, INFINITE);

    CloseHandle(hPipe);

    return 0;
}

int8_t confirm_ids(HANDLE hPipe, PassageiroInstance &passageiro) {
    DWORD cbWritten{}, cbRead{};
    Mensagem_Passageiro_request passageiroRequest{};
    Mensagem_Passageiro_response passageiroResponse{};
    passageiroRequest.request_type = Mensagem_passageiro_request_type::confirmacao;
    passageiroRequest.msg_content.passageiroInfo = passageiro.passageiroInfo;
    // Send a message to the pipe server to verify ids.
    bool fSuccess = WriteFile(
            hPipe,                  // pipe handle
            &passageiroRequest,             // message
            sizeof(Mensagem_Passageiro_request),              // message length
            &cbWritten,             // bytes written
            nullptr);                  // not overlapped

    if (!fSuccess) {
        _tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
        return -1;
    }

#ifdef _DEBUG
    tcerr << t("\n[DEBUG]: Message sent to server.\n");
#endif

    do {
        // Read from the pipe.
        fSuccess = ReadFile(
                hPipe,    // pipe handle
                &passageiroResponse,    // buffer to receive reply
                sizeof(passageiroResponse),  // size of buffer
                &cbRead,  // number of bytes read
                nullptr);    // not overlapped

        if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
            break;
    } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA


    if (!fSuccess) {
        tcerr << TEXT("[ERROR]: ReadFile from pipe failed. GLE=") << GetLastError() << std::endl;
        return -1;
    }

    if (passageiroResponse.resposta_type != Mensagem_passageiro_response_type::lol_ok) {
        if (passageiroResponse.resposta_type == Mensagem_passageiro_response_type::aeroporto_nao_existe) {
            tcerr << t("[ERROR]: Aeroporto não existe.\n");
        }
        tcerr << t("[ERROR]: Ligação sem sucesso.\n");
        return -1;
    }

    return 0;
}


//waits for sweet taste of death from the keyboard
DWORD WINAPI InputWaiter(LPVOID param) {
    bool exit = false;
    while (!exit) {
        tstring string_line{};
        std::getline(tcin, string_line);
        tstringstream stream(string_line);
        tstring command{};
        stream >> command;
        if (command == t("killme")) {
            tcout << t("Bye bye cruel world.\n");
            exit = true;
        } else if (command == t("help")) {
            tcout << t("Commands:")
                  << t("\n\thelp - printa isto.")
                  << t("\n\tkillme - termina o programa.")
                  << std::endl;
        } else {
            tcout << t("Commnado n reconhecido. espermenta `help` sem peliculas.\n");
        }
    }
    return 0;
}

DWORD WINAPI GetMessages(LPVOID lpparam) {
    HANDLE hPipe = lpparam;
    Mensagem_Passageiro_response passageiroResponse{};
    DWORD cbRead, cbWritten;
    bool fSuccess;
    while (true) {
        // Read client requests from the pipe. This simplistic code only allows messages
        // up to BUFSIZE characters in length.
        fSuccess = ReadFile(
                hPipe,        // handle to pipe
                &passageiroResponse,    // buffer to receive data
                sizeof(Mensagem_Passageiro_response), // size of buffer
                &cbRead, // number of bytes read
                nullptr);        // not overlapped I/O

        if (!fSuccess || cbRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                _tprintf(TEXT("InstanceThread: client disconnected.\n"));
            } else {
                _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
            }
            break;
        }
        // Process the incoming message.
        switch (passageiroResponse.resposta_type) {
            case Mensagem_passageiro_response_type::desembarcado: {
                tcout << t("Desembarcou.\n");
                return 0;
            }
            case Mensagem_passageiro_response_type::embarcado: {
                tcout << t("Embarcou.\n");
                break;
            }
            default:
                tcout << t("[ERROR]: Mensagem não possivel.\n");
                return -1;
        }
    }
    return 0;
}
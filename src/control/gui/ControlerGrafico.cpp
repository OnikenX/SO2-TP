// ControlerGrafico.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "../control.hpp"
#include <strsafe.h>

#define MAX_LOADSTRING 100

// Global Variables:
Menu *menu;
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

Aeroporto a;

HACCEL hAccelTable;
MSG msg;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);

BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    NovoAviao(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    ListaAeroportos(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    ListaPassageiros(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    ListaAvioes(HWND, UINT, WPARAM, LPARAM);




#ifdef _UNICODE
#pragma comment(linker, "/subsystem:windows /ENTRY:wWinMainCRTStartup")
#else
#pragma comment(linker, "/subsystem:windows /ENTRY:WinMainCRTStartup")
#endif

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR lpCmdLine,
                       _In_ int nCmdShow) {

    auto control = Control::create();
    if (!control) {
        tcout << t("Control não pode ser criado.") << std::endl;
        return ERRO_CONTROL_NAO_EXISTE;
    }

    control->menu.componentes_graphicos.hInstance = hInstance;
    control->menu.componentes_graphicos.hPrevInstance = hPrevInstance;
    control->menu.componentes_graphicos.lpCmdLine = lpCmdLine;
    control->menu.componentes_graphicos.nCmdShow = nCmdShow;
    int return_control = control->run();
    if (return_control != 0) {
        tcout << t("Erro ") << return_control << t(" a correr o control.") << std::endl;
    }

    return (int) msg.wParam;
    //hInst = hInstance;

}


void Menu::run() {
    menu = this;

    UNREFERENCED_PARAMETER(menu->componentes_graphicos.hPrevInstance);
    UNREFERENCED_PARAMETER(menu->componentes_graphicos.lpCmdLine);
    // Initialize global strings
    LoadString(menu->componentes_graphicos.hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(menu->componentes_graphicos.hInstance, IDC_CONTROLERGRAFICO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(menu->componentes_graphicos.hInstance);

    // Perform application initialization:
    if (!InitInstance(menu->componentes_graphicos.hInstance, menu->componentes_graphicos.nCmdShow)) {
        return;
    }

    hAccelTable = LoadAccelerators(menu->componentes_graphicos.hInstance, MAKEINTRESOURCE(IDC_CONTROLERGRAFICO));

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CONTROLERGRAFICO));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_CONTROLERGRAFICO);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    const TCHAR *msg_text = TEXT("O Nuno Esteve Aqui!!!");

    switch (message) {
        case WM_CREATE:
            SetTimer(hWnd, 1, 20, NULL);
            break;

        case WM_TIMER:
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case WM_COMMAND: {
            LONG_PTR userdata = GetWindowLongPtr(hWnd, GWLP_USERDATA);


            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_CRIARAEROPORTO:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_NovoAviao), hWnd, NovoAviao);
                    break;
                case IDM_Consulta_Aeroporto:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_LB_Aeroportos), hWnd, ListaAeroportos);
                    break;
                case IDM_Consulta_Aviao:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_LB_Avioes), hWnd, ListaAvioes);
                    break;
                case IDM_Consulta_Passageiros:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_LB_Passageiros), hWnd, ListaAvioes);
                    break;
                case IDM_NO_PLANES:
                    if (!menu->desativa_novos_avioes()) {
                        MessageBoxEx(nullptr, TEXT("DOORS CLOSED"), TEXT("O Nuno Esteve Aqui!!!"),
                            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
                    }
                    else {
                        MessageBoxEx(nullptr, TEXT("DOORS OPEN"), TEXT("O Nuno Esteve Aqui!!!"),
                            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
                    }
                     
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                    //case WM_PAINT:

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            TextOut(hdc, 500, 500, msg_text, _tcslen(msg_text));
            auto guard = CriticalSectionGuard(menu->control.critical_section_interno);
            
           // int i = 0;
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            for (auto& aviao : menu->control.avioes)
            {
                TextOut(hdc, aviao.PosA.x, aviao.PosA.y, t("A"), 1);
            }
            for (int i = 0; i < menu->control.aeroportos.size(); i++) {
                TextOut(hdc, menu->control.aeroportos[i].pos.x, menu->control.aeroportos[i].pos.y, menu->control.aeroportos[i].nome, 1);
            }
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            //  case WM_CREATE:

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


bool verificaMaxAeroportos(Menu *isto) {
    auto guard = CriticalSectionGuard(isto->control.critical_section_interno);
    if (isto->control.MAX_AEROPORTOS <= isto->counter_aeroporto) {
        MessageBoxEx(NULL, TEXT("Já foi atingido o limite de Aeroportos possiveis"), TEXT("ERRO!"),
                     MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
        //tcout << t("Já foi atingido o limite de Aeroportos possiveis") << std::endl;
        return false;
    }
    return true;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR) TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR) TRUE;
            }
            break;
    }
    return (INT_PTR) FALSE;
}

INT_PTR CALLBACK ListaAeroportos(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Add items to list.
        HWND hwndList = GetDlgItem(hDlg, IDC_ListAeroportos);
        for (int i = 0; i < menu->control.aeroportos.size(); i++)
        {
            int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0,
                (LPARAM)menu->control.aeroportos[i].nome);
            // Set the array index of the player as item data.
            // This enables us to retrieve the item from the array
            // even after the items are sorted by the list box.
            SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
        }
        // Set input focus to the list box.
        SetFocus(hwndList);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;

        case IDC_ListAeroportos:
        {
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                HWND hwndList = GetDlgItem(hDlg, IDC_ListAeroportos);

                // Get selected index.
                int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);

                // Get item data.
                int i = (int)SendMessage(hwndList, LB_GETITEMDATA, lbItem, 0);

                // Do something with the data from Roster[i]
                TCHAR buff[MAX_PATH];
                StringCbPrintf(buff, ARRAYSIZE(buff),
                    TEXT("Nome: %s\nLocal: X-> %d\tY-> %d"),
                    menu->control.aeroportos[i].nome, menu->control.aeroportos[i].pos.x,
                    menu->control.aeroportos[i].pos.y);

                SetDlgItemText(hDlg, IDC_STATIC, buff);
                return TRUE;
            }
            }
        }
        return TRUE;
        }
    }
    return FALSE;
}

INT_PTR CALLBACK ListaPassageiros(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Add items to list.
        HWND hwndList = GetDlgItem(hDlg, IDC_ListPassageiros);
        int i = 0;
        auto guard = CriticalSectionGuard(menu->control.critical_section_interno);
        for (auto& pass : menu->control.passageiros)
        {
            tstringstream stream;
            stream << t("pass") << i + 1;
            tstring s = stream.str();
            int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0,
                (LPARAM)s.c_str());
            // Set the array index of the player as item data.
            // This enables us to retrieve the item from the array
            // even after the items are sorted by the list box.
            SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
            i++;
        }
        // Set input focus to the list box.
        SetFocus(hwndList);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;

        case IDC_ListPassageiros:
        {
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                HWND hwndList = GetDlgItem(hDlg, IDC_ListPassageiros);

                // Get selected index.
                int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);

                // Get item data.
                int i = (int)SendMessage(hwndList, LB_GETITEMDATA, lbItem, 0);
                int j = 0;
                auto guard = CriticalSectionGuard(menu->control.critical_section_interno);
                for (auto& pass : menu->control.passageiros) {
                    if (i == j) {
                        //TCHAR buff[MAX_PATH];
                        tstringstream stream;
                        stream << t("Tempo para Embarcar-> ") << pass.info.tempo_para_embarcar << t("\nAeroporto Atual->") <<
                            pass.info.id_aeroporto_origem << t("\nAeroporto Destino->") << pass.info.id_aeroporto_destino;
                        tstring buff = stream.str();

                        SetDlgItemText(hDlg, IDC_STATICpas, buff.c_str());
                        return TRUE;
                    }
                    j++;
                }
                // Do something with the data from Roster[i]

            }
            }
        }
        return TRUE;
        }
    }
    return FALSE;
}

INT_PTR CALLBACK ListaAvioes(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Add items to list.
        HWND hwndList = GetDlgItem(hDlg, IDC_ListAvioes);
        int i=0;
        auto guard = CriticalSectionGuard(menu->control.critical_section_interno);
        for (auto& aviao : menu->control.avioes)
        {
            tstringstream stream;
            stream << aviao.IDAv;
            tstring s = stream.str();
            int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0,
                (LPARAM)s.c_str());
            // Set the array index of the player as item data.
            // This enables us to retrieve the item from the array
            // even after the items are sorted by the list box.
            SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
            i++;
        }
        // Set input focus to the list box.
        SetFocus(hwndList);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;

        case IDC_ListAvioes:
        {
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                HWND hwndList = GetDlgItem(hDlg, IDC_ListAvioes);

                // Get selected index.
                int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);

                // Get item data.
                int i = (int)SendMessage(hwndList, LB_GETITEMDATA, lbItem, 0);
                int j = 0;
                auto guard = CriticalSectionGuard(menu->control.critical_section_interno);
                for (auto& aviao : menu->control.avioes) {
                    if (i == j) {
                        //TCHAR buff[MAX_PATH];
                        tstringstream stream;
                        stream << t("ID: ") << aviao.IDAv << t("\nVelocidade->") << aviao.velocidade <<
                            t("\tCapaxidadeMax->") << aviao.CapMax << t("\nPosição Atual:\n X-> ") << aviao.PosA.x << t("\tY-> ")
                                << aviao.PosA.y << t("\nPosição Destino:\n X-> ")<< aviao.PosDest.x<<t("\tY-> ") << aviao.PosDest.y;
                        tstring buff = stream.str();

                        SetDlgItemText(hDlg, IDC_STATICav, buff.c_str());
                        return TRUE;
                    }
                    j++;
                }
                // Do something with the data from Roster[i]

            }
            }
        }
        return TRUE;
        }
    }
    return FALSE;
}

INT_PTR CALLBACK NovoAviao(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR) TRUE;

        case WM_COMMAND:
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
                /*case IDC_NomeAero:
                        GetDlgItemText(hDlg, IDC_NomeAero, cords.nome,50);
                        MessageBox(hDlg, cords.nome, L"edit text", 0);
                    break;
              */
                case IDOK:
                    GetDlgItemText(hDlg, IDC_NomeAero, menu->componentes_graphicos.aeroporto.nome, 50);
                    menu->componentes_graphicos.aeroporto.pos.x = GetDlgItemInt(hDlg, IDC_XCords, FALSE, FALSE);
                    menu->componentes_graphicos.aeroporto.pos.y = GetDlgItemInt(hDlg, IDC_YCords, FALSE, FALSE);
                    if (!menu->cria_aeroporto()) {
                        MessageBoxEx(NULL, TEXT("Já existe um aeroporto nas redondezas, não me parece boa ideia construir um aqui"),
                            TEXT("ERRO!"), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);

                        EndDialog(hDlg, LOWORD(wParam));
                        return (INT_PTR)TRUE;
                    }

                    MessageBox(hDlg, menu->componentes_graphicos.aeroporto.nome, t("Dados Inseridos"), 0);
                    EndDialog(hDlg, LOWORD(wParam));
                    menu->componentes_graphicos.aeroporto.IDAero = ++menu->counter_aeroporto;
                    {
                        auto guard = CriticalSectionGuard(menu->control.critical_section_interno);
                        menu->control.aeroportos.insert(menu->control.aeroportos.end(), menu->componentes_graphicos.aeroporto);
                    }
                    return (INT_PTR) TRUE;
                    break;


                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                    break;
            }

            /* if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
             {
                 MessageBoxEx(NULL, TEXT("Miau"), TEXT("O Nuno Esteve Aqui!!!"),
                     MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
                 EndDialog(hDlg, LOWORD(wParam));
                 return (INT_PTR)TRUE;
             }*/
            break;
    }
    return (INT_PTR) FALSE;
}


bool Menu::cria_aeroporto() {

    if (!verificaMaxAeroportos(this)) {
        MessageBoxEx(NULL, TEXT("Maximo de Aeroportos exedidos..."), TEXT("ERRO!"),
                     MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
        return false;
    }


    bool aeroporto_near = true;
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    for (auto &elem : this->control.aeroportos) {
        int tmpX = abs(long(elem.pos.x - menu->componentes_graphicos.aeroporto.pos.x));
        int tmpY = abs(long(elem.pos.y - menu->componentes_graphicos.aeroporto.pos.y));
        if (tmpX < 10 && tmpY < 10) {
            aeroporto_near = false;
        }
    }
    return aeroporto_near;

}

bool Menu::desativa_novos_avioes() {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    return this->control.aceita_avioes = !this->control.aceita_avioes;
}


INT_PTR CALLBACK Desativa(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR) TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                MessageBoxEx(nullptr, TEXT("Miau"), TEXT("O Nuno Esteve Aqui!!!"),
                             MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR) TRUE;
            }
            break;
    }
    return (INT_PTR) FALSE;
}


void verificaAeroporto(Aeroporto &a, Control &control, bool &exit_loop) {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    auto existe = std::find_if(std::begin(control.aeroportos), std::end(control.aeroportos),
                               [&](Aeroporto tmp) { return !_tcscmp(tmp.nome, a.nome); });
    if (existe != std::end(control.aeroportos)) {
        MessageBoxEx(NULL, TEXT("Tens de ser mais original, esse nome ja foi patentiado:"), TEXT("ERRO!"),
                     MB_OK | MB_ICONINFORMATION | MB_TASKMODAL, 0);
        //tcout << t("Tens de ser mais original, esse nome ja foi patentiado:");
    } else {
        exit_loop = true;
    }
}


void Menu::consulta_aeroporto() {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    for (int i = 0; i < counter_aeroporto; i++) {
        tcout << t("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@") << std::endl;
        tcout << t("Aeroporto nr ") << this->control.aeroportos[i].IDAero << std::endl;
        tcout << t("Aeroporto ") << this->control.aeroportos[i].nome << std::endl;
        tcout << t("X-> ") << this->control.aeroportos[i].pos.x << t("\tY-> ") << this->control.aeroportos[i].pos.y
              << std::endl;
    }
    tcout << t("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@") << std::endl;
}

void Menu::consultar_aviao() {
    auto guard = CriticalSectionGuard(control.critical_section_interno);
    for (auto &aviao : control.avioes) {
        tcout << t("#############################################################################") << std::endl;
        tcout << t("Avião nr ") << aviao.IDAv << std::endl;
        tcout << t("Posição atual:") << std::endl;
        tcout << t("X-> ") << aviao.PosA.x << t("\tY-> ") << aviao.PosA.y << std::endl;
        tcout << t("Destino:") << std::endl;
        tcout << t("X-> ") << aviao.PosDest.x << t("\tY-> ") << aviao.PosDest.y << std::endl;
        tcout << t("Capacidade Maxima: ") << aviao.CapMax << std::endl;
        tcout << t("Velocidade: ") << aviao.velocidade << std::endl;
    }
    tcout << t("#############################################################################") << std::endl;
}

Menu::Menu(Control &control) : control(control), counter_aeroporto(0) {}




#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <time.h>


#define TAM_BUFFER 10

//estrutura para o buffer circular
typedef struct {
    int id;
    int val;
}CelulaBuffer;

//representa a nossa memoria partilhada
typedef struct {
    int nProdutores;
    int nConsumidores;
    int posE; //proxima posicao de escrita
    int posL; //proxima posicao de leitura
    CelulaBuffer buffer[TAM_BUFFER]; //buffer circular em si (array de estruturas)
}BufferCircular;


//estrutura de apoio
typedef struct {
    BufferCircular* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    int terminar; // 1 para sair, 0 em caso contr�rio
    int id;
}DadosThreads;

int num_aleatorio(int min, int max) {
    return rand() % (max - min + 1) + min;
}

DWORD WINAPI ThreadProdutor(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    CelulaBuffer cel;
    int contador = 0;

    while (!dados->terminar) {
        cel.id = dados->id;
        cel.val = num_aleatorio(10, 99);

        //aqui entramos na logica da aula teorica

        //esperamos por uma posicao para escrevermos
        WaitForSingleObject(dados->hSemEscrita, INFINITE);

        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);

        //vamos copiar a variavel cel para a memoria partilhada (para a posi��o de escrita)
        CopyMemory(&dados->memPar->buffer[dados->memPar->posE], &cel, sizeof(CelulaBuffer));
        dados->memPar->posE++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte

        //se apos o incremento a posicao de escrita chegar ao fim, tenho de voltar ao inicio
        if (dados->memPar->posE == TAM_BUFFER)
            dados->memPar->posE = 0;

        //libertamos o mutex
        ReleaseMutex(dados->hMutex);

        //libertamos o semaforo. temos de libertar uma posicao de leitura
        ReleaseSemaphore(dados->hSemLeitura, 1, NULL);

        contador++;
        _tprintf(TEXT("P%d produziu %d.\n"), dados->id, cel.val);
        Sleep(num_aleatorio(2, 4) * 1000);
    }
    _tprintf(TEXT("P%d produziu %d items.\n"), dados->id, contador);

    return 0;
}

int _tmain(int argc, TCHAR* argv[])
{
    HANDLE hFileMap; //handle para o file map
    HANDLE hThread;
    DadosThreads dados;
    BOOL  primeiroProcesso = FALSE;
    TCHAR comando[100];



#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif

    srand((unsigned int)time(NULL));

    //criar semaforo que conta as escritas
    dados.hSemEscrita = CreateSemaphore(NULL, TAM_BUFFER, TAM_BUFFER, TEXT("SO2_SEMAFORO_ESCRITA"));

    //criar semaforo que conta as leituras
    //0 porque nao ha nada para ser lido e depois podemos ir at� um maximo de 10 posicoes para serem lidas
    dados.hSemLeitura = CreateSemaphore(NULL, 0, TAM_BUFFER, TEXT("SO2_SEMAFORO_LEITURA"));

    //criar mutex para os produtores
    dados.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_PRODUTOR"));

    if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL || dados.hMutex == NULL) {
        _tprintf(TEXT("Erro no CreateSemaphore ou no CreateMutex\n"));
        return -1;
    }

    //o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
    //se devolver um HANDLE ja existe e nao fazemos a inicializacao
    //se devolver NULL nao existe e vamos fazer a inicializacao

    hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));
    if (hFileMap == NULL) {
        primeiroProcesso = TRUE;
        //criamos o bloco de memoria partilhada
        hFileMap = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(BufferCircular), //tamanho da memoria partilhada
            TEXT("SO2_MEM_PARTILHADA"));//nome do filemapping. nome que vai ser usado para partilha entre processos

        if (hFileMap == NULL) {
            _tprintf(TEXT("Erro no CreateFileMapping\n"));
            return -1;
        }
    }

    //mapeamos o bloco de memoria para o espaco de endera�amento do nosso processo
    dados.memPar = (BufferCircular*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);


    if (dados.memPar == NULL) {
        _tprintf(TEXT("Erro no MapViewOfFile\n"));
        return -1;
    }

    if (primeiroProcesso == TRUE) {
        dados.memPar->nConsumidores = 0;
        dados.memPar->nProdutores = 0;
        dados.memPar->posE = 0;
        dados.memPar->posL = 0;
    }


    dados.terminar = 0;

    //temos de usar o mutex para aumentar o nProdutores para termos os ids corretos
    WaitForSingleObject(dados.hMutex, INFINITE);
    dados.memPar->nProdutores++;
    dados.id = dados.memPar->nProdutores;
    ReleaseMutex(dados.hMutex);


    //lancamos a thread
    hThread = CreateThread(NULL, 0, ThreadProdutor, &dados, 0, NULL);
    if (hThread != NULL) {
        _tprintf(TEXT("Escreva qualquer coisa para sair ...\n"));
        _getts_s(comando, 100);
        dados.terminar = 1;

        //esperar que a thread termine
        WaitForSingleObject(hThread, INFINITE);
    }

    UnmapViewOfFile(dados.memPar);
    //CloseHandles ... mas � feito automaticamente quando o processo termina

    return 0;

}
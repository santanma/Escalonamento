#include "fila.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

struct programa 
{
    char nomePrograma[50];
    int inicioPrograma; // Para Round Robin será Inicializado -1
    int duracaoPrograma; // Para Round Robin será Inicializado 1s
    TipoEscalonamento tipoEscalonamentoPrograma; 
    bool enfileirado;
    __pid_t pidPrograma;
    bool executando;
};

struct noPrograma 
{
    Programa programa;
    NoPrograma *proximo;
};

struct fila
{
    NoPrograma *inicio;
    NoPrograma *final;
};

void inicializarPrograma(Programa *programa,char *nomePrograma,int inicioPrograma,int duracaoPrograma,TipoEscalonamento tipoEscalonamentoPrograma,__pid_t pidPrograma)
{
    strcpy(programa->nomePrograma,nomePrograma);

    programa->inicioPrograma = inicioPrograma;
    programa->duracaoPrograma = duracaoPrograma;
    programa->tipoEscalonamentoPrograma = tipoEscalonamentoPrograma;

    programa->enfileirado = false;
    programa->executando = false;

    programa->pidPrograma = pidPrograma;
}

char* pegarNomePrograma (Programa* programa)
{
    return programa->nomePrograma;
}

int pegarTempoInicioPrograma(Programa *programa)
{
    return programa->inicioPrograma;
}

int pegarTempoDuracaoPrograma(Programa *programa)
{
    return programa->duracaoPrograma;
}

TipoEscalonamento pegarTipoEscalonamentoPrograma (Programa* programa)
{
    return programa->tipoEscalonamentoPrograma;
}

bool verificarEnfileiramento(Programa *programa)
{
    return programa->enfileirado;
}

Fila* criarFila()
{
    Fila *f = (Fila*) malloc (sizeof(Fila));
    f->inicio = f->final = NULL;
    return f;
}

void inserirFila (Fila *f,Programa *programa)
{
    NoPrograma *noPrograma = (NoPrograma*) malloc (sizeof(NoPrograma));

    programa->enfileirado = true;

    if(strcmp(programa->nomePrograma,"") == 0)
    {
        return;
    }

    Programa novoPrograma = (*programa);

    noPrograma->programa = novoPrograma;
    noPrograma->proximo = NULL;

    if(f->final != NULL)
        f->final->proximo = noPrograma;
    else 
        f->inicio = noPrograma;

    f->final = noPrograma;
}

void imprimirFila (Fila *f)
{
    NoPrograma *no = f->inicio;
    FILE *arquivoLog = fopen("log_EscalonadorInterpretador.txt","a+");

    for(; no != NULL; no = no->proximo)
    {  
        fprintf(arquivoLog,"Programa Imprimindo %s\n",no->programa.nomePrograma);
        fprintf(arquivoLog,"TipoEscalonamento %s ",no->programa.tipoEscalonamentoPrograma == REALTIME ? "Realtime" : "RoundRobin");
        fprintf(arquivoLog,"Programa %s|Inicio = %d|Duracao = %d| -->\n",no->programa.nomePrograma,
                                                                       no->programa.inicioPrograma,
                                                                       no->programa.duracaoPrograma);

    }
    fprintf(arquivoLog,"Terminou\n");
    fflush(arquivoLog);
    fclose(arquivoLog);
}

void ordenarFila (Fila **f)
{
    Fila *filaAux = criarFila();
    int controleTempoInserido = 0;
    double menorTempoInicial;
    
    do 
    {
        NoPrograma *no = (*f)->inicio;
        NoPrograma *menorNo = NULL;

        menorTempoInicial = INFINITY;  

        for(; no != NULL ; no = no->proximo)
        {
            if(no->programa.inicioPrograma < menorTempoInicial && no->programa.inicioPrograma >= controleTempoInserido)
            {
                menorTempoInicial = no->programa.inicioPrograma;
                menorNo = no;
            }
        }

        if(menorNo != NULL)
        {
            Programa *programa = (Programa*) malloc (sizeof(Programa));
            inicializarPrograma(programa,menorNo->programa.nomePrograma,
                                         menorNo->programa.inicioPrograma,
                                         menorNo->programa.duracaoPrograma,
                                         menorNo->programa.tipoEscalonamentoPrograma,
                                         menorNo->programa.pidPrograma);

            inserirFila(filaAux,programa);

            controleTempoInserido = menorNo->programa.inicioPrograma + menorNo->programa.duracaoPrograma;
        }

    } while (menorTempoInicial != INFINITY);

    (*f) = filaAux;
}

Programa* removerProgramaFila (Fila *f)
{
    NoPrograma *no = f->inicio;

    Programa *programa = (Programa*) malloc (sizeof(Programa));
    inicializarPrograma(programa,no->programa.nomePrograma,
                                 no->programa.inicioPrograma,
                                 no->programa.duracaoPrograma,
                                 no->programa.tipoEscalonamentoPrograma,
                                 no->programa.pidPrograma);

    f->inicio = no->proximo;
    if(f->inicio == NULL)
        f->final = NULL;
    
    return programa;
}

void setarPidPrograma(Programa *programa,__pid_t pidPrograma)
{
    programa->pidPrograma = pidPrograma;
}

__pid_t pegarPidPrograma(Programa *programa)
{
    return programa->pidPrograma;
}

bool filaVazia(Fila *f)
{
    return (f->inicio == NULL);
}

void setarExecucaoPrograma(Programa *programa,bool executando)
{
    programa->executando = executando;
}

bool programaEmExecucao(Programa *programa)
{
    return programa->executando;
}
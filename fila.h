#include <stdbool.h>
#include <stdio.h>

typedef struct programa Programa;
typedef enum tipoescalonamento TipoEscalonamento;
enum tipoescalonamento 
{
    ROUND_ROBBIN,
    REALTIME
};

typedef struct noPrograma NoPrograma;
typedef struct fila Fila;


void inicializarPrograma(Programa *programa,char *nomePrograma,int inicioPrograma,int duracaoPrograma,TipoEscalonamento tipoEscalonamentoPrograma,__pid_t pidPrograma);

char* pegarNomePrograma (Programa* programa);

int pegarTempoInicioPrograma(Programa *programa);

int pegarTempoDuracaoPrograma(Programa *programa);
 
TipoEscalonamento pegarTipoEscalonamentoPrograma (Programa* programa);

bool verificarEnfileiramento(Programa *programa);

Fila* criarFila();

void inserirFila (Fila *f,Programa *programa);

void imprimirFila (Fila *f);

void ordenarFila (Fila **f);

Programa* removerProgramaFila (Fila *f);

void setarPidPrograma(Programa *programa,__pid_t pidPrograma);

__pid_t pegarPidPrograma(Programa *programa);

bool filaVazia(Fila *f);

void setarExecucaoPrograma(Programa *programa,bool executando);

bool programaEmExecucao(Programa *programa);
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>

#include "fila.h"

FILE *arquivoLog;

#define NOME_ARQUIVO "exec_UmRT.txt"
#define MAX_LINHA_ARQUIVO 50

void inicializarArquivoLog (char *nomeArquivoLog);
void inicializarControleRealTime (int *controleRealTime,int tamanhoVetor);

void inicializarArquivoLog (char *nomeArquivoLog)
{
	arquivoLog = fopen(nomeArquivoLog,"w");
	
	if(arquivoLog == NULL)
	{
		fprintf(arquivoLog,"Erro na Abertura do Arquivo de Log para Escrita\n");
		exit(0);
	}
	
	fprintf(arquivoLog,"Arquivo de Log Criado com Sucesso ---> %s\n",nomeArquivoLog);
    fclose(arquivoLog);
}

void inicializarControleRealTime (int *controleRealTime, int tamanhoVetor)
{
    for(int i = 0; i < tamanhoVetor; i++)
        controleRealTime[i] = 0;
}

int main (void)
{
    FILE *arquivoInterpretador;
    __pid_t pid;

    char nomePrograma[50];
    char tipoEscalonamentoArquivo /*Auxiliar Para Ler o Caracter do Arquivo*/;
    int inicioPrograma;
    int duracaoPrograma;
    bool alocado;

    Programa *programa;
    bool *interpretadorEncerrado;

    key_t chavePrograma = ftok("key_1.txt",'a');
    key_t chaveInterpretador = ftok("key_2.txt",'a');

    int memoriaCompartilhadaPrograma = shmget(chavePrograma,sizeof(Programa*),IPC_CREAT|0666);
    int memoriaCompartilhadaInterpretador = shmget(chaveInterpretador,sizeof(bool),IPC_CREAT|0666);

    programa = (Programa*) shmat (memoriaCompartilhadaPrograma,0,0);
    interpretadorEncerrado = (bool*) shmat (memoriaCompartilhadaInterpretador,0,0);

    *interpretadorEncerrado = false;

    int controleRealTime[60];

    inicializarControleRealTime(controleRealTime,60);

    pid = fork();

    if(pid == 0) // Execução do Filho -- Responsável pela Leitura do exec.txt
    {
        inicializarArquivoLog("log_EscalonadorInterpretador.txt");
        arquivoLog = fopen("log_EscalonadorInterpretador.txt","a+");

        arquivoInterpretador = fopen(NOME_ARQUIVO,"r");

        if(arquivoInterpretador == NULL)
	    {
		    fprintf(arquivoLog,"Erro na Abertura do Arquivo Interpretador para Leitura\n");
		    exit(0);
	    }

        while(fscanf(arquivoInterpretador,"%*s <%[^>]> %c=<%d> D=<%d>",nomePrograma,&tipoEscalonamentoArquivo,&inicioPrograma,&duracaoPrograma) != EOF)
        {
            if(tipoEscalonamentoArquivo == 'I') // Real Time
            {
                fprintf(arquivoLog,"Lendo Processo Realtime %s Inicio = %d Duracao = %d\n",nomePrograma,inicioPrograma,duracaoPrograma);
                
                alocado = true;

                // Verificar Limite de Tempo Máximo 
                if(inicioPrograma + duracaoPrograma > 60)
                {
                    alocado = false;
                    fprintf(arquivoLog,"Processo Realtime %s não Alocado - Tempo Máximo Excedido\n",nomePrograma);
                }
                else
                {
                    // Verificar Conflitos
                    for(int i = inicioPrograma; i < inicioPrograma + duracaoPrograma; i++)
                    {
                        if(controleRealTime[i] == 0)
                            continue;
                        else
                        {
                            alocado = false;
                            fprintf(arquivoLog,"Processo Realtime %s não Alocado - Tempos Conflitantes\n",nomePrograma);
                            break;
                        }
                    }

                    if(alocado)
                    {
                        for(int i = inicioPrograma; i < inicioPrograma + duracaoPrograma; i++)
                        {   
                            controleRealTime[i] = 1;
                        }

                        inicializarPrograma(programa,nomePrograma,inicioPrograma,duracaoPrograma,REALTIME,0);
                    }
                }

                tipoEscalonamentoArquivo = '*';
            }
            else //Round Robin
            {
                fprintf(arquivoLog,"Lendo Processo Round Robbin %s\n",nomePrograma);

                inicializarPrograma(programa,nomePrograma,-1,1,ROUND_ROBBIN,0);
            }

            fflush(arquivoLog);

            while(!verificarEnfileiramento(programa))
            {
                // Espera Retornar do Escalonador 
            }
            printf("Executando Interpretador\n");

            sleep(1);
        }

        *interpretadorEncerrado = true;
    }
    else // Execução do Pai
    {
        char *args[] = {NULL};
        execvp("./escalonador",args);
    }

    // Fechar Arquivos e Liberar Memória
    fclose(arquivoLog);
    fclose(arquivoInterpretador);

    shmdt(programa);
    shmdt(interpretadorEncerrado);

    shmctl(memoriaCompartilhadaPrograma,IPC_RMID,NULL);
    shmctl(memoriaCompartilhadaInterpretador,IPC_RMID,NULL);
}
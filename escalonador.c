#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#include "fila.h"

Fila *filaRealTime;
Fila *filaRoundRobin;

#define MAX_TAMANHO_PROGRAMA 100

int main (void)
{
    // Mesma Chave do Interpretador.c para Compartilhamento de Memória
    Programa *programa;
    Programa *programaRealTime;
    Programa *programaRoundRobin;
    Programa *programaRemovido;

    FILE *arquivoLog;

    char nomePrograma[MAX_TAMANHO_PROGRAMA];
    char *args[] = {NULL};

    bool *interpretadorEncerrado;
    bool executandoRealTime;

    struct timeval tempoInicial;
    struct timeval tempoFinal;
    
    int tempoDecorrido;
    int inicioPrograma;
    int duracaoPrograma;

    bool filaOrdenada;

    arquivoLog = fopen("log_EscalonadorInterpretador.txt","a+");

    filaRealTime = criarFila();
    filaRoundRobin = criarFila();

    key_t chavePrograma = ftok("key_1.txt",'a');
    key_t chaveInterpretador = ftok("key_2.txt",'a');
    key_t chaveRealTime = ftok("key_3.txt",'a');
    key_t chaveRoundRobin = ftok("key_4.txt",'a');

    int memoriaCompartilhadaPrograma = shmget(chavePrograma,sizeof(Programa*),IPC_CREAT|0666);
    int memoriaCompartilhadaInterpretador = shmget(chaveInterpretador,sizeof(bool),IPC_CREAT|0666);
    int memoriaCompartilhadaRealTime = shmget(chaveRealTime,sizeof(Programa*),IPC_CREAT|0666);
    int memoriaCompartilhadaRoundRobin = shmget(chaveRoundRobin,sizeof(Programa*),IPC_CREAT|0666);

    programa = (Programa*) shmat (memoriaCompartilhadaPrograma,0,0);
    interpretadorEncerrado = (bool*) shmat (memoriaCompartilhadaInterpretador,0,0);
    programaRealTime = (Programa*) shmat (memoriaCompartilhadaRealTime,0,0);
    programaRoundRobin = (Programa*) shmat (memoriaCompartilhadaRoundRobin,0,0);

    // Inicializar Com Valores Padrão
    inicializarPrograma(programaRealTime,"RealTime",-1,-1,REALTIME,0);
    inicializarPrograma(programaRoundRobin,"RoundRobin",-1,-1,ROUND_ROBBIN,0);

    while(1)
    {
        if(!(*interpretadorEncerrado) || !verificarEnfileiramento(programa))
        {
            if(pegarTipoEscalonamentoPrograma(programa) == REALTIME)    
            {   
                inserirFila(filaRealTime,programa);
            }
            else
            {
                inserirFila(filaRoundRobin,programa);
            }

            while(verificarEnfileiramento(programa))
            {
                // Espera Retornar do Interpretador    
                if(*interpretadorEncerrado) break;
            }
            // fprintf(arquivoLog,"Executando Escalonador\n");
            printf("Executando Escalonador\n");
        }
        else
        {
            if(!filaOrdenada)
            {
                ordenarFila(&filaRealTime);  

                filaOrdenada = true;

                // fprintf(arquivoLog,"\n*******Filas no Início do Programa*******\n");
                printf("\n*******Filas no Início do Programa*******\n");
                imprimirFila(filaRealTime);
                imprimirFila(filaRoundRobin);
                printf("*****************************************\n");

                //Iniciar Contagem
                gettimeofday(&tempoInicial,NULL);
                gettimeofday(&tempoFinal,NULL);
                continue;
            }
            else 
            {
                tempoDecorrido = (tempoFinal.tv_sec - tempoInicial.tv_sec)%60;

                if(tempoDecorrido == 0 &&
                !(strcmp(pegarNomePrograma(programaRealTime),"RealTime") == 0)) // Reinicia o Loop para RealTime se houver algum Programa RealTime
                    executandoRealTime = true;

                if(!filaVazia(filaRealTime) && strcmp(pegarNomePrograma(programaRealTime),"RealTime") == 0)
                {
                    executandoRealTime = true;

                    programaRemovido = removerProgramaFila(filaRealTime);

                    inicializarPrograma(programaRealTime,pegarNomePrograma(programaRemovido),
                                                           pegarTempoInicioPrograma(programaRemovido),
                                                           pegarTempoDuracaoPrograma(programaRemovido),
                                                           pegarTipoEscalonamentoPrograma(programaRemovido),
                                                           pegarPidPrograma(programaRemovido));

                    inicioPrograma = pegarTempoInicioPrograma(programaRealTime);
                    duracaoPrograma = pegarTempoDuracaoPrograma(programaRealTime);
                }

                if((!(strcmp(pegarNomePrograma(programaRealTime),"RealTime") == 0)) && 
                   (tempoDecorrido >= inicioPrograma && tempoDecorrido <= inicioPrograma + duracaoPrograma)
                   && executandoRealTime) // Prioridade RealTime
                {
                    if(pegarPidPrograma(programaRealTime) != 0 &&                            
                        waitpid(pegarPidPrograma(programaRealTime),NULL,WNOHANG) != 0)
                    {   
                        executandoRealTime = false;
                        inicializarPrograma(programaRealTime,"RealTime",-1,-1,REALTIME,0);
                        continue;
                    }

                    if(tempoDecorrido == inicioPrograma + duracaoPrograma)
                    {
                        kill(pegarPidPrograma(programaRealTime),SIGSTOP);

                        if(!filaVazia(filaRealTime))
                        {
                            inserirFila(filaRealTime,programaRealTime);
                            inicializarPrograma(programaRealTime,"RealTime",-1,-1,REALTIME,0);
                        }
                        
                        executandoRealTime = false;
                        continue;
                    }
                    else
                    {
                        strcpy(nomePrograma,"./");
                        strcat(nomePrograma,pegarNomePrograma(programaRealTime));
 
                        // fprintf(arquivoLog,"T:%d Executando RealTime %s\n",tempoDecorrido,nomePrograma);
                        printf("T:%d Executando RealTime %s\n",tempoDecorrido,nomePrograma);

                        if(fork() == 0)
                        {
                            __pid_t pidPrograma = pegarPidPrograma(programaRealTime);

                            if(pidPrograma == 0)
                            {
                                setarPidPrograma(programaRealTime,getpid());
                                execvp(nomePrograma,args);
                            }
                            else
                            {
                                kill(pidPrograma,SIGCONT);
                                exit(0);
                            }
                        }
                    }
                }
                else // Chance de Execução para o Round Robin
                {
                    if(!filaVazia(filaRoundRobin))
                    {
                        // Parar Programa Anterior
                        if(strcmp(pegarNomePrograma(programaRoundRobin),"RoundRobin") != 0)
                        {   
                            kill(pegarPidPrograma(programaRoundRobin),SIGSTOP);
                        }

                        programaRemovido = removerProgramaFila(filaRoundRobin);

                        if(pegarPidPrograma(programaRemovido) != 0 
                        && waitpid(pegarPidPrograma(programaRemovido),NULL,WNOHANG) != 0)
                        {   
                            continue;
                        } 

                        inicializarPrograma(programaRoundRobin,pegarNomePrograma(programaRemovido),
                                                               pegarTempoInicioPrograma(programaRemovido),
                                                               pegarTempoDuracaoPrograma(programaRemovido),
                                                               pegarTipoEscalonamentoPrograma(programaRemovido),
                                                               pegarPidPrograma(programaRemovido));

                        strcpy(nomePrograma,"./");
                        strcat(nomePrograma,pegarNomePrograma(programaRoundRobin));

                        // fprintf(arquivoLog,"T:%d Executando Round Robin %s\n",tempoDecorrido,nomePrograma);
                        printf("T:%d Executando Round Robin %s\n",tempoDecorrido,nomePrograma);

                        if(fork() == 0)
                        {   
                            __pid_t pidPrograma = pegarPidPrograma(programaRoundRobin);

                            if(pidPrograma == 0)
                            {
                                setarPidPrograma(programaRoundRobin,getpid());
                                setarExecucaoPrograma(programaRoundRobin,true);
                                execvp(nomePrograma,args);
                            }
                            else
                            {
                                setarExecucaoPrograma(programaRoundRobin,true);
                                kill(pidPrograma,SIGCONT);
                                exit(0); //Impede que siga Criando mais Filhos
                            }
                        }
                                            
                        while(!programaEmExecucao(programaRoundRobin)) // Impede que Insira na Fila antes do Programa começar a Executar
                        {
                        
                        }

                        setarExecucaoPrograma(programaRoundRobin,false);
                    
                        if(waitpid(pegarPidPrograma(programaRoundRobin),NULL,WNOHANG) == 0)
                        {   
                            inserirFila(filaRoundRobin,programaRoundRobin);
                        }
                    }
                    else 
                    {
                        if(!(filaVazia(filaRealTime) 
                            && filaVazia(filaRoundRobin) 
                            && waitpid(pegarPidPrograma(programaRealTime),NULL,WNOHANG) != 0
                            && waitpid(pegarPidPrograma(programaRoundRobin),NULL,WNOHANG) != 0
                            && !executandoRealTime)) // Continua Imprimindo até acabarem todos os Programas
                            {
                                // fprintf(arquivoLog,"T:%d - Aguardando\n",tempoDecorrido);
                                printf("T:%d - Aguardando\n",tempoDecorrido);
                            }
                    }
                }
            }
        }

        if(filaVazia(filaRealTime) 
             && filaVazia(filaRoundRobin) 
             && waitpid(pegarPidPrograma(programaRealTime),NULL,WNOHANG) != 0
             && waitpid(pegarPidPrograma(programaRoundRobin),NULL,WNOHANG) != 0
             && !executandoRealTime)
             {
                // fprintf(arquivoLog,"Fim de Todos os Programas\n"); 
                printf("Fim de Todos os Programas\n"); 
                break;
             }
        
       // fflush(arquivoLog);
        sleep(1);
        gettimeofday(&tempoFinal,NULL);
    }

    // fprintf(arquivoLog,"\n*******Filas ao Final do Programa*******\n");
    printf("\n*******Filas ao Final do Programa*******\n");

    imprimirFila(filaRealTime);
    imprimirFila(filaRoundRobin);

    fclose(arquivoLog);

    shmdt(programa);
    shmdt(interpretadorEncerrado);
    shmdt(programaRealTime);
    shmdt(programaRoundRobin);

    shmctl(memoriaCompartilhadaPrograma,IPC_RMID,NULL);
    shmctl(memoriaCompartilhadaInterpretador,IPC_RMID,NULL);
    shmctl(memoriaCompartilhadaRealTime,IPC_RMID,NULL);
    shmctl(memoriaCompartilhadaRoundRobin,IPC_RMID,NULL);
}
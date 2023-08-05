#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

int main (void)
{
    unsigned int i,j;

    printf("Imprimir P2 %d\n",getpid());

    clock_t inicio = clock();

    for(i = 0; i < 2; i++)
        for(j = 0; j < UINT_MAX; j++);

    clock_t termino = clock();

    double tempoDecorrido = (double)(termino - inicio)/ CLOCKS_PER_SEC;

    printf("T2 = %lf\n",tempoDecorrido);
}

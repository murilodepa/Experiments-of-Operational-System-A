/*******************************************************************************
* Este programa esta baseado no segundo experimento do curso sobre tempo real 
* do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #2: Multi-Tasking, Measuring Drift
*'
*    Programmer: Eric Sorton
*          Date: 1/27/97
*           For: MSE599, Special Topics Class
*
*       Purpose: When a basic sleep call is used to determine the precise time
*                when an action will occur the problem of drift occurs. 
*                The measurement of time is imprecise.  Similarly, the time in 
*                which the sleep call returns is imprecise.  Over time, this 
*                will cause the ocurrence of time to drift.  Just as if a clock 
*                loses 1 second every day, over one day, it is significant, but 
*                over a year, it loses 365 seconds, which is over 6 minutes.  
*                This is an example of drift.
*
*       Proposito: Quando uma chamada básica sleep e usada para determinar o
*                instante exato em que alguma acao vai ocorrer, ocorre o problema
*                do desvio. A medicao de tempo e imprecisa. Similarmente, o tempo
*                que demora o retorno da chamada sleep tambem e impreciso. Ao
*                longo do tempo, isto ocasionara um desvio de tempo. Algo como se
*                um relogio perdesse um segundo a cada dia. Ao longo de um dia, 
*                essa diferenca e insignificante, mas, ao longo de um ano, sao 
*                perdidos 365 segundos, o que e superior a 6 minutos. Este e um
*                exemplo de desvio.
*******************************************************************************/

/* Includes Necessarios, verifique se as bibliotecas no diretorio sys/ estao lah. Caso nao estejam, verifique onde estao e altere o include */
#include <unistd.h>    /* for gettimeofday() and fork() */
#include <stdio.h>     /* for printf() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h>  /* for wait() */
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* NO_OF_CHILDREN eh o numero de filhos a serem criados, cada qual responsavel pela medida do desvio. */
#define NO_OF_CHILDREN 5

/* Programa Principal. Contem tanto o codigo para o processo pai como o codigo dos futuros filhos */
int main(int argc, char *argv[])
{
    /* start_time e stop_time conterao o valor de tempo antes e depois que as trocas de contexto comecem */
    struct timeval start_time;
    struct timeval stop_time;

    /* Outras variaveis importantes */
    float drift;
    int count, i = 0;
    int Pid, rtn;
    char PidChar[20];

    /* Criacao dos processos filhos */
    rtn = 1;

    Pid = getpid();
    sprintf(PidChar, "%d", Pid);
    
    char *args[] = {"2200", PidChar, NULL};
    
    for (count = 0; count < NO_OF_CHILDREN; count++)
    {
        if (rtn != 0)
        {
            rtn = fork();
        }
        else
        {
            break;
        }
    }
    
    /* Verifica-se rtn para determinar se o processo eh pai ou filho */
    if (rtn == 0)
    { 
        /* Portanto, sou filho. Faco coisas de filho. */
        execvp("./filho", args);
    }
    else
    {
        /* Sou pai, aguardo o termino dos filhos */
        for (count = 0; count < NO_OF_CHILDREN; count++)
            wait(NULL);
    }

    exit(0);
}
/*******************************************************************************
* Este programa esta baseado no segundo experimento do curso sobre tempo real 
* do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #2: Multi-Tasking, Measuring Drift
*
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
*       Proposito: Quando uma chamada b�sica sleep e usada para determinar o
*                instante exato em que alguma acao vai ocorrer, ocorre o problema
*                do desvio. A medicao de tempo e imprecisa. Similarmente, o tempo
*                que demora o retorno da chamada sleep tambem e impreciso. Ao
*                longo do tempo, isto ocasionara um desvio de tempo. Algo como se
*                um relogio perdesse um segundo a cada dia. Ao longo de um dia, 
*                essa diferenca e insignificante, mas, ao longo de um ano, sao 
*                perdidos 365 segundos, o que e superior a 6 minutos. Este e um
*                exemplo de desvio.
*******************************************************************************/

/* Includes Necessarios */
#include <sys/time.h>  /* for gettimeofday() */
#include <unistd.h>	   /* for gettimeofday() and fork() */
#include <stdio.h>	   /* for printf() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h>  /* for wait() */
#include <stdlib.h>

/* NO_OF_ITERATIONS e o numero de vezes que vai se repetir o loop existente em cada futuro processo filho. */
#define NO_OF_ITERATIONS 1000

/* NO_OF_CHILDREN eh o numero de filhos a serem criados, cada qual responsavel pela medida do desvio. */
#define NO_OF_CHILDREN 3

/* SLEEP_TIME corresponde a quantidade de tempo para ficar bloqueado. */
#define SLEEP_TIME 1000

/* MICRO_PER_SECOND define o numero de microsegundos em um segundo */
#define MICRO_PER_SECOND 1000000

/* Programa Principal. Contem tanto o codigo para o processo pai como o codigo dos futuros filhos */
int main(int argc, char *argv[])
{
	/* start_time e stop_time conterao o valor de tempo antes e depois que as trocas de contexto comecem */
	struct timeval start_time;
	struct timeval stop_time;

	/* Outras variaveis importantes */
	float drift;
	int count;
	int child_no, rtn;

	/* Criacao dos processos filhos */
	rtn = 1;

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
		child_no = count;

		/* Primeiro, obtenho o tempo inicial. */
		gettimeofday(&start_time, NULL);

		/* Este loop ocasiona a minha dormencia, de acordo com SLEEP_TIME, tantas vezes quanto NO_OF_ITERATIONS */
		for (count = 0; count < NO_OF_ITERATIONS; count++)
			usleep(SLEEP_TIME);
		
		/* Paraobter o tempo final */
		gettimeofday(&stop_time, NULL);

		/* Calcula-se o desvio */
		drift = (float)(stop_time.tv_sec - start_time.tv_sec);
		drift += (stop_time.tv_usec - start_time.tv_usec) / (float)MICRO_PER_SECOND;

		/* Exibe os resultados */
		printf("Filho #%d -- desvio total: %.5f -- desvio medio: %.8f\n",
			   child_no, drift - NO_OF_ITERATIONS * SLEEP_TIME / MICRO_PER_SECOND,
			   (drift - NO_OF_ITERATIONS * SLEEP_TIME / MICRO_PER_SECOND) / NO_OF_ITERATIONS);
	}
	else
	{
		/* Sou pai, aguardo o termino dos filhos */
		for (count = 0; count < NO_OF_CHILDREN; count++)
			wait(NULL);
	}

	exit(0);
}
/* Includes Necessarios */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>  /* for gettimeofday() */
#include <unistd.h>	   /* for gettimeofday() and fork() */
#include <stdio.h>	   /* for printf() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h>  /* for wait() */
#include <stdlib.h>

/* NO_OF_ITERATIONS e o numero de vezes que vai se repetir o loop existente em cada futuro processo filho. */
#define NO_OF_ITERATIONS 1000

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

	gettimeofday(&start_time, NULL);

	int sleep_time = atoi(argv[0]);

	/* Este loop ocasiona a minha dormencia, de acordo com SLEEP_TIME, tantas vezes quanto NO_OF_ITERATIONS */
	for (count = 0; count < NO_OF_ITERATIONS; count++)
		usleep(sleep_time);

	/* Para obter o tempo final */
	gettimeofday(&stop_time, NULL);

	/* Calcula-se o desvio */
	drift = (float)(stop_time.tv_sec - start_time.tv_sec);
	drift += (stop_time.tv_usec - start_time.tv_usec) / (float)MICRO_PER_SECOND;

	/* Exibe os resultados */
	printf("Filho # %d -- Desvio Total: %.5f -- Desvio Medio: %.8f\n",
	getpid() - atoi(argv[1]), drift - NO_OF_ITERATIONS * sleep_time / MICRO_PER_SECOND,
	(drift - NO_OF_ITERATIONS * sleep_time / MICRO_PER_SECOND) / NO_OF_ITERATIONS);

	return 0;
}
/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #5: Semaphores
*
*    Programmer: Eric Sorton
*          Date: 3/17/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to demonstrate how semaphores
*		 can be used to protect a critical region.  Its sole purpose
*		 is to print a character string (namely the alphabet) to the
*		 screen.  Any number of processes can be used to cooperatively
*		 (or non-cooperatively) print the string to the screen.  An
*		 index is stored in shared memory, this index is the index into
*		 the array that identifies which character within the string
*		 should be printed next.  Without semaphores, all the processes
*		 access this index simultaneously and conflicts occur.  With
*		 semahpores, the character string is displayed neatly to the
*		 screen.
*
*		 The optional semaphore protection can be compiled into the
*		 program using the MACRO definition of PROTECT.  To compile
*		 the semaphore protection into the program, uncomment the
*		 #define below.
*
*
*       Proposito: O proposito deste programa e o de demonstrar como semaforos
*		podem ser usados para proteger uma regiao critica. O programa exibe
*		um string de caracteres (na realidade um alfabeto). Um n�mero 
*		qualquer de processos pode ser usado para exibir o string, seja
*		de maneira cooperativa ou nao cooperativa. Um indice e armazenado
*		em memoria compartilhada, este indice e aquele usado para 
* 		identificar qual caractere deve ser exibido em seguida. Sem 
*		semaforos, todos os processos acessam esse indice concorrentemente 
*		causando conflitos. Com semaforos, o string de caracteres e exibido
*		de maneira correta (caracteres do alfabeto na ordem correta e apenas
*		um de cada caractere).
*
*		A protecao opcional com semaforo pode ser compilada no programa
*		usando a definicao de MACRO denominada PROTECT. Para compilar a
*		protecao com semaforo, retire o comentario do #define que segue.
*
*
*******************************************************************************/

#define PROTECT

/* Includes Necessarios */
#include <stdlib.h>
#include <errno.h>	   /* errno and error codes */
#include <sys/time.h>  /* for gettimeofday() */
#include <stdio.h>	   /* for printf() */
#include <unistd.h>	   /* for fork() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h>  /* for wait() */
#include <signal.h>	   /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>   /* for all IPC function calls */
#include <sys/shm.h>   /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>   /* for semget(), semop(), semctl() */

/* Constantes Necessarias */
#define SEM_KEY2 0x4428
#define SEM_KEY 0x1243
#define SHM_KEY 0x2549
#define SHM_KEY2 0x1337
#define SHM_KEY3 0x6496

#define NO_OF_CHILDREN 8

// Criando memória compartilhada do tipo struct buffer
struct buffer
{
	char buffer[67];
	int qtd_cons, qtd_prod;
};

// Criando o índice da memória compartilhada
struct buffer *g_shm_buffer;

/*
 * As seguintes variaveis globais contem informacao importante. A variavel
 * g_sem_id e g_shm_id contem as identificacoes IPC para o semaforo e para
 * o segmento de memoria compartilhada que sao usados pelo programa. A variavel
 * g_shm_addr e um ponteiro inteiro que aponta para o segmento de memoria
 * compartilhada que contera o indice inteiro da matriz de caracteres que contem
 * o alfabeto que sera exibido.
 */
int g_sem_id, g_sem_id2;
int g_shm_id, g_shm_id2, g_shm_id3;
int *g_shm_addr, *g_shm_addr2;

/*
 * As seguintes duas estruturas contem a informacao necessaria para controlar
 * semaforos em relacao a "fecharem", se nao permitem acesso, ou 
 * "abrirem", se permitirem acesso. As estruturas sao incializadas ao inicio
 * do programa principal e usadas na rotina PrintAlphabet(). Como elas sao
 * inicializadas no programa principal, antes da criacao dos processos filhos,
 * elas podem ser usadas nesses processos sem a necessidade de nova associacao
 * ou mudancas.
*/
struct sembuf g_sem_op1[1];
struct sembuf g_sem_op2[1];

/*
 * O seguinte vetor de caracteres contem o alfabeto que constituira o string
 * que sera exibido.
*/
char g_letters_and_numbers[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890";

/* Funcoes */
void Produtor(int cod);
void Consumidor(void);

/* Programa Principal */
int main(int argc, char *argv[])
{
	/* Variaveis necessarias */
	int rtn;
	int count;

	/*
     * Para armazenar os ids dos processos filhos, permitindo o posterior
     * uso do comando kill
     */
	int pid[NO_OF_CHILDREN];

	/* Construindo a estrutura de controle do semaforo */
	g_sem_op1[0].sem_num = 0;
	g_sem_op1[0].sem_op = 1;
	g_sem_op1[0].sem_flg = 0;

	g_sem_op2[0].sem_num = 0;
	g_sem_op2[0].sem_op = -1;
	g_sem_op2[0].sem_flg = 0;

	/* Criando o semaforo 1 */
	if ((g_sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if (semop(g_sem_id, g_sem_op1, 1) == -1)
	{
		fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	// Criando o semaforo 2
	if ((g_sem_id2 = semget(SEM_KEY2, 1, IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if (semop(g_sem_id2, g_sem_op1, 1) == -1)
	{
		fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	/* Criando indice 1 compartilhado */
	if ((g_shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if ((g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1)
	{
		fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	*g_shm_addr = 0;

	/* Criando indice 2 compartilhado */
	if ((g_shm_id2 = shmget(SHM_KEY2, sizeof(int), IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	if ((g_shm_addr2 = (int *)shmat(g_shm_id2, NULL, 0)) == (int *)-1)
	{
		fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	*g_shm_addr2 = 0;
	*g_shm_addr = 0;

	/* Criando buffer compartilhado */
	if ((g_shm_id3 = shmget(SHM_KEY3, sizeof(struct buffer), IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if ((g_shm_buffer = (struct buffer *)shmat(g_shm_id3, NULL, 0)) == (struct buffer *)-1)
	{
		fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	g_shm_buffer->qtd_cons = 0;
	g_shm_buffer->qtd_prod = 0;

	/* Criando os filhos */
	rtn = 1;

	for (count = 0; count < NO_OF_CHILDREN; count++)
	{
		if (rtn != 0)
		{
			pid[count] = rtn = fork();
		}
		else
		{
			break;
		}
	}

	/* Verificando o valor retornado para determinar se o processo eh pai ou filho */
	if (rtn == 0)
	{
		/* Eu sou um filho */
		printf("Filho %i comecou ...\n", count);
		if (count <= 4)
		{
			Produtor(count);
		}
		else
		{
			Consumidor();
		}
	}
	else
	{
		usleep(50000);

		/* Matando os processos filhos */
		for (int i = 0; i < 8; i++)
		{
			kill(pid[i], SIGKILL);
		}

		/* Removendo as memórias compartilhadas */
		if (shmctl(g_shm_id, IPC_RMID, NULL) != 0)
		{
			fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
			exit(1);
		}

		if (shmctl(g_shm_id2, IPC_RMID, NULL) != 0)
		{
			fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
			exit(1);
		}

		if (shmctl(g_shm_id3, IPC_RMID, NULL) != 0)
		{
			fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
			exit(1);
		}

		/* Removendo o semáforo */
		if (semctl(g_sem_id, 0, IPC_RMID, 0) != 0)
		{
			fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
			exit(1);
		}

		if (semctl(g_sem_id2, 0, IPC_RMID, 0) != 0)
		{
			fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
			exit(1);
		}

		exit(0);
	}
}

/*
 * Esta rotina realiza a exibicao de caracteres. Nela e calculado um numero
 * pseudo-randomico entre 1 e 3 para determinar o numero de caracteres a exibir.
 * Se a protecao esta estabelecida, a rotina entao consegue o recurso. Em
 * seguida, PrintChars() acessa o indice com seu valor corrente a partir da
 * memoria compartilhada. A rotina entra em loop, exibindo o numero aleatorio de
 * caracteres. Finalmente, a rotina incrementa o indice, conforme o necessario,
 * e libera o recurso, se for o caso.
*/

void Consumidor()
{
	struct timeval tv;
	int number;
	int tmp_index;
	int i;
	int indice1, indice2;

	usleep(200);
	while (1)
	{

#ifdef PROTECT
		if (semop(g_sem_id, g_sem_op2, 1) == -1)
		{
			fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
			exit(1);
		}
#endif

		indice1 = *g_shm_addr;

#ifdef PROTECT
		if (semop(g_sem_id, g_sem_op1, 1) == -1)
		{
			fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
			exit(1);
		}
#endif

#ifdef PROTECT
		if (semop(g_sem_id2, g_sem_op2, 1) == -1)
		{
			fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
			exit(1);
		}
#endif

		indice2 = *g_shm_addr2;

#ifdef PROTECT
		if (semop(g_sem_id2, g_sem_op1, 1) == -1)
		{
			fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
			exit(1);
		}
#endif

		if (gettimeofday(&tv, NULL) == -1)
		{
			fprintf(stderr, "Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}

		number = ((tv.tv_usec / 47) % 5) + 1;

		if (indice1 > (indice2 + number))
		{
			usleep(20 * number);

#ifdef PROTECT
			if (semop(g_sem_id2, g_sem_op2, 1) == -1)
			{
				fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
				exit(1);
			}
#endif

			tmp_index = indice2;

			for (i = 0; i < number; i++)
			{
				g_shm_buffer->buffer[(tmp_index + i) % 66] = '#';
				usleep(1);
			}

			*g_shm_addr2 = tmp_index + i;

			if ((tmp_index + i) / 66 > g_shm_buffer->qtd_cons)
			{
				for (int j = 0; j < 66; j++)
				{
					fprintf(stderr, "%c", g_shm_buffer->buffer[j]);
				}

				g_shm_buffer->qtd_cons += 1;
				fprintf(stderr, "\n");
			}

#ifdef PROTECT
			if (semop(g_sem_id2, g_sem_op1, 1) == -1)
			{
				fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
				exit(1);
			}
#endif
		}
	}
}

void Produtor(int count)
{
	struct timeval tv;
	int number;
	char produzir[20];

	int tmp_index;
	int i;

	/* Este tempo permite que todos os filhos sejam inciados */
	usleep(200);

	/* Entrando no loop principal */
	while (1)
	{
		usleep(20 * number);
		/*
         * Conseguindo o tempo corrente, os microsegundos desse tempo
         * sao usados como um numero pseudo-randomico. Em seguida,
         * calcula o numero randomico atraves de um algoritmo simples
		 */
		if (gettimeofday(&tv, NULL) == -1)
		{
			fprintf(stderr, "Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}

		number = ((tv.tv_usec / 47) % 5) + 1;

		/*
		 * O #ifdef PROTECT inclui este pedaco de codigo se a macro
         * PROTECT estiver definida. Para sua definicao, retire o comentario
         * que a acompanha. semop() e chamada para fechar o semaforo.
         */
#ifdef PROTECT
		if (semop(g_sem_id, g_sem_op2, 1) == -1)
		{
			fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
			exit(1);
		}
#endif

		/* Lendo o indice do segmento de memoria compartilhada */
		tmp_index = *g_shm_addr;

		/*
         * Repita o numero especificado de vezes, esteja certo de nao
         * ultrapassar os limites do vetor, o comando if garante isso
		 */
		for (i = 0; i < number; i++)
		{
			g_shm_buffer->buffer[(tmp_index + i) % 66] = g_letters_and_numbers[(tmp_index + i) % 66];
			produzir[i] = g_letters_and_numbers[(tmp_index + i) % 66];
			usleep(1);
		}

		/* Atualizando o indice na memoria compartilhada */
		*g_shm_addr = tmp_index + i;

		/*
      	 * Se o indice e maior que o tamanho do alfabeto, exibe um
       	 * caractere return para iniciar a linha seguinte e coloca
       	 * zero no indice
		 */
		if ((tmp_index + i) / 66 > g_shm_buffer->qtd_prod)
		{
			for (int j = 0; j < 66; j++)
			{
				fprintf(stderr, "%c", g_shm_buffer->buffer[j]);
			}
			fprintf(stderr, "\n");
			g_shm_buffer->qtd_prod += 1;
		}

		fprintf(stderr, "O responsavel e o filho : %d  producao: ", count);

		for (int j = 0; j < i; j++)
		{
			fprintf(stderr, "%c", produzir[j]);
		}

		printf("\n");

		/* Liberando o recurso se a macro PROTECT estiver definida */
#ifdef PROTECT
		if (semop(g_sem_id, g_sem_op1, 1) == -1)
		{
			fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
			exit(1);
		}
#endif
	}
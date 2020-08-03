// Algortimo para resolução do problema do "Barbeiro Dorminhoco"

// Includes necessários
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// Define's necessários
#define NUMBARB 3
#define NUMCLI 27
#define SEM_KEY 0x1243
#define SEM_KEY2 0x1255
#define SEM_KEY3 0x1275

// Criação de um mutex
pthread_mutex_t mutex;

// Criação das duas thread's utilizadas no problema
pthread_t barbeiros[NUMBARB];
pthread_t clientes[NUMCLI];

struct sembuf g_lock_op[27];
struct sembuf g_unlock_op[27];

int g_sem_cli;
int g_sem_bar;
int g_sem_print;

struct informComp
{
	unsigned char vetPreOrdenado[NUMCLI][1030];
	unsigned char vetPosOrdenado[NUMCLI][1030];
	int indices[NUMCLI];
	int intuitoCortar[NUMCLI];
	int fila;
	int barbeiroQueCortara[NUMCLI];
};
typedef struct informComp informComp;

informComp auxInformGeral;

bool work = true;

void apreciate_hair(int id, float delta);
void *barbeiro(void *id);
void *cliente(void *id);
void cut_hair(unsigned char Vetor[], int Tamanho);
void semaphoreStruct();
void removeSem(int g_sem_id);

void apreciate_hair(int id, float delta)
{

	if (semop(g_sem_print, g_lock_op, 1) == -1)
	{
		fprintf(stderr, "chamada semop() falhou, impossivel bloquear o semaforo!\n");
		exit(1);
	}

	printf("%d teve cabelo cortado por %d e levou %5f\n", id, auxInformGeral.barbeiroQueCortara[id], delta);
	printf("Vetor pre ordenado: \n");
	for (int i = 0; i < auxInformGeral.indices[id]; i++)
	{
		printf("%d |", auxInformGeral.vetPreOrdenado[id][i]);
	}
	printf("\n\n");

	printf("Vetor pos ordenado: \n");
	for (int i = 0; i < auxInformGeral.indices[id]; i++)
	{
		printf("%d |", auxInformGeral.vetPosOrdenado[id][i]);
	}
	printf("\n\n");

	if (semop(g_sem_print, g_unlock_op, 1) == -1)
	{
		fprintf(stderr, "chamada semop() falhou, impossivel desbloquear o semaforo!\n");
		exit(1);
	}

	return;
}

void *barbeiro(void *id)
{
	struct timeval time;
	int numBarbeiro = *(int *)id;
	int clienteEscolhido = -1;

	while (work)
	{
		clienteEscolhido = -1;
		pthread_mutex_lock(&mutex);
		for (int i = 0; i < NUMCLI; i++)
		{
			if (auxInformGeral.intuitoCortar[i] == 1)
			{
				clienteEscolhido = i;
				auxInformGeral.intuitoCortar[clienteEscolhido] = 0;
				auxInformGeral.barbeiroQueCortara[clienteEscolhido] = numBarbeiro;
				auxInformGeral.fila -= 1;
				break;
			}
		}
		pthread_mutex_unlock(&mutex);

		if (clienteEscolhido != -1)
		{
			if (semop(g_sem_cli, g_lock_op, 1) == -1)
			{
				fprintf(stderr, "chamada semop() falhou, impossivel bloquear o semaforo!\n");
				exit(1);
			}

			strcpy(auxInformGeral.vetPosOrdenado[clienteEscolhido], auxInformGeral.vetPreOrdenado[clienteEscolhido]);
			cut_hair(auxInformGeral.vetPosOrdenado[clienteEscolhido], auxInformGeral.indices[clienteEscolhido]);

			if (semop(g_sem_bar, (g_unlock_op + clienteEscolhido), 1) == -1)
			{
				fprintf(stderr, "chamada semop() falhou, impossivel desbloquear o semaforo!\n");
				exit(1);
			}
		}
	}

	pthread_exit(NULL);
}

void *cliente(void *id)
{
	int numeroCliente = *(int *)id;
	unsigned char aux[1030];
	struct timeval antes;
	struct timeval depois;
	float delta;

	gettimeofday(&antes, NULL);

	int flag = 0;
	while (flag != 1)
	{
		pthread_mutex_lock(&mutex);
		if (auxInformGeral.fila < 7)
		{
			srand(time(NULL) + numeroCliente);
			flag = 1;
			auxInformGeral.indices[numeroCliente] = ((rand() % 1021) + 2);
			auxInformGeral.fila += 1;

			if (semop(g_sem_cli, g_unlock_op, 1) == -1)
			{
				fprintf(stderr, "chamada semop() falhou, impossivel desbloquear o semaforo!\n");
				exit(1);
			}
			pthread_mutex_unlock(&mutex);

			for (int i = 0; i < auxInformGeral.indices[numeroCliente]; i++)
			{
				auxInformGeral.vetPreOrdenado[numeroCliente][i] = ((rand() + numeroCliente) % 254) + 1;
			}

			auxInformGeral.intuitoCortar[numeroCliente] = 1;

			if (semop(g_sem_bar, (g_lock_op + numeroCliente), 1) == -1)
			{
				fprintf(stderr, "chamada semop() falhou, impossivel bloquear o semaforo!\n");
				exit(1);
			}
			gettimeofday(&depois, NULL);
			delta = depois.tv_usec - antes.tv_usec;
			apreciate_hair(numeroCliente, delta);
		}
		else
		{
			pthread_mutex_unlock(&mutex);
		}
	}

	pthread_exit(NULL);
}

void cut_hair(unsigned char Vetor[], int Tamanho)
{
	int Cont1 = 0, Cont2 = 0, Auxiliar = 0, Posicao = 0;

	for (Cont1 = 0; Cont1 < (Tamanho - 1); Cont1++)
	{
		Posicao = Cont1;

		for (Cont2 = (Cont1 + 1); Cont2 <= (Tamanho - 1); Cont2++)
		{
			if ((int)Vetor[Cont2] < (int)Vetor[Posicao])
			{
				Posicao = Cont2;
			}
		}

		if (Cont1 != Posicao)
		{
			Auxiliar = Vetor[Cont1];
			Vetor[Cont1] = Vetor[Posicao];
			Vetor[Posicao] = Auxiliar;
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	int tb, tc;

	int numBarbeiro[NUMBARB];
	int numCliente[NUMCLI];

	for (int i = 0; i < NUMCLI; i++)
	{
		for (int j = 0; j < 1024; j++)
		{
			auxInformGeral.vetPosOrdenado[i][j] = -1;
		}
	}

	if (pthread_mutex_init(&mutex, NULL))
	{
		printf("Erro ao iniciar o mutex\n");
		exit(-1);
	}

	for (int i = 0; i < 27; i++)
	{
		g_lock_op[i].sem_num = i;
		g_lock_op[i].sem_op = -1;
		g_lock_op[i].sem_flg = 0;

		g_unlock_op[i].sem_num = i;
		g_unlock_op[i].sem_op = 1;
		g_unlock_op[i].sem_flg = 0;
	}

	if ((g_sem_cli = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos CLIENTES!\n");
		exit(1);
	}

	if ((g_sem_print = semget(SEM_KEY3, 1, IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos CLIENTES!\n");
		exit(1);
	}

	if ((g_sem_bar = semget(SEM_KEY2, NUMCLI, IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos BARBEIROS!\n");
		exit(1);
	}
	if (semop(g_sem_print, g_unlock_op, 1) == -1)
	{
		fprintf(stderr, "chamada semop() falhou, impossivel desbloquear o semaforo!\n");
		exit(1);
	}

	for (int i = 0; i < NUMCLI; i++)
	{
		auxInformGeral.intuitoCortar[i] = 0;
	}
	auxInformGeral.fila = 0;

	for (int i = 0; i < NUMBARB; i++)
	{
		numBarbeiro[i] = i;
		tb = pthread_create(&barbeiros[i], NULL, barbeiro, (void *)&numBarbeiro[i]);
		if (tb)
		{
			printf("ERRO: impossivel criar um thread consumidor\n");
			exit(0);
		}
	}

	for (int i = 0; i < NUMCLI; i++)
	{
		numCliente[i] = i;
		tc = pthread_create(&clientes[i], NULL, cliente, (void *)&numCliente[i]);
		if (tc)
		{
			printf("ERRO: impossivel criar um thread consumidor\n");
			exit(0);
		}
	}

	for (int i = 0; i < NUMCLI; i++)
	{
		if (pthread_join(clientes[i], NULL))
		{
			printf("Impossível esperar thread cliente\n");
			exit(-1);
		}
	}
	work = false;

	for (int i = 0; i < NUMBARB; i++)
	{
		if (pthread_join(barbeiros[i], NULL))
		{
			printf("Impossível esperar thread barbeiro\n");
			exit(-1);
		}
	}

	if (pthread_mutex_destroy(&mutex))
	{
		printf("Impossível destruir mutex cliente\n");
		exit(-1);
	}

	if (semctl(g_sem_bar, 0, IPC_RMID, 0) != 0)
	{
		fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
		exit(1);
	}

	if (semctl(g_sem_cli, 0, IPC_RMID, 0) != 0)
	{
		fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
		exit(1);
	}

	if (semctl(g_sem_print, 0, IPC_RMID, 0) != 0)
	{
		fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
		exit(1);
	}

	exit(0);
}
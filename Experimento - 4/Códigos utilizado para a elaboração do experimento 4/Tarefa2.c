// Includes necessários
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex;
pthread_mutex_t mutex_filo[5];
pthread_t filosofos[5];
int refeicao[5] = {0};
int status[5];
int j = 0;
int filomen[5] = {0, 1, 2, 3, 4};

void *filosofo(void *num)
{
	int *id = (int *)(num);

	while (refeicao[*id] < 365)
	{
		pensar();
		pegar_hashi(*id);
		usleep(5000);
		devolver_hashi(*id);
	}

	for (int i; i < 5; i++)
	{
		printf("\n\nnumero de refeições de cara = %d\n", refeicao[i]);
	}

	pthread_exit(NULL);
}

void devolver_hashi(int num)
{
	pthread_mutex_lock(&(mutex));
	status[num] = 0;
	testar((num + 4) % 5);
	testar((num + 1) % 5);
	pthread_mutex_unlock(&(mutex));
}

void pegar_hashi(int id_filosofo)
{
	pthread_mutex_lock(&(mutex));
	status[id_filosofo] = 1;
	testar(id_filosofo);
	pthread_mutex_unlock(&(mutex));
	pthread_mutex_lock(&(mutex_filo[id_filosofo]));
	refeicao[(id_filosofo)] += 1;
}

void testar(int num)
{
	printf("Filosofo %d tem intencao \n", num + 1);
	for (int i = 0; i < 5; i++)
	{
		switch (status[i])
		{
		case 0:
			printf("o filosofo %d esta pensando \n", i + 1);
			break;
		case 1:
			printf("o filosofo %d esta faminto \n", i + 1);
			break;
		case 2:
			printf("o filosofo %d esta comendo \n", i + 1);
			break;
		}
	}

	if (status[num] == 1 && status[(num + 1) % 5] != 2 && status[(num + 4) % 5] != 2)
	{
		status[num] = 2;
		pthread_mutex_unlock(&(mutex_filo[num]));
	}
}

void pensar()
{
	usleep(12);
}

int main(int argc, char *argv[])
{
	int tf;

	pthread_mutex_init(&mutex, NULL);
	for (int i = 0; i < 5; i++)
	{
		pthread_mutex_init(&mutex_filo[i], NULL);
	}

	for (j = 0; j < 5; j++)
	{
		tf = pthread_create(&filosofos[j], NULL, filosofo, (void *)&filomen[j]);
		if (tf)
		{
			printf("erro ao criar filosofo %d", j);
			exit(0);
		}
	}

	for (int i = 0; i < 5; i++)
	{
		pthread_join(filosofos[i], NULL);
	}

	pthread_mutex_destroy(&(mutex));

	for (int i = 0; i < 5; i++)
	{
		pthread_mutex_destroy(&(mutex_filo[i]));
	}
}
/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #3: Shared Resources, Measureing Message Queue Transfer Time
*
*    Programmer: Eric Sorton
*          Date: 2/11/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to measure the time it takes
*                a message to be transfered across a message queue.  The
*                total time will include the time to make the call to msgsnd(),
*                the time for the system to transfer the message, the time
*                for the context switch, and finally, the time for the other
*                end to call msgrcv().
*
*                The algorithm for this program is very simple:
*
*                   o The parent creates the message queue
*                   o The parents starts two children
*                   o The first child will:
*                         - Receive a message on the queue
*                         - Call gettimeofday() to get the current time
*                         - Using the time in the message, calculate
*                              the difference and store it in an array
*                         - Loop (X number of times)
*	   			  - Display the results
*                   o The second child will:
*                         - Call gettimeofday() to get the current time
*                         - Place the time in a message
*                         - Place the message on the queue
*                         - Pause to allow the other child to run
*                         - Loop (X number of times)
*                   o The parent waits for the children to finish
*
* Traduzindo: 
*
*     Prop�sito: O prop�sito deste programa � a medicao do tempo que leva
*                uma mensagem para ser transferida por uma fila de mensagens.
*                O tempo total incluira o tempo para realizar a chamada 
*                msgsnd(), o tempo para o sistema transferir a mensagem, o
*                tempo para troca de contexto e, finalmente, o tempo para,
*                na outra ponta, ocorrer a chamada msgrcv().
*
*                O algoritmo para este programa e bastante simples:
*
*                   o O pai cria a fila de mensagens
*                   o O pai inicializa dois filhos
*                   o O primeiro filho:
*                         - Recebe uma mensagem pela fila
*                         - Chama gettimeofday() para obter o tempo atual
*                         - Usando o tempo existente na mensagem, calcula
*                              a diferenca
*                         - Repete (numero X de vezes)
*				  - Exibe os resultados
*                   o O segundo filho:
*                         - Chama gettimeofday() para obter o tempo atual
*                         - Coloca o tempo em uma mensagem
*                         - Coloca a mensagem na fila
*                         - Realiza uma pausa para permitir a execucao do irmao
*                         - Repete (numero X de vezes)
*                   o O pai espera os filhos terminarem
*
*******************************************************************************/

/* Includes Necessarios */
#include <stdlib.h>
#include <sys/time.h>  /* for gettimeofday() */
#include <stdio.h>	   /* for printf() */
#include <unistd.h>	   /* for fork() */
#include <sys/types.h> /* for wait(), msgget(), msgctl() */
#include <wait.h>	   /* for wait() */
#include <sys/ipc.h>   /* for msgget(), msgctl() */
#include <sys/msg.h>   /* for msgget(), msgctl() */

/*
 * NO_OF_ITERATIONS corresponde ao numero de mensagens que serao enviadas.
 * Se este numero cresce, o tempo de execucao tambem deve crescer.
 */
#define NO_OF_ITERATIONS 500

/* NO_OF_CHILDREN eh o numero de filhos a serem criados. */
#define NO_OF_CHILDREN 3

/* MICRO_PER_SECOND define o numero de microsegundos em um segundo */
#define MICRO_PER_SECOND 1000000

/*
 * MESSAGE_QUEUE_ID eh uma chave arbitraria, foi escolhido um numero qualquer,
 * que deve ser unico. Se outra pessoa estiver executando este mesmo programa
 * ao mesmo tempo, o numero pode ter que ser mudado!
 */
#define MESSAGE_QUEUE_ID 3102

/* Constantes */
#define SENDER_DELAY_TIME 10

/* Definindo o tipo da mensagem como 1 */
#define MESSAGE_MTYPE 1

/* Definindo variável para manipular o tamanho da mensagem */
int tamanhoMensagem = 1;

/* Struct para armazenar dados enviados e rercebidos por mensagem entre processos */
typedef struct
{
	float max, min, total, medio;
} data_filho2;

/* Prótotipo das funções utilizadas no programa principal */
void Sender(int queue_id, int identificador, data_filho2 dados);
void Receiver(int queue_id, int identificador, data_filho2 *dados);

/* Programa principal */
int main(int argc, char *argv[])
{
	/* Algumas variaveis necessarias */
	int rtn = 1;
	int count = 0;

	/* Variaveis relativas a fila de mensagem (id) */
	int queue_id, queue_id_2;
	data_filho2 dados;

	/* Variável para receber a chave de cada fila de mensagem (key) */
	key_t key = MESSAGE_QUEUE_ID;
	key_t key2 = 703;

	/* Entrada do dado referente ao tamanho da mensagem, e calculado como solicitado no enunciado da tarefa */
	printf("Digite o tamanho da mensagem: ");
	scanf("%d", &tamanhoMensagem);
	tamanhoMensagem = tamanhoMensagem * 512;

	/* Criacao dos processos filhos */
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

	/*
	 * Verifica o valor retornado para determinar se o processo eh pai ou filho
	 *
	 * OBS:  o valor de count no loop anterior indicara cada um dos filhos
	 *       count = 1 para o primeiro filho, 2 para o segundo, etc.
	 */
	if (rtn == 0 && count == 1)
	{

		/* Cria a fila de mensagens */
		if ((queue_id = msgget(key, IPC_CREAT | 0666)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		/* Chamada da função para realizar o envio da mensagem com os dados necessários da tarefa */
		Sender(queue_id, 0, dados);

		exit(0);
	}
	else if (rtn == 0 && count == 2)
	{
		/* Cria a fila de mensagens */
		if ((queue_id = msgget(key, IPC_CREAT | 0666)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		/* Chamada da função para realizar o recebimento da mensagem com os dados necessários da tarefa */
		Receiver(queue_id, 0, &dados);

		/* Removendo a fila de mensagens */
		if (msgctl(queue_id, IPC_RMID, NULL) == -1)
		{
			fprintf(stderr, "Impossivel remover a fila!\n");
			exit(1);
		}

		/* Cria a segunda fila de mensagens */
		if ((queue_id_2 = msgget(key2, IPC_CREAT | 0666)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		/* Chamada da função para realizar o envio da mensagem com os dados necessários da tarefa */
		Sender(queue_id_2, 1, dados);

		exit(0);
	}
	else if (rtn == 0 && count == 3)
	{
		/* Cria a segunda fila de mensagens */
		if ((queue_id_2 = msgget(key2, IPC_CREAT | 0666)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		/* Chamada da função para realizar o recebimento da mensagem com os dados necessários da tarefa */
		Receiver(queue_id_2, 1, &dados);

		/* Removendo a segunda fila de mensagens */
		if (msgctl(queue_id_2, IPC_RMID, NULL) == -1)
		{
			fprintf(stderr, "Impossivel remover a fila!\n");
			exit(1);
		}

		exit(0);
	}
	else
	{

		/* Sou o pai aguardando meus filhos terminarem */
		printf("Pai aguardando ...\n");

		for (count = 0; count < NO_OF_CHILDREN; count++)
			wait(NULL);

		exit(0);
	}
}

/*
 * O tipo de dados seguinte pode ser usado para declarar uma estrutura que
 * contera os dados que serao transferidos pela fila.  A estrutura vai conter 
 * um numero de mensagem (msg_no) e o tempo de envio (send_time).  Para filas 
 * de mensagens, o tipo da estrutura pode definir qualquer dado que seja necessario.
 */
typedef struct
{
	unsigned int msg_no;
	struct timeval send_time;
	int j;
} data_t;

/* 
 * O conteudo de uma estrutura com o seguinte tipo de dados sera enviado 
 * atraves da fila de mensagens. O tipo define dois dados.  O primeiro eh
 * o tipo da mensagem (mtype) que sera como uma identificacao de mensagem. 
 * Neste exemplo, o tipo eh sempre o mesmo. O segundo eh um vetor com tamanho
 * igual ao definido pelo tipo declarado anteriormente. Os dados a serem 
 * transferidos sao colocados nesse vetor. Na maioria das circunstancias,
 * esta estrutura nao necessita mudar.
 */
typedef struct
{
	long mtype;
	char mtext[5120];
} msgbuf_t;

/* Esta funcao executa o recebimento das mensagens */
void Receiver(int queue_id, int identificador, data_filho2 *dados)
{
	/* Variaveis locais */
	int count;
	struct timeval receive_time;
	float delta;
	float max = 0, min = 100;
	float total;

	/* Este eh o buffer para receber a mensagem */
	msgbuf_t message_buffer;

	/* Analisa qual é o identificador passado como parâmetro */ 
	if (identificador == 0)
	{
	    /*
	 	 * Este e o ponteiro para os dados no buffer.  Note
	     * como e setado para apontar para o mtext no buffer
	     */
		data_t *data_ptr = (data_t *)(message_buffer.mtext);

	    /* Inicia o loop */
		for (count = 0; count < NO_OF_ITERATIONS; count++)
		{

			/* Recebe qualquer mensagem do tipo MESSAGE_MTYPE */
			if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, tamanhoMensagem, MESSAGE_MTYPE, 0) == -1)
			{
				fprintf(stderr, "Impossivel receber mensagem!\n");
				exit(1);
			}

			/* Chama gettimeofday() */
			gettimeofday(&receive_time, NULL);

			/* Calcula a diferenca */
			delta = receive_time.tv_sec - data_ptr->send_time.tv_sec;
			delta += (receive_time.tv_usec - data_ptr->send_time.tv_usec) / (float)MICRO_PER_SECOND;
			total += delta;

			/* Salva o tempo máximo */
			if (delta > max)
				max = delta;
		
			/* Salva o tempo mínimo */
			if (delta < min)
				min = delta;
		}

		/* Exibe os resultados */
		(*dados).max = max;
		(*dados).min = min;
		(*dados).total = total;
		(*dados).medio = total / NO_OF_ITERATIONS;
	}
	else
	{
		/* Este e o buffer para a segunda fila de mensagens enviadas */
		msgbuf_t message_buffer2;

		/*
	 	* Este e o ponteiro para os dados no buffer.  Note
	 	* como e setado para apontar para mtext no buffer
	 	*/
		data_filho2 *data_enviar = (data_filho2 *)(message_buffer2.mtext);
		
		/* Recebe qualquer mensagem do tipo MESSAGE_MTYPE */
		if (msgrcv(queue_id, (struct msgbuf *)&message_buffer2, tamanhoMensagem, MESSAGE_MTYPE, 0) == -1)
		{
			fprintf(stderr, "Impossivel receber mensagem!\n");
			exit(1);
		}

		/* Exibe os resultados */
		printf("O tempo medio de transferencia: %.10f\n", data_enviar->medio);
		printf("o tempo maximo de transferencia: %.10f\n", data_enviar->max);
		printf("o tempo minimo de transferencia: %.10f\n", data_enviar->min);
		printf("o tempo total de transferencia: %.10f\n", data_enviar->total);
	}

	return;
}

/* Esta funcao envia mensagens */
void Sender(int queue_id, int identificador, data_filho2 dados)
{
	/* Variaveis locais */
	int count;

	/* Variável necessária para analisar o tempo de execução */
	struct timeval send_time;

	/* Este e o buffer para as mensagens enviadas */
	msgbuf_t message_buffer;

	/*
	 * Este e o ponteiro para od dados no buffer.  Note
	 * como e setado para apontar para mtext no buffer
	 */

	/* Analisa qual é o identificador passado como parâmetro */ 
	if (identificador == 0)
	{
		/*
	 	* Este e o ponteiro para os dados no buffer.  Note
	 	* como e setado para apontar para mtext no buffer
	 	*/
		data_t *data_ptr = (data_t *)(message_buffer.mtext);

		/* Inicia o loop */
		for (count = 0; count < NO_OF_ITERATIONS; count++)
		{
			/* Chama gettimeofday() */
			gettimeofday(&send_time, NULL);

			/* Realiza a preparação para enviar ou receber os dados por mensagem */
			message_buffer.mtype = MESSAGE_MTYPE;
			data_ptr->msg_no = count;
			data_ptr->send_time = send_time;

			/*
		 	* Envia a mensagem... usa a identificacao da fila, um ponteiro
		 	* para o buffer, e o tamanho dos dados enviados
		 	*/
			if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, tamanhoMensagem, 0) == -1)
			{
				fprintf(stderr, "Impossivel enviar mensagem!\n");
				exit(1);
			}

			/* "Dorme" por um curto espaco de tempo */
			usleep(SENDER_DELAY_TIME);
		}
	}
	else
	{
		/* Este e o buffer para a segunda fila de mensagens enviadas */
		msgbuf_t message_buffer2;

		/*
	 	* Este e o ponteiro para os dados no buffer.  Note
	 	* como e setado para apontar para mtext no buffer
	 	*/		
		data_filho2 *data_enviar = (data_filho2 *)(message_buffer2.mtext);
		
		/* Realiza a preparação para enviar ou receber os dados por mensagem */
		message_buffer2.mtype = MESSAGE_MTYPE;
		data_enviar->max = dados.max;
		data_enviar->medio = dados.medio;
		data_enviar->total = dados.total;
		data_enviar->min = dados.min;

		/*
		 * Envia a mensagem... usa a identificacao da fila, um ponteiro
		 * para o buffer, e o tamanho dos dados enviados
		 */
		if (msgsnd(queue_id, (struct msgbuf *)&message_buffer2, tamanhoMensagem, 0) == -1)
		{
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
	}

	return;
}

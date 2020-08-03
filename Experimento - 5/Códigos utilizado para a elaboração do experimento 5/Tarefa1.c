// Algortimo para resolução do problema do "Barbeiro Dorminhoco"

// Includes necessários
#include <stdlib.h>
#include <errno.h>    
#include <sys/time.h> 
#include <stdio.h>    
#include <unistd.h>   
#include <sys/types.h>
#include <sys/wait.h> 
#include <signal.h>   
#include <sys/ipc.h>  
#include <sys/shm.h>  
#include <sys/sem.h>   
#include <sys/msg.h>  
#include <string.h>
#include <time.h> 

// Define's necessários
#define CHAIRS 7    
#define CLIENTES 20
#define BARBEIROS 2 
#define NUMEROFILHOS 22
#define NO_OF_ITERATIONS 0 

// Criação das chaves necessárias
#define SHM_KEY 0x2549
#define SEM_KEY 0x1243
#define SEM_KEY2 0x1255

int *Espera;
int *g_shm_addr;
int g_shm_id;
int queue_id;
key_t key = 550;
int g_sem_id, g_sem_id2;

typedef struct{
    unsigned char vetorPreOrdenado[1030];
    unsigned char vetorPosOrdenado[1030];
    int tamVet;
    long int pidCliente;
    long int pidBarbeiro;
    struct timeval send_time;
} data_t;

typedef struct{
    long int mtype;
    char mtext[sizeof(data_t)];
} msgbuf_t;

struct sembuf g_sem_op1[1];
struct sembuf g_sem_op2[1];

void cliente(int pid){  
    unsigned char vetorRand[1030];
    
    int randomico = ((rand() % 1020) + 2);
    if (semop(g_sem_id, g_sem_op2, 1) == -1){
        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
        exit(1);
    }
    
    int fila = *g_shm_addr;
    struct timeval receive_time;

    if (fila < 7){
        *g_shm_addr = fila + 1;
        if (semop(g_sem_id, g_sem_op1, 1) == -1){
            fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }

        msgbuf_t message_buffer;
        data_t *data_ptr = (data_t *)(message_buffer.mtext);

        for (int i = 0; i < randomico; i++){
            vetorRand[i] = ((rand() + pid) % 255)+1;
        }

        strcpy(data_ptr->vetorPreOrdenado, vetorRand);
        message_buffer.mtype = 55;
        data_ptr->tamVet = randomico;
        data_ptr->pidCliente = pid;

        gettimeofday(&receive_time, NULL);
        data_ptr->send_time = receive_time;

        if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 0) == -1){
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }
        apreciate_hair(pid);
    }
    else{
            if (semop(g_sem_id, g_sem_op1, 1) == -1){
            fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }
        printf("\n cliente %d  não encontrou acento livre e foi embora\n", pid);
    }
    return;
}

void cut_hair(unsigned char vet[], int tam){
    SelectSort(vet, tam);
    return;
}

void apreciate_hair(int pid){
    float delta;
    msgbuf_t message_buffer;
    struct timeval send_time;
    data_t *data_ptr = (data_t *)(message_buffer.mtext);

    if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), pid, 0) == -1){
        fprintf(stderr, "Impossivel receber mensagem!\n");
        exit(1);
    }

    if (semop(g_sem_id2, g_sem_op2, 1) == -1){
        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
        exit(1);
    }

    gettimeofday(&send_time, NULL);
    delta = (send_time.tv_usec - data_ptr->send_time.tv_usec) / (float)1000000;
    printf("/n O barbeiro: %d, cortou o cabelo do cliente: %d, levando %.20fs para o fazer\n ", data_ptr->pidBarbeiro, data_ptr->pidCliente, delta);

    printf("\nVetor nao ordenado: \n");
    for (int count = 0; count < data_ptr->tamVet; count++){
        printf(" | %d | ", data_ptr->vetorPreOrdenado[count]);
    }
    printf("\n");

    printf("\nVetor ordenado:\n");
    for (int count = 0; count < data_ptr->tamVet; count++){
        printf(" | %d | ", data_ptr->vetorPosOrdenado[count]);
    }

    printf("\n");

    if (semop(g_sem_id2, g_sem_op1, 1) == -1){
        fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
        exit(1);
    }

    return;
}

void barbeiro(int pid)
{
    char *vetOrdenado;
    msgbuf_t message_buffer;
    data_t *data_ptr = (data_t *)(message_buffer.mtext);
    char aux[1024];

    while (1)
    {
        if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 55, 0) == -1){
            exit(1);
        }

        int fila = *g_shm_addr;

        if (semop(g_sem_id, g_sem_op2, 1) == -1){
            fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
            exit(1);
        }

        *g_shm_addr = fila - 1;

        if (semop(g_sem_id, g_sem_op1, 1) == -1){
            fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }

        strcpy(data_ptr->vetorPosOrdenado, data_ptr->vetorPreOrdenado);
        SelectSort(data_ptr->vetorPosOrdenado, data_ptr->tamVet);

        message_buffer.mtype = data_ptr->pidCliente;
        data_ptr->pidBarbeiro = pid;

        if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 0) == -1){
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }
    }
    return;
}

void SelectSort(unsigned char Vetor[], int Tamanho)
{
    int Aux1 = 0, Aux2 = 0, Aux3 = 0, Posicao = 0;

    for (Aux1 = 0; Aux1 < (Tamanho - 1); Aux1++){
        Posicao = Aux1;

        for (Aux2 = (Aux1 + 1); Aux2 <= (Tamanho - 1); Aux2++){
            if ((int)Vetor[Aux2] > (int)Vetor[Posicao]){
                Posicao = Aux2;
            }
        }

        if (Aux1 != Posicao){
            Aux3 = Vetor[Aux1];
            Vetor[Aux1] = Vetor[Posicao];
            Vetor[Posicao] = Aux3;
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    int rtn = 1;
    int count = 0;
    int pid[3];

    g_sem_op1[0].sem_num = 0;
    g_sem_op1[0].sem_op = 1;
    g_sem_op1[0].sem_flg = 0;

    g_sem_op2[0].sem_num = 0;
    g_sem_op2[0].sem_op = -1;
    g_sem_op2[0].sem_flg = 0;

    if ((g_sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }

    if (semop(g_sem_id, g_sem_op1, 1) == -1){
        fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }

    if ((g_sem_id2 = semget(SEM_KEY2, 1, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }

    if (semop(g_sem_id2, g_sem_op1, 1) == -1){
        fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }

    if ((queue_id = msgget(key, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
        exit(1);
    }

    if ((g_shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    if ((g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1){ 
        fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }

    *g_shm_addr = 0;

    for (count = 0; count < NUMEROFILHOS; count++){
        if (rtn != 0){
            if (count == 1 || count == 2){
                pid[count] = rtn = fork();
            }
            else{
                rtn = fork();
            }
        }
        else{
            break;
        }
    }

    if (rtn == 0){
        if (count == 1){
            barbeiro(count);
        }
        else{
            if (count == 2){
                barbeiro(count);
            }
            else{
                cliente((count - 2));
            }
        }
    }
    else{
        for (count = 0; count < 20; count++){
            wait(NULL);
        }
        if (msgctl(queue_id, IPC_RMID, NULL) == -1){
            fprintf(stderr, "Impossivel remover a fila!\n");
            exit(1);
        }
        if (shmctl(g_shm_id, IPC_RMID, NULL) != 0){
            fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
            exit(1);
        }
        if (semctl(g_sem_id, 0, IPC_RMID, 0) != 0){
            fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
            exit(1);
        }
        if (semctl(g_sem_id2, 0, IPC_RMID, 0) != 0){
            fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
            exit(1);
        }

        kill(pid[1], SIGKILL);
        kill(pid[2], SIGKILL);
        fprintf(stderr, "Fim do expediente\n");
    }

    exit(0);
}
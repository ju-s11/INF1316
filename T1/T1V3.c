#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define NUM_PROCESSOS 3 
#define TIMESLICE 5000000000000000000000000000000
#define MAX_IRQ 50
#define MAX_EXECUCOES 3
#define D1 1
#define D2 2

#define SHM_KEY 1236

typedef struct {
    pid_t pid;
    int id;
    int pc; // Program Counter
    int estado; // 0: executando, 1: bloqueado ou pausado, 2: finalizado
    int dispositivo; // dispositivo associado, se bloqueado
    char operacao; // operação (R, W, X)
    int acessos[2]; // contador de acessos para dispositivos D1 e D2
    int execucoes;
} Processo;

typedef struct {
    Processo processo[NUM_PROCESSOS];  
    int irq;  
    char op; 
    int inter_controller_finished;
} MemoriaCompartilhada;


void ajusta_prontos(Processo* prontos) {
    prontos[0] = prontos[1];
    prontos[1] = prontos[2];
    prontos[2].pid = -1;
    prontos[2].pc = 0;
    prontos[2].estado = 1;
    prontos[2].dispositivo = 0;
    prontos[2].operacao = 'X'; 
    prontos[2].acessos[0] = 0;
    prontos[2].acessos[1] = 0;
}

void adiciona_prontos(Processo* prontos, Processo processo) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (prontos[i].pid == -1) {
            prontos[i] = processo;
            return; // Adiciona e sai da função
        }
    }
}

void ajusta_em_espera(Processo *em_espera) {
    em_espera[0] = em_espera[1];
    em_espera[1] = em_espera[2];
    em_espera[2].pid = -1;
    em_espera[2].pc = 0;
    em_espera[2].estado = 1;
    em_espera[2].dispositivo = 0;
    em_espera[2].operacao = 'X'; 
    em_espera[2].acessos[0] = 0;
    em_espera[2].acessos[1] = 0;
}

void adiciona_em_espera(Processo* em_espera, Processo processo) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (em_espera[i].pid == -1) {
            em_espera[i] = processo;
            return; // Adiciona e sai da função
        }
    }
}


void interControllerSim(MemoriaCompartilhada *shm) {
    srand(time(NULL));
    int irq_count = 0;

    while (irq_count < MAX_IRQ) {
        usleep(TIMESLICE);
        shm->irq = 0;    
        printf("InterControllerSim: Gerando IRQ0 (Time slice)\n");
        irq_count++;

        int irq_type = rand() % 100;
        if (irq_type < 10) {
            shm->irq = 1;  
            shm->op = 'W'; 
            printf("InterControllerSim: Gerando IRQ1 (Dispositivo D1) - Operação: Write\n");
        } else if (irq_type < 15) {
            shm->irq = 2;  
            shm->op = 'R'; 
            printf("InterControllerSim: Gerando IRQ2 (Dispositivo D2) - Operação: Read\n");
        }
    }

    printf("InterControllerSim: Limite de gerações de IRQ alcançado. Encerrando...\n");
    shm->inter_controller_finished = 1; 
    shm->inter_controller_finished = 1;
    exit(0); 
}

void kernelSim(MemoriaCompartilhada *shm) {
    int PC = 0;         
    int d;              
    int Dx;             
    char Op;           

    while (PC < MAX_EXECUCOES && !shm->inter_controller_finished) { // Limitar o número de execuções
        sleep(TIMESLICE);  // Pausa

        // Gera um número aleatório entre 1 e 100
        d = rand() % 100 + 1;

        // Verifica se a chamada de sistema deve ser executada
        if (d < 15) {  // 15% de chance de executar uma syscall
            // Define Dx aleatoriamente
            if (d % 2) {
                Dx = D1;
            } else {
                Dx = D2;
            }

            // Define a operação (R, W, X) aleatoriamente
            if (d % 3 == 1) {
                Op = 'R';  // Read
            } else if (d % 3 == 2) {
                Op = 'W';  // Write
            } else {
                Op = 'X';  // Execute
            }

            // Verifica se deve executar uma syscall
        if (shm->irq > 0) {
            printf("Syscall: (D%d, %c)\n", shm->irq == 1 ? D1 : D2, shm->op);
            shm->irq = 0; // Reseta a IRQ após o uso
        }

        PC++;
    }

    // printf("kernelSim: Encerrando...\n");
    }
}

int main() {
    printf("Tamanho da MemoriaCompartilhada: %lu bytes\n", sizeof(MemoriaCompartilhada));
    srand(time(NULL));  // Inicializa a semente para números aleatórios

    // Cria a memória compartilhada
    int shm_id = shmget(SHM_KEY, sizeof(MemoriaCompartilhada), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Erro ao criar memória compartilhada com shget");
        exit(EXIT_FAILURE);
    }

    MemoriaCompartilhada *shm = (MemoriaCompartilhada *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("Erro ao anexar memória compartilhada com shmat");
        exit(EXIT_FAILURE);
    }

    // Inicializa a memória compartilhada
    memset(shm, 0, sizeof(MemoriaCompartilhada));
    shm->inter_controller_finished = 0; // Inicializa como não terminado

    // Inicia o controlador de interrupções em um processo separado
    if (fork() == 0) {
        interControllerSim(shm);
        exit(0);
    }

    // Simulação do Kernel
    kernelSim(shm);

    // Aguarda o término do processo do controlador de interrupções
    wait(NULL);

    // Limpeza
    shmdt(shm);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}


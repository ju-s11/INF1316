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

#define NUM_PROCESSOS 3 //número de processos que o kernel irá lidar.
#define TIMESLICE 1 //time slide = 500 ms. está 1 segundo para vizualização melhor do estado do programa.

//Memória Compartilhada

#define SHM_KEY 0x1234   //chave para memória compartilhada

//Visualizando os estados dos processos

//estrutura para representar o contexto de um processo de aplicação
typedef struct {
    int pc; //Program Counter (contador de instruções)
    int estado; //0: executando, 1: bloqueado, 2: finalizado
    int dispositivo; //dispositivo associado, se bloqueado
    char operacao; //operação (R, W, X) realizada
    int acessos[2]; //contador de acessos para dispositivos D1 e D2
} Processo;

//estrutura que será armazenada na memória compartilhada
typedef struct {
    Processo processo[NUM_PROCESSOS];  //estado dos processos filhos
    int irq;  //indica a interrupção gerada pelo controlador (0: IRQ0, 1: IRQ1, 2: IRQ2)
} MemoriaCompartilhada;

//implementar a fila de prontos

//implementar a fila de espera

//Time sharing

// Função que simula o controlador de interrupções
void interControllerSim(MemoriaCompartilhada *shm) {
    srand(time(NULL));
    while (1) {
        // Gera IRQ0 a cada 500ms (time slice)
        usleep(500000);  // 500 ms para representar o time slice
        shm->irq = 0;    // IRQ0 indica interrupção de relógio
        printf("InterControllerSim: Gerando IRQ0 (Time slice)\n");

        // Gera interrupções aleatórias para dispositivos D1 e D2
        int irq_type = rand() % 3;  // Gera IRQ1 ou IRQ2 aleatoriamente
        if (irq_type == 1) {
            shm->irq = 1;  // IRQ1 para dispositivo D1
            printf("InterControllerSim: Gerando IRQ1 (Dispositivo D1)\n");
        } else if (irq_type == 2) {
            shm->irq = 2;  // IRQ2 para dispositivo D2
            printf("InterControllerSim: Gerando IRQ2 (Dispositivo D2)\n");
        }
    }
}


// Função do kernelSim (gerencia os filhos)
void kernelSim(MemoriaCompartilhada *shm) {
    pid_t pids[NUM_PROCESSOS];
    
    //cria processos filhos
    pids[0] = fork();
    shm->processo[0].estado = 2; //marca pi como READY
    if (pids[0] == 0) { //bloco de código de P1.
        printf("P1 criado.\n"); 
        execl("./p1", "p1", (char *)NULL);
        kill(pids[0], SIGSTOP);
    }

    pids[1] = fork();
    shm->processo[1].estado = 2; //marca pi como READY
    if (pids[1] == 0) { //bloco de código de P2.
        printf("P2 criado.\n");
        execl("./p2", "p2", (char *)NULL);
        kill(pids[1], SIGSTOP);
    }

    pids[2] = fork();
    shm->processo[2].estado = 2; //marca pi como READY
    if (pids[2] == 0) { //bloco de código de P3.
        printf("P3 criado.\n");
        execl("./p3", "p3", (char *)NULL); 
        kill(pids[2], SIGSTOP);
    }

    //IRQ0 - interrupção por clock

    while (1) {
        int n = 0;
        kill(pids[n], SIGSTOP); //pausa o processo atual
        //adicionar processo no fim da fila de prontos e ajustar a fila
        printf("KernelSim: Processo %d está na fila READY e foi pausado\n", n);
        n++; //passa para o próximo processo na "fila" de prontos e executa ele. na verdade ele sempre executará o primeiro elemento da fila de prontos
        kill(pids[n], SIGCONT); //executa o próximo processo da fila de prontos
        sleep(TIMESLICE);
    }

    /*
    for (int i = 0; i < NUM_PROCESSOS; i++) {
            if (shm->processo[i] == 2) {  // READY
                printf("KernelSim: Processo %d está READY e será executado\n", i);
                shm->processo[i] = 0;  // Executando (RUNNING)
                printf("KernelSim: Processo %d está sendo executado\n", i);
                sleep(1);
                shm->processo[i] = 2;  // Volta a ficar READY
            }
        }
    */
}

//função main (onde os processos devem ser criados) 
int main() {
    int shm_id;
    MemoriaCompartilhada *shm;

    //cria a memória compartilhada
    shm_id = shmget(SHM_KEY, sizeof(MemoriaCompartilhada), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Falha ao criar memória compartilhada\n");
        exit(1);
    }

    //anexa a memória compartilhada
    shm = (MemoriaCompartilhada *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("Falha ao anexar memória compartilhada\n");
        exit(1);
    }

    //inicializa a memória compartilhada
    shm->irq = 0;
    memset(shm->processo, 0, sizeof(shm->processo));

    //cria o processo kernelSim
    pid_t kernel_pid = fork();
    printf("KernelSim criado!\n");
    if (kernel_pid == 0) {
        kernelSim(shm);  //executa o kernelSim
        exit(0);
    }

    //cria o processo interControllerSim
    pid_t inter_pid = fork();
    printf("InterControllerSim criado!\n");
    if (inter_pid == 0) {
        interControllerSim(shm);  //executa o interControllerSim
        exit(0);
    }

    //espera pelos processos kernelSim e interControllerSim
    wait(NULL);
    wait(NULL);

    //desanexa a memória compartilhada
    shmdt(shm);

    //remove a memória compartilhada
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

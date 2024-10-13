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
#define TIMESLICE 500000 //time slide = 500 ms.
#define MAX 1000

//Memória Compartilhada

#define SHM_KEY 0x1234   //chave para memória compartilhada

//Visualizando os estados dos processos

//estrutura para representar o contexto de um processo de aplicação
typedef struct {
    pid_t pid;
    int pc; //Program Counter (contador de instruções)
    int estado; //0: executando, 1: bloqueado ou pausado, 2: finalizado
    int dispositivo; //dispositivo associado, se bloqueado
    char operacao; //operação (R, W, X) realizada
    int acessos[2]; //contador de acessos para dispositivos D1 e D2
} Processo;

//estrutura que será armazenada na memória compartilhada
typedef struct {
    Processo processo[NUM_PROCESSOS];  //estado dos processos filhos
    int irq;  //indica a interrupção gerada pelo controlador (0: IRQ0, 1: IRQ1, 2: IRQ2)
    char op; //operação de uma igerada interrupção pelo controlador (read: R, write: W)
} MemoriaCompartilhada;

//função auxiliar

void ajusta_prontos(Processo* prontos) {
    prontos[0] = prontos[1];
    prontos[1] = prontos[2];
    //esvazia dados do último da lista...
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
        }
    }
}

void ajusta_em_espera(Processo *em_espera) {
    em_espera[0] = em_espera[1];
    em_espera[1] = em_espera[2];
    //esvazia dados do último da lista...
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
        }
    }
}

//função que simula o controlador de interrupções
void interControllerSim(MemoriaCompartilhada *shm) {
    srand(time(NULL));
    while (1) {
        // Gera IRQ0 a cada 500ms (time slice)
        usleep(TIMESLICE);  // 500 ms para representar o time slice
        shm->irq = 0;    // IRQ0 indica interrupção de relógio
        printf("InterControllerSim: Gerando IRQ0 (Time slice)\n");

        // Gera IRQ1 com probabilidade de 10%
        int irq_type = rand() % 100;  // Gera um número aleatório entre 0 e 99
        if (irq_type < 10) {  // 10% de chance (0 a 9)
            shm->irq = 1;  // IRQ1 para dispositivo D1
            shm->op = 'W'; // Define operação como escrita (W)
            printf("InterControllerSim: Gerando IRQ1 (Dispositivo D1) - Operação: Write\n");
        }

        // Gera IRQ2 com probabilidade de 5%
        irq_type = rand() % 100;  // Gera outro número aleatório entre 0 e 99
        if (irq_type < 5) {  // 5% de chance (0 a 4)
            shm->irq = 2;  // IRQ2 para dispositivo D2
            shm->op = 'R'; // Define operação como leitura (R)
            printf("InterControllerSim: Gerando IRQ2 (Dispositivo D2) - Operação: Read\n");
        }
    }
}


// Função do kernelSim (gerencia os filhos)
void kernelSim(MemoriaCompartilhada *shm) {
    pid_t pids[NUM_PROCESSOS];
    
    //cria processos filhos

    //P1
    pids[0] = fork();

    if (pids[0] < 0) {
    perror("Erro ao criar processo P1");
    exit(1);
    }
    if (pids[0] == 0) { //bloco de código de P1.
        printf("P1 criado.\n"); 
        if (execl("./p1", "p1", (char *)NULL) == -1) {
            perror("Erro ao executar p1");
            exit(1);
        }
        shm->processo[0].pid = getpid();
        shm->processo[0].pc = 0;
        shm->processo[0].estado = 0;
        shm->processo[0].dispositivo = 0; //0, pois nenhum dispositivo está acessando o processo.
        shm->processo[0].operacao = 'X'; //X, pois nenhuma operação está sendo realizada
        shm->processo[0].acessos[0] = 0; //0, pois nenhum dispositivo acessou o processo ainda.
        shm->processo[0].acessos[1] = 0;
        exit(0);
    }
    kill(pids[0], SIGSTOP);
    shm->processo[0].estado = 1;


    //P2
    pids[1] = fork();
    if (pids[1] < 0) {
    perror("Erro ao criar processo P2");
    exit(1);
    }
    if (pids[1] == 0) { //bloco de código de P2.
        printf("P2 criado.\n");
        if (execl("./p1", "p1", (char *)NULL) == -1) {
            perror("Erro ao executar p1");
            exit(1);
        }
        shm->processo[1].pid = pids[1];
        shm->processo[1].pc = 0;
        shm->processo[1].estado = 0;
        shm->processo[1].dispositivo = 0;
        shm->processo[1].operacao = 'X';
        shm->processo[1].acessos[0] = 0;
        shm->processo[1].acessos[1] = 0;
        exit(0);
    }
    kill(pids[1], SIGSTOP);
    shm->processo[1].estado = 1;


    //P3
    pids[2] = fork();
    if (pids[2] < 0) {
    perror("Erro ao criar processo P3");
    exit(1);
    }
    if (pids[2] == 0) { //bloco de código de P3.
        printf("P3 criado.\n");
        if (execl("./p3", "p3", (char *)NULL) == -1) {
            perror("Erro ao executar p3.");
            exit(1);
        }
        shm->processo[2].pid = pids[2];
        shm->processo[2].pc = 0;
        shm->processo[2].estado = 0;
        shm->processo[2].dispositivo = 0;
        shm->processo[2].operacao = 'X'; 
        shm->processo[2].acessos[0] = 0;
        shm->processo[2].acessos[1] = 0;
        exit(0);
    }
    kill(pids[2], SIGSTOP);
    shm->processo[2].estado = 1;


    Processo prontos[NUM_PROCESSOS]; //array com processos prontos, que podem ser executados em seguida.
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        prontos[i].pid = shm->processo[i].pid;
        printf("O pid de P%d é %d.\n", i+1, prontos[i].pid);
    }

    Processo em_espera[NUM_PROCESSOS]; //array de processos em espera, que foram interrompidos e estão aguardando resposta. começa vazia, só é preenchida em caso de interrupções.
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        shm->processo[i].pid = -1;
        shm->processo[i].pc = 0;
        shm->processo[i].estado = 1;
        shm->processo[i].dispositivo = 0;
        shm->processo[i].operacao = 'X'; 
        shm->processo[i].acessos[0] = 0;
        shm->processo[i].acessos[1] = 0;
    }

    kill(shm->processo[0].pid, SIGCONT);
    shm->processo[0].pc++;
    printf("KernelSim: Processo %d executando. PC: %d.\n", shm->processo[0].pid, shm->processo[0].pc);
    shm->processo[0].estado = 0;
    ajusta_prontos(prontos);

    while (shm->processo[0].pc < MAX || shm->processo[1].pc < MAX || shm->processo[2].pc < MAX) {
        for (int i = 0; i < NUM_PROCESSOS; i++) {
            if(shm->processo[i].estado == 0) { //descobre qual processo está executando.
                if (shm->irq == 0) { //IRQ0
                    kill(shm->processo[i].pid, SIGSTOP);
                    shm->processo[i].estado = 1; //atualizar contexto do processo
                    printf("KernelSim: Processo %d foi adicionado na fila PRONTOS\n", shm->processo[i].pid);
                    adiciona_prontos(prontos, shm->processo[i]); //adiciona no fim do array de prontos.
                }
                if (shm->irq == 1) { //IRQ1
                    kill(shm->processo[i].pid, SIGSTOP);
                    //guardar contexto do processo
                    shm->processo[0].estado = 1;
                    shm->processo[0].dispositivo = 1; //D1 fez o acesso.
                    shm->processo[0].operacao = shm->op;
                    shm->processo[0].acessos[0] += 1; //adiciona 1 ao número de acessos pelo D1.
                    adiciona_em_espera(em_espera, shm->processo[i]); //add no ultimo lugar desocupado do array de espera.
                    sleep(1); //D1 demora 1 segundo para dar resposta.
                    ajusta_em_espera(em_espera);
                    printf("KernelSim: Processo %d foi adicionado na fila PRONTOS\n", shm->processo[i].pid);
                    adiciona_prontos(prontos, shm->processo[i]);
                }
                if (shm->irq == 2) { //IRQ2
                    kill(shm->processo[i].pid, SIGSTOP);
                    //guardar contexto do processo
                    shm->processo[0].estado = 1;
                    shm->processo[0].dispositivo = 2; //D2 fez o acesso.
                    shm->processo[0].operacao = shm->op;
                    shm->processo[0].acessos[1] += 1; //adiciona 1 ao número de acessos pelo D2.
                    adiciona_em_espera(em_espera, shm->processo[i]);
                    sleep(2); //D2 demora 2 segundos para dar resposta.
                    ajusta_em_espera(em_espera);
                    printf("KernelSim: Processo %d foi adicionado na fila PRONTOS\n", shm->processo[i].pid);
                    adiciona_prontos(prontos, shm->processo[i]);
                }

                for (int j = 0; j < NUM_PROCESSOS; j++) {
                    if (shm->processo[j].pid == prontos[0].pid) { //executa o primeiro processo da fila de prontos. 
                        kill(shm->processo[j].pid, SIGCONT);
                        shm->processo[j].pc++;
                        printf("KernelSim: Processo %d executando. PC: %d.\n", shm->processo[j].pid, shm->processo[j].pc);
                        shm->processo[j].estado = 0; //muda o estado do processo para executando!
                        ajusta_prontos(prontos); //tira processo da fila de prontos e ajusta o array...
                    }
                }
            }
        }
    }

    kill(shm->processo[0].pid, SIGKILL);
    kill(shm->processo[1].pid, SIGKILL);
    kill(shm->processo[2].pid, SIGKILL);
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
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        shm->processo[i].estado = 1;
        shm->processo[i].pc = 0;
        memset(shm->processo[i].acessos, 0, sizeof(shm->processo[i].acessos));  //zera os acessos
    }

    //cria o processo kernelSim
    pid_t kernel_pid = fork();

    if (kernel_pid == 0) {
        printf("KernelSim criado!\n");
        kernelSim(shm);  //executa o kernelSim
        exit(0);
    }

    //cria o processo interControllerSim
    pid_t inter_pid = fork();

    if (inter_pid == 0) {
        printf("InterControllerSim criado!\n");
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

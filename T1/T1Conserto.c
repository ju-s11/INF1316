#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define NUM_PROCESSOS 1  //número de processos que o kernel irá lidar.
#define TIMESLICE 500000 //time slice = 500 ms.
#define PROB_1 10 //probabilidade de IRQ1 de 10% de chance (0 a 9)
#define PROB_2 5 //probabilidade de IRQ2 de 5% de chance (0 a 4)
#define MAX 10 // Defina um valor máximo de iterações

#define EXECUTANDO 0
#define BLOQUEADO 1
#define FINALIZADO 2

#define READ 'R'
#define WRITE 'W'
#define EXECUTE 'X'



//estrutura para representar o contexto de um processo de aplicação
typedef struct {
    pid_t pid;
    int pc; //Program Counter (contador de instruções)
    int estado; //0: executando, 1: bloqueado ou pausado, 2: finalizado
    int dispositivo; //dispositivo associado, se bloqueado
    char operacao; //operação (R, W, X) realizada
     int acessos[2]; // array para contar acessos a dispositivos
} Processo;

Processo processo[NUM_PROCESSOS]; //armazena os processos filhos
Processo prontos[NUM_PROCESSOS]; //armazena os processos prontos
Processo em_espera[NUM_PROCESSOS]; //armazena os processos na fila de espera

char op;
int irq;


//função auxiliar de manipulação das filas "prontos" e "em_espera"

//inicializar as filas de prontos e em espera
void inicializa_filas() {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        prontos[i].pid = -1; // Inicializa PID como -1 para indicar que está vazio
        em_espera[i].pid = -1; // Inicializa PID como -1 para indicar que está vazio
    }
}

// prontos:
void adiciona_prontos(Processo *prontos, Processo processo) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (prontos[i].pid == -1) {
            prontos[i] = processo;
            printf("Processo adicionado à fila de prontos.\n");
            return;
        }
    }
   printf("Fila de prontos cheia.\n");
}

void ajusta_prontos(Processo *prontos) {
    for (int i = 0; i < NUM_PROCESSOS - 1; i++) {
        prontos[i] = prontos[i + 1]; // Move todos os processos para frente
    }
    // Esvazia o último da lista:
    prontos[NUM_PROCESSOS - 1].pid = -1;
    prontos[NUM_PROCESSOS - 1].pc = 0;
    prontos[NUM_PROCESSOS - 1].estado = 1;
    prontos[NUM_PROCESSOS - 1].dispositivo = 0;
    prontos[NUM_PROCESSOS - 1].operacao = 0; 
    prontos[NUM_PROCESSOS - 1].acessos[0] = 0;
    prontos[NUM_PROCESSOS - 1].acessos[1] = 0;
    printf("Lista de prontos ajustada.\n");
}

// em espera:
void adiciona_em_espera(Processo *em_espera, Processo processo) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (em_espera[i].pid == -1) {
            em_espera[i] = processo;
           printf("Processo adicionado à fila de espera.\n");
           return;
        }
    }
    printf("Fila de espera cheia.\n");
}

void ajusta_em_espera(Processo *em_espera) {
    for (int i = 0; i < NUM_PROCESSOS - 1; i++) {
        em_espera[i] = em_espera[i + 1]; // Move todos os processos para frente
    }
    // Esvazia o último da lista:
    em_espera[NUM_PROCESSOS - 1].pid = -1;
    em_espera[NUM_PROCESSOS - 1].pc = 0;
    em_espera[NUM_PROCESSOS - 1].estado = 1;
    em_espera[NUM_PROCESSOS - 1].dispositivo = 0;
    em_espera[NUM_PROCESSOS - 1].operacao = 0; 
    em_espera[NUM_PROCESSOS - 1].acessos[0] = 0;
    em_espera[NUM_PROCESSOS - 1].acessos[1] = 0;
    printf("Lista de espera ajustada.\n");
}

// Funções de Solicitação de Acesso:

//d1
void solicitar_acesso_d1(Processo *proc) {
    proc->dispositivo = 1; // Espera pelo D1
    proc->estado = 1; // Muda para estado bloqueado
    // Adiciona o processo à fila de bloqueados
    adiciona_em_espera(em_espera, *proc); // Adiciona o processo à fila de espera
   printf("Processo bloqueado esperando por D1\n");
}

//d2
void solicitar_acesso_d2(Processo *proc) {
    proc->dispositivo = 2; // Espera pelo D2
    proc->estado = 1; // Muda para estado bloqueado
    // Adiciona o processo à fila de bloqueados
    adiciona_em_espera(em_espera, *proc); // Adiciona o processo à fila de espera
    printf("Processo bloqueado esperando por D2\n");
}


//funções para tratamento de sinais
//IRQ0
void handle_sigalrm(int signum) {
    printf("IRQ0: SIGALRM recebido!\n");
    for (int i = 0; i < NUM_PROCESSOS; i++) 
    { 
        kill(processo[i].pid, SIGSTOP);
        processo[i].estado = 1; //atualizar contexto do processo
        printf("KernelSim: Processo %d foi adicionado na fila PRONTOS\n", processo[i].pid);
        adiciona_prontos(prontos, processo[i]); //adiciona no fim do array de prontos.
    }
}

//IRQ1
void handle_sigusr1(int signum) {
    printf("IRQ1: SIGUSR1 recebido - interrupção do dispositivo D1\n");
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (processo[i].dispositivo == 1)  
        { // Verifica se o processo está aguardando D1
            kill(processo[i].pid, SIGSTOP);
            processo[i].estado = 1;
            processo[i].dispositivo = 1; //D1 fez o acesso.
            processo[i].operacao = op;
            processo[i].acessos[0] += 1; //adiciona 1 ao número de acessos pelo D1.
            adiciona_em_espera(em_espera, processo[i]); //add no ultimo lugar desocupado do array de espera.
            sleep(1); //D1 demora 1 segundo para dar resposta.
            ajusta_em_espera(em_espera);
            printf("KernelSim: Processo %d foi adicionado na fila PRONTOS\n", processo[i].pid);
            adiciona_prontos(prontos, processo[i]);
        }
    }
}

//IRQ2
void handle_sigusr2(int signum) {
    printf("IRQ2: SIGUSR2 recebido - interrupção do dispositivo D2\n");
    for (int i = 0; i < NUM_PROCESSOS; i++) { 
        kill(processo[i].pid, SIGSTOP); // Para o processo atual
        //guardar contexto do processo
        processo[i].estado = 1;
        processo[i].dispositivo = 2; //D2 fez o acesso.
        processo[i].operacao = op;
        processo[i].acessos[1] += 1; //adiciona 1 ao número de acessos pelo D2.
        adiciona_em_espera(em_espera, processo[i]);
        sleep(2); //D2 demora 2 segundos para dar resposta.
        printf("KernelSim: Processo %d foi adicionado na fila PRONTOS\n", processo[i].pid);
        adiciona_prontos(prontos, processo[i]);
    }
}

// Inicializando a estrutura
void inicializa_processo() {
    processo[0].pid = getpid();   // Atribui o PID do processo atual
    processo[0].pc = 0;           // Inicializa o contador de programa
    processo[0].estado = EXECUTANDO;  // Estado inicial: EXECUTANDO
    processo[0].dispositivo = 0;   // Nenhum dispositivo associado inicialmente
    processo[0].operacao = READ;   // Operação inicial: READ (leitura)
    processo[0].acessos[0] = 0;    // Acessos ao dispositivo D1
    processo[0].acessos[1] = 0;    // Acessos ao dispositivo D2
}


//função para executar o programa certo!
void executa_processo(int n) {
    if (n <= 1 || n > 6) {
        printf("Número inválido: %d. Escolha um valor maior que 1 e menor ou igual a 6.\n", n);
        return;
    }

    inicializa_processo();  // Inicializa o processo antes de executar

    //constrói o nome do executável baseado no número n
    char executavel[10];
    snprintf(executavel, sizeof(executavel), "./p%d", n);  // Cria "./p2", "./p3", etc.

    //executa o executável pn usando execl()
    execl(executavel, executavel, (char *)NULL);
    
    //se execl falhar, exibe uma mensagem de erro
    perror("Erro ao executar o programa");
    exit(1);  //sai com erro se a execução falhar
}

char determina_operacao(int tipo_operacao) {
    if (tipo_operacao == 0) {
        return READ;  // Operação de leitura
    } else if (tipo_operacao == 1) {
        return WRITE;  // Operação de escrita
    } else if (tipo_operacao == 2) {
        return EXECUTE;  // Operação de execução
    }
    return 0;  // Retorna 0 ou um valor padrão se o tipo não for válido
}


//função do controlador de interrupções (InterControllerSim)
void interControllerSim() {
    srand(time(NULL));

    while (1) {
        usleep(TIMESLICE); //500 ms para representar o time slice

        // Gerar IRQ0 (time slice)
        printf("InterControllerSim: Gerando IRQ0 (Time slice)\n");
        kill(getppid(), SIGALRM);  //envia SIGALRM para o kernelSim (pai)

        //gera IRQ1 com probabilidade de 10%
        int tipo_irq = rand() % 100;
        if (tipo_irq < PROB_1) {
            printf("InterControllerSim: Gerando IRQ1 (Dispositivo D1)\n");
            kill(getppid(), SIGUSR1);  //envia SIGUSR1 para o kernelSim (pai)
            // Tipo de operação no dispositivo: leitura (R), escrita (W) ou execução (X)
            int tipo_operacao = rand() % 3; // 0, 1 ou 2
            op = determina_operacao(tipo_operacao); // Chama a função para determinar a operação
        }

        //gera IRQ2 com probabilidade de 5%
        tipo_irq = rand() % 100;
        if (tipo_irq < PROB_2) {
            printf("InterControllerSim: Gerando IRQ2 (Dispositivo D2)\n");
            kill(getppid(), SIGUSR2);  //envia SIGUSR2 para o kernelSim (pai)
            // Tipo de operação no dispositivo: leitura (R), escrita (W) ou execução (X)
            int tipo_operacao = rand() % 3;
            op = determina_operacao(tipo_operacao); // Chama a função para determinar a operação
        }
    }
}

//função do kernelSim (gerencia os filhos)
void kernelSim() {
    pid_t pids[NUM_PROCESSOS];

    //configura os tratadores de sinal
    signal(SIGALRM, handle_sigalrm);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    //cria processos filhos
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {  //código do processo filho
            printf("P%d criado com PID %d.\n", (i + 1), getpid());
            executa_processo(i);
        } else if (pids[i] > 0) {  //código do processo pai (kernelSim)
            processo[i].pid = pids[i];
            processo[i].pc = 0;
            processo[i].estado = 1;  //processo pausado inicialmente
            processo[i].dispositivo = 0; //0, pois nenhum dispositivo está acessando o processo.
            processo[i].operacao = 0; //0, pois nenhuma operação está sendo realizada
            processo[i].acessos[0] = 0; //0, pois nenhum dispositivo acessou o processo ainda.
            processo[i].acessos[1] = 0;
            kill(pids[i], SIGSTOP);  //pausa o processo filho após a criação.
            printf("Processo P%d (PID %d) está no estado PRONTO.\n", (i + 1), pids[i]);
        } else {
            perror("Erro ao criar processo filho");
            exit(1);
        }
    }

    Processo prontos[NUM_PROCESSOS]; //array com processos prontos, que podem ser executados em seguida.
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        prontos[i] = processo[i];
        printf("O pid de P%d é %d NA FILA DE PRONTOS.\n", i+1, prontos[i].pid);
    }

    Processo em_espera[NUM_PROCESSOS]; //array de processos em espera, que foram interrompidos e estão aguardando resposta. começa vazia, só é preenchida em caso de interrupções.
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        processo[i].pid = -1;
        processo[i].pc = 0;
        processo[i].estado = 1;
        processo[i].dispositivo = 0;
        processo[i].operacao = 0; 
        processo[i].acessos[0] = 0;
        processo[i].acessos[1] = 0;
        printf("FILA DE EM ESPERA criada!\n");
    }

    while (processo[0].pc < MAX || processo[1].pc < MAX || processo[2].pc < MAX) {
        for (int i = 0; i < NUM_PROCESSOS; i++) {
           
            for (int j = 0; j < NUM_PROCESSOS; j++) {
                if (processo[j].pid == prontos[0].pid) { //executa o primeiro processo da fila de prontos.
                    kill(processo[j].pid, SIGCONT);
                    processo[j].pc++;
                    printf("KernelSim: Processo %d executando. PC: %d.\n", processo[j].pid, processo[j].pc);
                    processo[j].estado = 0; //muda o estado do processo para executando!
                    ajusta_prontos(prontos); //tira processo da fila de prontos e ajusta o array...
                }
            }
        }
    }


}

//função main (onde os processos são criados)
int main() {
    //cria o processo kernelSim
    pid_t kernel_pid = fork();

    if (kernel_pid == 0) {
        printf("KernelSim criado!\n");
        kernelSim();  //executa o kernelSim
        exit(0);
    }

    //cria o processo interControllerSim
    pid_t inter_pid = fork();

    if (inter_pid == 0) {
        printf("InterControllerSim criado!\n");
        interControllerSim();  //executa o interControllerSim
        exit(0);
    }

    //espera pelos processos kernelSim e interControllerSim
    wait(NULL);
    wait(NULL);

    return 0;
}

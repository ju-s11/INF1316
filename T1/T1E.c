#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSOS 5

typedef struct {
    int operacao;
    pid_t pid;
    int estado; // 0 para executando, 1 para em espera
} Processo;

// Filas para processos prontos e em espera
Processo prontos[NUM_PROCESSOS];
Processo em_espera[NUM_PROCESSOS];

// Inicialização das variáveis globais
int cont_prontos = 0;
int cont_em_espera = 0;
Processo processos[NUM_PROCESSOS]; // Variável global para processos

// Funções para gerenciar as filas
void ajusta_prontos(Processo p) {
    if (cont_prontos < NUM_PROCESSOS) {
        prontos[cont_prontos++] = p;
    }
}

void adiciona_prontos(Processo p) {
    if (cont_prontos < NUM_PROCESSOS) {
        prontos[cont_prontos++] = p;
    }
}

void ajusta_em_espera(Processo p) {
    if (cont_em_espera < NUM_PROCESSOS) {
        em_espera[cont_em_espera++] = p;
    }
}

void adiciona_em_espera(Processo p) {
    if (cont_em_espera < NUM_PROCESSOS) {
        em_espera[cont_em_espera++] = p;
    }
}

// Manipuladores de sinal
void handle_sigalrm(int sig) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (processos[i].estado == 0) { // Se está executando
            adiciona_prontos(processos[i]); // Adiciona ao fim da fila de prontos
            processos[i].estado = 1; // Muda o estado para em espera
        }
    }
}

void handle_sigusr1(int sig) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (processos[i].estado == 1) { // Se está em espera
            adiciona_em_espera(processos[i]); // Adiciona à fila de espera
            processos[i].estado = 0; // Muda o estado para pronto
        }
    }
}

void handle_sigusr2(int sig) {
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        if (processos[i].estado == 0) { // Se está pronto
            adiciona_prontos(processos[i]); // Adiciona ao fim da fila de prontos
            processos[i].estado = 1; // Muda o estado para em espera
        }
    }
}

// Função para executar processos
void executa_processo(int id) {
    // Simula a execução do processo
    printf("Processo %d está executando...\n", id);
    sleep(1); // Simula tempo de execução
}

// Função principal
int main() {
    // Cria os processos filhos
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Processo filho
            executa_processo(i + 1);
            exit(0); // Termina após executar o programa
        } else {
            processos[i].pid = pid; // Armazena o PID do processo filho
            processos[i].operacao = 0; // Inicializa operação
            processos[i].estado = 0; // Estado: executando
            printf("KernelSim: Criou o processo %d com PID %d\n", i + 1, pid);
        }
    }

    // Configura os manipuladores de sinal
    signal(SIGALRM, handle_sigalrm);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    // Aguarda os processos filhos terminarem
    for (int i = 0; i < NUM_PROCESSOS; i++) {
        wait(NULL); // Espera o término do processo filho
    }

    return 0;
}

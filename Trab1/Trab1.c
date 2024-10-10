#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

// Definições de constantes
#define NUM_PROCESSES 3
#define TIME_SLICE 1  // Em segundos

// Protótipos
void start_kernel();
void start_intercontroller();
void process_application(int id);
void handle_sigusr1(int sig);
void handle_sigusr2(int sig);

// Variáveis globais
int app_pids[NUM_PROCESSES];  // Armazena os PIDs dos processos de aplicação

int main() {
    // Inicializa o KernelSim e InterController Sim
    pid_t kernel_pid, intercontroller_pid;

    kernel_pid = fork();
    if (kernel_pid == 0) {
        start_kernel();  // KernelSim
        exit(0);
    }

    intercontroller_pid = fork();
    if (intercontroller_pid == 0) {
        start_intercontroller();  // InterController Sim
        exit(0);
    }

    // Espera a finalização dos processos
    wait(NULL);
    wait(NULL);

    return 0;
}

// Função que simula o KernelSim
void start_kernel() {
    printf("KernelSim iniciado!\n");

    for (int i = 0; i < NUM_PROCESSES; i++) {
        app_pids[i] = fork();
        if (app_pids[i] == 0) {
            process_application(i + 1);  // Processo de aplicação
            exit(0);
        }
    }

    // Exemplo de escalonamento básico
    int current_process = 0;
    while (1) {
        printf("Kernel: Executando processo %d\n", current_process + 1);
        kill(app_pids[current_process], SIGCONT);  // Retoma o processo

        sleep(TIME_SLICE);  // Simula o time slice

        kill(app_pids[current_process], SIGSTOP);  // Pausa o processo
        current_process = (current_process + 1) % NUM_PROCESSES;  // Escalonamento circular
    }
}

// Função que simula o InterController Sim
void start_intercontroller() {
    printf("InterController Sim iniciado!\n");
    while (1) {
        sleep(1);  // A cada 1 segundo, gera interrupções (simulando o IRQ0)
        printf("InterController: Gerando interrupção (IRQ0)\n");
        kill(getppid(), SIGUSR1);  // Envia sinal para o KernelSim
    }
}

// Função que simula um processo de aplicação
void process_application(int id) {
    signal(SIGCONT, handle_sigusr1);  // Define o sinal para retomar
    signal(SIGSTOP, handle_sigusr2);  // Define o sinal para pausar

    int pc = 0;  // Contador de programa

    while (1) {
        printf("Processo %d: Executando, PC = %d\n", id, pc);
        pc++;
        sleep(1);  // Simula o tempo de execução de uma instrução
    }
}

// Funções para tratar sinais
void handle_sigusr1(int sig) {
    // Continua a execução
    printf("Processo recebeu SIGCONT, retomando execução...\n");
}

void handle_sigusr2(int sig) {
    // Pausa a execução
    printf("Processo recebeu SIGSTOP, pausando...\n");
    pause();  // Aguarda o próximo SIGCONT
}
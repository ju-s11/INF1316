#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

// Estrutura para manter o estado do processo (PC, syscall, etc.)
typedef struct {
    int pid;
    int PC;  // Program Counter
    int estado;  // 0 - Executando, 1 - Bloqueado, 2 - Finalizado
    int syscall_espera;  // Indica se há uma syscall pendente (D1 ou D2)
} Processo;

Processo processos[5];  // Array de processos (A1, A2, A3, ...)

// Funções para lidar com interrupções
void irq0_handler(int sig);
void irq1_handler(int sig);
void irq2_handler(int sig);

void criar_processos();  // Função para criar os processos de aplicação
void controlador_interrupcoes();  // Simula o controlador de interrupções
void kernel_sim();  // Simulador do Kernel

int main() {
    criar_processos();
    kernel_sim();
    return 0;
}

void criar_processos() {
    // Criação dos processos de aplicação (A1, A2, ...)
    for (int i = 0; i < 3; i++) {
        int pid = fork();
        if (pid == 0) {
            // Processo filho executa código da aplicação
            while (1) {
                printf("Processo %d executando com PC = %d\n", getpid(), processos[i].PC);
                processos[i].PC++;
                sleep(1);  // Simula tempo de execução
            }
        } else {
            processos[i].pid = pid;
            processos[i].PC = 0;
            processos[i].estado = 0;  // Executando
        }
    }
}

void controlador_interrupcoes() {
    // Este processo deve gerar interrupções periodicamente (IRQ0, IRQ1, IRQ2)
    while (1) {
        sleep(1);  // Tempo de 1 segundo entre interrupções
        kill(getppid(), SIGUSR1);  // Envia interrupção ao kernel
    }
}

void kernel_sim() {
    signal(SIGUSR1, irq0_handler);  // Interrupção para troca de contexto
    signal(SIGUSR2, irq1_handler);  // Interrupção de dispositivo D1
    signal(SIGTERM, irq2_handler);  // Interrupção de dispositivo D2

    while (1) {
        // Simulação do kernel, gerenciando processos
        pause();  // Espera uma interrupção
    }
}

// Manipuladores de interrupções
void irq0_handler(int sig) {
    printf("IRQ0: Troca de contexto\n");
    // Lógica de troca de contexto
}

void irq1_handler(int sig) {
    printf("IRQ1: Dispositivo D1 finalizou\n");
    // Lógica de liberação de processos bloqueados
}

void irq2_handler(int sig) {
    printf("IRQ2: Dispositivo D2 finalizou\n");
    // Lógica de liberação de processos bloqueados
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define BUFFER_SIZE 100

struct sembuf sem_op;

int main() {
    int shmid, semid;
    char *memoria_compartilhada;
    char buffer[BUFFER_SIZE];

    // Criar memória compartilhada
    shmid = shmget(SHM_KEY, BUFFER_SIZE, 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("Erro ao criar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    // Associar a memória compartilhada
    memoria_compartilhada = (char *) shmat(shmid, NULL, 0);
    if (memoria_compartilhada == (char *) -1) {
        perror("Erro ao associar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    // Criar semáforo
    semid = semget(SEM_KEY, 1, 0644 | IPC_CREAT);
    if (semid == -1) {
        perror("Erro ao criar semáforo");
        exit(EXIT_FAILURE);
    }

    // Inicializar o semáforo
    semctl(semid, 0, SETVAL, 1);

    while (1) {
        printf("Digite uma mensagem: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strlen(buffer) - 1] = '\0'; // Remove o newline

        // Esperar pelo semáforo
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;  // Down (P)
        sem_op.sem_flg = 0;
        semop(semid, &sem_op, 1);

        // Escrever na memória compartilhada
        strncpy(memoria_compartilhada, buffer, BUFFER_SIZE);

        // Liberar o semáforo
        sem_op.sem_op = 1;   // Up (V)
        semop(semid, &sem_op, 1);
    }

    // Desanexar memória compartilhada
    shmdt(memoria_compartilhada);

    return 0;
}

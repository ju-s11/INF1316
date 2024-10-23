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

    // Associar a memória compartilhada
    shmid = shmget(SHM_KEY, BUFFER_SIZE, 0644);
    if (shmid == -1) {
        perror("Erro ao acessar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    memoria_compartilhada = (char *) shmat(shmid, NULL, 0);
    if (memoria_compartilhada == (char *) -1) {
        perror("Erro ao associar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    // Acessar o semáforo
    semid = semget(SEM_KEY, 1, 0644);
    if (semid == -1) {
        perror("Erro ao acessar semáforo");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Esperar pelo semáforo
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;  // Down (P)
        sem_op.sem_flg = 0;
        semop(semid, &sem_op, 1);

        // Verificar se a mensagem não está vazia
        if (strlen(memoria_compartilhada) > 0) {
            printf("Mensagem recebida: %s\n", memoria_compartilhada);
            // Limpar a memória após leitura
            memset(memoria_compartilhada, 0, BUFFER_SIZE);
        }

        // Liberar o semáforo
        sem_op.sem_op = 1;   // Up (V)
        semop(semid, &sem_op, 1);

        sleep(1); // Evitar loop acelerado
    }

    // Desanexar memória compartilhada
    shmdt(memoria_compartilhada);

    return 0;
}

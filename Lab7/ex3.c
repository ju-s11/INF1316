#include <sys/sem.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

union semun { 
  int val; 
  struct semid_ds *buf; 
  ushort *array; 
}; 

// Inicializa o valor do semáforo 
int setSemValue(int semId, int value); 
// Remove o semáforo 
void delSemValue(int semId); 
// Operação P 
int semaforoP(int semId); 
// Operação V 
int semaforoV(int semId);

int *variavel_compartilhada;

int main() { 
  int semId, shmid; 
  pid_t pid1, pid2;

  // Configuração da memória compartilhada
  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("Erro ao criar memória compartilhada");
    exit(1);
  }
  variavel_compartilhada = (int*) shmat(shmid, NULL, 0);
  if (variavel_compartilhada == (int*) -1) {
    perror("Erro ao associar memória compartilhada");
    exit(1);
  }
  *variavel_compartilhada = 0; // Inicializa variável compartilhada com 0

  // Criação do semáforo
  semId = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
  if (semId == -1) {
    perror("Erro ao criar semáforo");
    exit(1);
  }
  setSemValue(semId, 1); // Inicializa o semáforo com valor 1 para permitir que o primeiro processo comece

  // Criar processos filho
  pid1 = fork();
  if (pid1 == 0) {
    // Processo produtor (soma 1)
    for (int i = 0; i < 10; i++) {
      semaforoP(semId); // Espera a liberação do semáforo
      *variavel_compartilhada += 1;
      printf("Produtor: Somou 1, valor atual: %d\n", *variavel_compartilhada);
      semaforoV(semId); // Libera o semáforo para o consumidor
      sleep(rand() % 2);
    }
    shmdt(variavel_compartilhada);
    exit(0);
  }

  pid2 = fork();
  if (pid2 == 0) {
    // Processo consumidor (soma 5)
    for (int i = 0; i < 10; i++) {
      semaforoP(semId); // Espera a liberação do semáforo
      *variavel_compartilhada += 5;
      printf("Consumidor: Somou 5, valor atual: %d\n", *variavel_compartilhada);
      semaforoV(semId); // Libera o semáforo para o produtor
      sleep(rand() % 2);
    }
    shmdt(variavel_compartilhada);
    exit(0);
  }

  // Espera pelos processos filhos terminarem
  wait(NULL);
  wait(NULL);

  // Resultado final
  printf("Valor final: %d\n", *variavel_compartilhada);

  // Limpeza
  delSemValue(semId);
  shmdt(variavel_compartilhada);
  shmctl(shmid, IPC_RMID, 0);

  return 0; 
}

int setSemValue(int semId, int value) { 
  union semun semUnion; 
  semUnion.val = value; 
  return semctl(semId, 0, SETVAL, semUnion); 
} 

void delSemValue(int semId) { 
  union semun semUnion; 
  semctl(semId, 0, IPC_RMID, semUnion); 
} 

int semaforoP(int semId) {
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}

int semaforoV(int semId) {
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}

//thiago henriques 2211171
// exemplo inicial modificado para os processos produtor e consumidor
#include <sys/sem.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>

#define BUFFER_SIZE 16

// Buffer compartilhado
char buffer[BUFFER_SIZE];
int count = 0; // Contador de caracteres no buffer

union semun
{ 
  int val; 
  struct semid_ds *buf; 
  ushort *array; 
}; 
// inicializa o valor do semáforo 
int setSemValue(int semId); 
// remove o semáforo 
void delSemValue(int semId); 
// operação P 
int semaforoP(int semId, int semNum);
//operação V 
int semaforoV(int semId, int semNum);

int mutex;
int empty;
int full;

int main (int argc, char * argv[]) 
{ 
  printf("Processso %d iniciado\n", getpid());
  int i; 
  char letra;
//   char letra = 'o';
  int semId; 
    
  if (argc > 1) 
  { 
    semId = semget (8752, 3, 0666 | IPC_CREAT); // 3 semaforos
    setSemValue(semId); 
    // letra = 'x';
    sleep (2); 
  } else {
     while ((semId = semget(8752, 3, 0666)) < 0) {
            putchar('.'); fflush(stdout);
            sleep(1);
        }
    }
      // Processo produtor
      if (argc > 1) {
        while (1) {
            char input = getchar(); // Captura um caractere digitado pelo usuário

            semaforoP(semId, 1); // Decrementa empty (espera por espaço)
            semaforoP(semId, 0); // Decrementa mutex (entra na seção crítica)

            // Adiciona o caractere ao buffer
            buffer[count++] = input;
            printf("Produtor %d: %c (Count: %d)\n", getpid(), input, count);

            // Se o buffer estiver cheio, o consumidor será ativado
            if (count == BUFFER_SIZE) {
                semaforoV(semId, 2); // Incrementa full (sinaliza que está cheio)
                count = 0; // Reseta o contador
            }

            semaforoV(semId, 0); // Incrementa mutex (sai da seção crítica)
            semaforoV(semId, 1); // Incrementa empty (há espaço novamente)
        }
    }
  // Consumidor
  else 
  { 
     while (1) {
            semaforoP(semId, 2); // Decrementa full (espera buffer cheio)
            semaforoP(semId, 0); // Decrementa mutex (entra na seção crítica)

            // Imprime o conteúdo do buffer
            for (int i = 0; i < BUFFER_SIZE; i++) {
                putchar(buffer[i]);
            }
            printf("\n");

            semaforoV(semId, 0); // Incrementa mutex (sai da seção crítica)
            semaforoV(semId, 1); // Incrementa empty (buffer agora está vazio)
        }
    }
    return 0; 
  }

int setSemValue(int semId) 
{ 
  union semun semUnion; 
  semUnion.val = 1; 
  semctl(semId, 0, SETVAL, semUnion); // mutex
  semUnion.val = BUFFER_SIZE; 
  semctl(semId, 1, SETVAL, semUnion); // empty
  semUnion.val = 0; 
  semctl(semId, 2, SETVAL, semUnion); // full
  return 0; 
}
void delSemValue(int semId) 
{ 
  union semun semUnion; 
  semctl(semId, 0, IPC_RMID, semUnion); 
} 
int semaforoP(int semId, int semNum) {
    struct sembuf semB;
    semB.sem_num = semNum;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}
int semaforoV(int semId, int semNum) {
    struct sembuf semB;
    semB.sem_num = semNum;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}

// thiago henriques 2211171
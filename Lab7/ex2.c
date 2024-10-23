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
int semaforoP(int semId); 
//operação V 
int semaforoV(int semId);

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
      // Processo produtor
      for (int i = 0; i < 10; i++)
            {
                semaforoP(empty); // Espera por espaço vazio
                semaforoP(mutex); // Entra na seção crítica

                // Produz um novo caractere
                buffer[count++] = 'A' + (i % 26); // Produz um caractere
                printf("Produtor %d: %c (Count: %d)\n", getpid(), buffer[count - 1], count); // Imprime caractere produzido

                semaforoV(mutex); // Sai da seção crítica
                semaforoV(full);   // Incrementa o número de itens cheios
                sleep(rand() % 3); // Simula tempo de produção
            }

            delSemValue(semId); // Remove semáforos ao final
      }

  else 
  { 
      while ((semId = semget(8752, 3, 0666)) < 0) // Também muda para 3
    { 
    //   putchar ('.'); fflush(stdout); 
      sleep (1); 
  } 
}
  // Processo consumidor
  for (i=0; i<10; i++) 
  { 
    // semaforoP(semId); 
    semaforoP(full);  // Espera por itens cheios
    semaforoP(mutex); // Entra na seção crítica

      // Verifica se há algo no buffer antes de consumir
      if (count > 0) {
          char letra = buffer[--count]; // Consome o caractere
          printf("Consumidor %d: %c (Count: %d)\n", getpid(), letra, count); // Imprime caractere consumido
      }

      
    // putchar (toupper(letra)); fflush(stdout); 
    // sleep (rand() %3); 
    // putchar (letra); fflush(stdout); 
    // semaforoV(semId); 

    semaforoV(mutex); // Sai da seção crítica
    semaforoV(empty);  // Incrementa o número de espaços vazios
    sleep (rand() %2); 
  } 
  printf ("\nProcesso %d terminou\n", getpid()); 
  if (argc > 1) 
  { 
    sleep(1); // mudei para n precisar esperando 10 segundos pra destruir o semáforo sempre que for rodar 
    delSemValue(semId); 
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
int semaforoP(int semId) 
{ 
  struct sembuf semB; 
  semB.sem_num = 0; 
  semB.sem_op = -1; 
  semB.sem_flg = SEM_UNDO; 
  semop(semId, &semB, 1); 
  return 0; 
} 
int semaforoV(int semId) 
{ 
  struct sembuf semB; 
  semB.sem_num = 0; 
  semB.sem_op = 1; 
  semB.sem_flg = SEM_UNDO; 
  semop(semId, &semB, 1); 
  return 0; 
}

// thiago henriques 2211171
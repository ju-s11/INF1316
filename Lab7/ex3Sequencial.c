// exemplo inicial modificado para os processos produtor e consumidor
#include <sys/sem.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>

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

int *variavel_compartilhada;

int main (int argc, char * argv[]) 
{ 

  int i; 
  char letra = 'o';
  int semId, shmid; 

  // Criação de memória compartilhada
  
  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);

  printf("Processso %d iniciado com shmid: %d\n", getpid(), shmid);
  if (shmid == -1){
    perror("Erro ao criar memoria compartilhada");
  }

  variavel_compartilhada = (int*) shmat(shmid, NULL, 0); 
  *variavel_compartilhada = 0; // Inicializa variável compartilhada com 0

  if (argc > 1) 
  { 
    semId = semget (8752, 1, 0666 | IPC_CREAT); //  1 semáforo (mutex)
    setSemValue(semId); 
    letra = 'x';
    sleep (2); 
      // Processo que soma 1 à variável
      for (int i = 0; i < 4; i++)
            {
                semaforoP(semId); // Região crítica
                shmid = atoi(argv[1]);
                variavel_compartilhada = (int*) shmat(shmid, NULL, 0); 
                if (variavel_compartilhada == (int*) -1) {
                    perror("Erro ao associar memória compartilhada");
                    exit(1);
                }

                *variavel_compartilhada = *variavel_compartilhada + 1; 

                printf("Processo %d: Somou 1, valor atual: %d no endereço %p\n com shmid=%d", getpid(), *variavel_compartilhada, variavel_compartilhada, shmid);
                semaforoV(semId); // Fim da região crítica
                sleep(rand() % 3); // Simula tempo de produção
            }

            // Processo que soma 5 à variável
        for (i=0; i<4; i++) 
        { 
            semaforoP(semId); // Região crítica
            shmid = atoi(argv[1]);
            variavel_compartilhada = (int*) shmat(shmid, NULL, 0); 
            if (variavel_compartilhada == (int*) -1) {
                perror("Erro ao associar memória compartilhada");
                exit(1);
            }

            *variavel_compartilhada = *variavel_compartilhada + 5;

            printf("Processo %d: Somou 5, valor atual: %d no endereço %p\n com shmid=%d", getpid(), *variavel_compartilhada, variavel_compartilhada, shmid);
            semaforoV(semId); // Fim da região crítica
            sleep(rand() % 2); // Simula tempo de processamento

                    // delSemValue(semId); // Remove semáforos ao final
                    shmdt(variavel_compartilhada); // Desanexa a memória compartilhada
                    shmctl(shmid, IPC_RMID, NULL); // Remove a memória compartilhada
      }
            // Processo que soma 1 à variável
      for (int i = 0; i < 4; i++)
            {
                semaforoP(semId); // Região crítica
                shmid = atoi(argv[1]);
                variavel_compartilhada = (int*) shmat(shmid, NULL, 0); 
                if (variavel_compartilhada == (int*) -1) {
                    perror("Erro ao associar memória compartilhada");
                    exit(1);
                }

                *variavel_compartilhada = *variavel_compartilhada + 1; 

                printf("Processo %d: Somou 1, valor atual: %d no endereço %p\n com shmid=%d", getpid(), *variavel_compartilhada, variavel_compartilhada, shmid);
                semaforoV(semId); // Fim da região crítica
                sleep(rand() % 3); // Simula tempo de produção
            }
  }

  else 
  { 
      while ((semId = semget(8752, 1, 0666)) < 0) // Aguardando semáforo
    { 
      putchar ('.'); fflush(stdout); 
      sleep (1); 
  } 
  printf ("\nProcesso %d terminou\n", getpid()); 
  if (argc > 1) 
  { 
    sleep(1); // mudei para n precisar esperando 10 segundos pra destruir o semáforo sempre que for rodar 
    // delSemValue(semId); 
    } 
    return 0; 
  }
}

int setSemValue(int semId) 
{ 
  union semun semUnion; 
  semUnion.val = 1; 
  return semctl(semId, 0, SETVAL, semUnion); 
} 
void delSemValue(int semId) 
{ 
  union semun semUnion; 
  semctl(semId, 0, IPC_RMID, semUnion); 
} 
int semaforoP(int semId) {
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;
    
    printf("Processo %d tentando adquirir semáforo...\n", getpid());
    semop(semId, &semB, 1);
    printf("Processo %d adquiriu semáforo.\n", getpid());
    return 0;
}

int semaforoV(int semId) {
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;

    semop(semId, &semB, 1);
    printf("Processo %d liberou semáforo.\n", getpid());
    return 0;
}

// thiago henriques 2211171
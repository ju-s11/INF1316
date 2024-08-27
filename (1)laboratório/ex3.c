//Laboratório 1 - 20/08/2024 - Julia Simão e Thiago Henriques - Exercício 3

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SIZE 10

void bubble_sort(int arr[], int n) {
  int i, j, temp;
  for (i = 0; i < n - 1; i++) {
    for (j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

/*
void bubble_sort (int vetor[], int n) {
    int k, j, aux;

    for (k = 1; k < n; k++) {
        printf("\n[%d] ", k);

        for (j = 0; j < n - 1; j++) {
            printf("%d, ", j);

            if (vetor[j] > vetor[j + 1]) {
                aux = vetor[j];
                vetor[j] = vetor[j + 1];
                vetor[j + 1] = aux;
            }
        }
    }
}

código base para a função bubble_sort:
http://devfuria.com.br/logica-de-programacao/exemplos-na-linguagem-c-do-algoritmo-bubble-sort/
*/

int main(void) {
  int mypid, pid, status;
  int arr[SIZE] = {9, 4, 7, 3, 2, 8, 5, 1, 6, 0}; //vetor de 10 posições.

  printf("Vetor antes do fork():\n");
  for (int i = 0; i < SIZE; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");

  mypid = getpid();
  printf("Pai (PID %d) está executando.\n", mypid);

  pid = fork(); //criando o processo FILHO.

  if (pid != 0) {            //Pai
    waitpid(-1, &status, 0); //espera algum FILHO terminar para executar.
    printf("Vetor no PAI depois do waitpid():\n");
    for (int i = 0; i < SIZE; i++) {
      printf("%d ", arr[i]);
    }
    printf("\n");
    printf("Filho (PID %d) terminou.\n", pid);
  } else { //Filho
    pid = getpid();
    printf("Filho (PID %d) está executando.\n", pid);

    bubble_sort(arr, SIZE); //FILHO ordena o vetor
    printf("Vetor no FILHO (ordenado):\n");
    for (int i = 0; i < SIZE; i++) {
      printf("%d ", arr[i]);
    }
    printf("\n");

    exit(0);
  }

  printf("Pai (PID %d) terminou.\n", mypid);
  return 0;
}
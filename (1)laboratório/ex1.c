//Laboratório 1 - 20/08/2024 - Julia Simão e Thiago Henriques - Exercício 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int mypid, pid, status; //mypid: pid do processo atual (PAI); pid: pid do processo FILHO; status: status do processo FILHO.

  mypid = getpid();
  printf("Pai (PID %d) está executando.\n", mypid);

  pid = fork(); //criando o processo FILHO. obtendo o pid do FILHO (ou seja, 0).

  if (pid != 0) { //Pai
    waitpid(-1, &status, 0); //esperando o processo FILHO (-1: espera algum processo FILHO acabar, o primeiro ou o mais rápido) terminar para executar.
    printf("Filho (PID %d) terminou.\n", pid);
  }

  else { //Filho
    pid = getpid();
    printf("Filho (PID %d) está executando.\n", pid);
    exit(3);
  }

  printf("Pai (PID %d) terminou.\n", mypid);
  return 0;
}
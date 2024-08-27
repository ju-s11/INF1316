//Laboratório 1 - 20/08/2024 - Julia Simão e Thiago Henriques - Exercício 4 (echo)

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int mypid, pid, status;

    mypid = getpid();
    printf("Pai (PID %d) está executando.\n", mypid);

    pid = fork(); //criando o processo FILHO.

    if (pid != 0) { //Pai
        waitpid(-1, &status, 0); //espera algum FILHO terminar.
        printf("Filho (PID %d) terminou.\n", pid);
    } else { //Filho
        pid = getpid();
        printf("Filho (PID %d) está executando o comando 'echo'.\n", pid);

        //executa o comando 'echo' da shell.
        execl("/bin/echo", "echo", "Alô Mundo", (char *)NULL);

        //caso o execl falhe.
        perror("execl falhou");
        exit(1);
    }

    printf("Pai (PID %d) terminou.\n", mypid);
    return 0;
}

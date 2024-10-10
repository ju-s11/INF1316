#include <stdio.h>
#include <time.h>

unsigned long long fatorial(int n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    unsigned long long resultado = 1;
    for (int i = 2; i <= n; i++) {
        resultado *= i;
    }
    return resultado;
}

int main() {
    for (int i = 0; i <= 20; i++) {
        printf("O fatorial de %d é %llu\n", i, fatorial(i));
        sleep(1);
    }
    return 0;
}

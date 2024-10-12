#include <stdio.h>
#include <time.h>

int main() {
    for (int i,j = 0; i <= 200; i++, j+=2) {
        printf("Pares(%d) = %d\n", i, j);
        sleep(1);
    }

    return 0;
}

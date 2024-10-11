#include <stdio.h>
#include <time.h>

int main() {
    printf("Números pares de 0 a 1000:\n");
    
    for (int i = 0; i <= 200; i += 2) {
        printf("%d\n", i);
        sleep(1);
    }

    return 0;
}

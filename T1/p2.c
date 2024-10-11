#include <stdio.h>
#include <time.h>

unsigned long long fibonacci(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;

    unsigned long long a = 0, b = 1, c;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

int main() {
    for (int i = 0; i <= 50; i++) {
        printf("Fibonacci(%d) = %llu\n", i, fibonacci(i));
        sleep(1);
    }
    return 0;
}

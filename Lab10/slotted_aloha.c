#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main() {
    int N, T;
    float p;

    int successful = 0;
    int collisions = 0;
    int idle = 0;

    srand(time(NULL));

    printf("Enter number of nodes: ");
    scanf("%d", &N);

    printf("Enter transmission probability (p): ");
    scanf("%f", &p);

    printf("Enter total number of slots: ");
    scanf("%d", &T);

    if (N <= 0 || T <= 0 || p < 0 || p > 1) {
        printf("Invalid input!\n");
        return 1;
    }

    for (int slot = 1; slot <= T; slot++) {
        int transmissions = 0;

        for (int node = 0; node < N; node++) {
            float random_value = (float)rand() / RAND_MAX;

            if (random_value < p) {
                transmissions++;
            }
        }

        if (transmissions == 0) {
            idle++;
        }
        else if (transmissions == 1) {
            successful++;
        }
        else {
            collisions++;
        }
    }

    float observed_efficiency = (float)successful / T;
    float theoretical_efficiency = N * p * (float)pow(1 - p, N - 1);

    printf("\nSuccessful slots      : %d\n", successful);
    printf("Collision slots       : %d\n", collisions);
    printf("Idle slots            : %d\n", idle);

    printf("\nObserved efficiency   : %.2f%%\n",
           observed_efficiency * 100);

    printf("Theoretical efficiency: %.2f%%\n",
           theoretical_efficiency * 100);

    return 0;
}

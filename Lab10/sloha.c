#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int seed = 42;

int main(){
    srand(seed);
    int nodes, slots;
    float prob;

    int missCount = 0;
    int hitCount = 0;
    int collision = 0;

    int hit = 0;
    printf("Nodes: ");
    scanf("%d", &nodes);

    printf("Transmission probability: ");
    scanf("%f", &prob);

    printf("Total Slots: ");
    scanf("%d", &slots);

    for(int i=0; i<slots; i++){
        hit = 0;

        for(int j=0; j<nodes; j++){
            if((1.0 * rand())/RAND_MAX < prob){
                if(hit == 0)
                    hit = 1;
                else
                    hit = 2;
            }
        }

        switch(hit){
            case 0:
                missCount++;
                break;
            case 1:
                hitCount++;
                break;
            case 2:
                collision++;
                break;
            default:
                exit(1);
        }
    }

    printf("\nSuccessful Slots: %d\n", hitCount);
    printf("Collision Slots: %d\n", collision);
    printf("Idle Slots: %d\n", missCount);

    printf("\nObserved Efficiency: %.2f %\n", (100.0 * hitCount)/slots);
    printf("Theoretical Efficiency: %.2f %\n", 100 * nodes * (prob) * pow(1-prob, nodes-1));
}

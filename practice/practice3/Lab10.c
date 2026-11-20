#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(){
    srand(time(NULL));
    int nodes, slots;
    float prob;

    printf("Enter number of nodes: ");
    scanf("%d", &nodes);
    printf("Enter number of slots: ");
    scanf("%d", &slots);
    printf("Enter probability of transmission (0 < p < 1): ");
    scanf("%f", &prob);

    int miss = 0, hit = 0, collide = 0;

    for(int i=0; i<slots; i++){
        int collision = 0;
        
        for(int j=0; j<nodes; j++){
            float randNum = (float)rand() / RAND_MAX;
            if(randNum < prob){
                collision++;
            }
        }

        if(collision > 1){
            collide++;
        } else if(collision == 1){
            hit++;
        } else {
            miss++;
        }
    }

    printf("Results:\n");
    printf("Missed transmissions: %d\n", miss);
    printf("Successful transmissions: %d\n", hit);
    printf("Collisions: %d\n", collide);

    printf("Efficiency: %.2f%%\n", (float)hit / slots * 100);
    printf("Ideal efficiency: %.2f%%\n", (float)nodes * prob * pow(1-prob, nodes-1) * 100);

    return 0;
}
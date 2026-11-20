
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(){
    srand(time(NULL));

    int nodes;
    float prob, totalTime, packetTime;

    printf("Enter number of nodes: ");
    scanf("%d", &nodes);
    printf("Enter total simulation time: ");
    scanf("%f", &totalTime);
    printf("Enter packet transmission time: ");
    scanf("%f", &packetTime);
    printf("Enter probability of transmission: ");
    scanf("%f", &prob);

    int miss = 0, hit = 0, collide = 0;

    // Number of packets each node can attempt
    int intervals = (int)(totalTime / packetTime);

    // Store transmission start times
    float **transmissions = malloc(nodes * sizeof(float *));

    for(int i = 0; i < nodes; i++){
        transmissions[i] = malloc(intervals * sizeof(float));
    }

    // Generate random transmission times
    for(int i = 0; i < nodes; i++){
        for(int j = 0; j < intervals; j++){
            float randNum = (float)rand() / RAND_MAX;

            if(randNum < prob){
                transmissions[i][j] =
                    ((float)rand() / RAND_MAX) * totalTime;
            } else {
                transmissions[i][j] = -1;
            }
        }
    }

    // Check collisions
    for(int i = 0; i < nodes; i++){
        for(int j = 0; j < intervals; j++){

            float start = transmissions[i][j];

            if(start < 0)
                continue;

            int collision = 0;

            for(int k = 0; k < nodes; k++){
                for(int l = 0; l < intervals; l++){

                    if(i == k && j == l)
                        continue;

                    float other = transmissions[k][l];

                    if(other < 0)
                        continue;

                    // Check whether transmission intervals overlap
                    if(start < other + packetTime &&
                       other < start + packetTime){
                        collision = 1;
                    }
                }
            }

            if(collision)
                collide++;
            else
                hit++;
        }
    }

    // Count attempted transmissions and idle opportunities
    int totalAttempts = 0;

    for(int i = 0; i < nodes; i++){
        for(int j = 0; j < intervals; j++){
            if(transmissions[i][j] >= 0)
                totalAttempts++;
        }
    }

    miss = nodes * intervals - totalAttempts;

    printf("\nResults:\n");
    printf("Missed transmissions: %d\n", miss);
    printf("Successful transmissions: %d\n", hit);
    printf("Collisions: %d\n", collide);

    printf("Efficiency: %.2f%%\n",
        (float)hit * packetTime / totalTime * 100);

    printf("Total transmission attempts: %d\n", totalAttempts);

    printf("Ideal Pure ALOHA maximum throughput: 18.4%%\n");

    for(int i = 0; i < nodes; i++)
        free(transmissions[i]);

    free(transmissions);

    return 0;
}

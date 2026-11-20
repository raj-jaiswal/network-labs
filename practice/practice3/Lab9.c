#include <stdio.h>
#include <stdlib.h>

#define INT_MAX 10000000

int adjMatrix[100][100];
int nRouters, nLinks;
int sequences[100];

typedef struct {
    int routerID;
    int sequenceNumber;
    int linkState[100];
} LSA;

typedef struct {
    int routerID;
    int cost;
} ForwardingEntry;

typedef struct {
    int cost;
    int visited;
    int nextHop;
} Node;

void simHello(){
    for(int i=0; i<nRouters; i++){
        for(int j=0; j<nRouters; j++){
            if(adjMatrix[i][j] != -1){
                printf("%d HELLO %d\n", i, j);
            }
        }
        printf("\n");
    }
}

void dijkstras(int routerID){
    Node nodes[100];
    for(int i=0; i<nRouters; i++){
        nodes[i].cost = INT_MAX;
        nodes[i].visited = 0;
        nodes[i].nextHop = -1;
    }

    nodes[routerID].cost = 0;
    nodes[routerID].nextHop = routerID;
    
    for(int i=0; i<nRouters; i++){
        int minCost = INT_MAX;
        int minNode = -1;

        for(int j=0; j<nRouters; j++){
            if(!nodes[j].visited && nodes[j].cost < minCost){
                minCost = nodes[j].cost;
                minNode = j;
            }
        }

        nodes[minNode].visited = 1;
        for(int j=0; j<nRouters; j++){
            if(adjMatrix[minNode][j] != -1 && !nodes[j].visited){
                if(minCost + adjMatrix[minNode][j] < nodes[j].cost){
                    nodes[j].cost = minCost + adjMatrix[minNode][j];
                    nodes[j].nextHop = (minNode == routerID) ? j : nodes[minNode].nextHop;
                }
            }
        }
    }

    printf("Forwarding Table for Router %d:\n", routerID);
    for(int i=0; i<nRouters; i++){
        printf("Destination: %d, Cost: %d, Next Hop: %d\n", i, nodes[i].cost, nodes[i].nextHop);
    }
}

int main(){
    printf("Enter the number of routers: ");
    scanf("%d", &nRouters);
    printf("Enter the number of links: ");
    scanf("%d", &nLinks);

    for(int i=0; i<nRouters; i++){
        sequences[i] = 1;
        for(int j=0; j<nRouters; j++){
            adjMatrix[i][j] = -1;
        }
    }

    printf("Enter Link Details (router1 router2 cost): \n");
    for(int i=0; i<nLinks; i++){
        int r1, r2, c;
        scanf("%d %d %d", &r1, &r2, &c);
        adjMatrix[r1][r2] = c;
        adjMatrix[r2][r1] = c;
    }

    int choice;
    printf("Options: \n");
    printf("1. Display Network Topology\n");
    printf("2. Simulate Hello Packet Exchange\n");
    printf("3. Generate LSAs\n");
    printf("4. Simulate LSA Flooding\n");
    printf("5. Display Received LSAs at a Router\n");
    printf("6. Display forwarding table\n");
    printf("7. Exit\n");

    printf("\nEnter your choice: ");
    scanf("%d", &choice);

    while(choice != 7){
        switch(choice){
            case 1:
                printf("Network Topology:\n");
                for(int i=0; i<nRouters; i++){
                    printf("Router %d: ", i);
                    for(int j=0; j<nRouters; j++){
                        if(adjMatrix[i][j] != -1) printf("%d ", adjMatrix[i][j]);
                    }
                    printf("\n");
                }
                break;
            case 2:
                simHello();
                break;
            case 3:
                printf("Generating LSAs...\n");
                // Generate LSAs logic here
                break;
            case 4:
                printf("Simulating LSA Flooding...\n");
                // Simulate LSA Flooding logic here
                break;
            case 5:
                printf("Displaying Received LSAs at a Router...\n");
                // Display Received LSAs logic here
                break;
            case 6:
                printf("Router ID: ");
                int routerID;
                scanf("%d", &routerID);
                dijkstras(routerID);
                break;
            case 7:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
        printf("\nEnter your choice: ");
        scanf("%d", &choice);
    }

    return 0;
}
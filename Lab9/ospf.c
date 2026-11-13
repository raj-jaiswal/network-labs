#include <stdio.h>
#include <string.h>

#define MAX 10
#define MAXQ 10000

typedef struct {
    int seq;
    int count;
    int neighbor[MAX];
    int cost[MAX];
} LSA;

typedef struct {
    int from, to, origin, seq;
} Event;

int n, m;
int adj[MAX][MAX];
LSA generated[MAX], lsdb[MAX][MAX];
int lsaReady = 0;

void displayTopology() {
    printf("\nNetwork Topology:\n");

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (adj[i][j])
                printf("Router %d -- Router %d : Cost = %d\n", i, j, adj[i][j]);
        }
    }
}

void helloExchange() {
    printf("\nHELLO Packet Exchange:\n");

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (adj[i][j]) {
                printf("Router %d sends HELLO to Router %d\n", i, j);
                printf("Router %d receives HELLO from Router %d\n", j, i);

                printf("Router %d sends HELLO to Router %d\n", j, i);
                printf("Router %d receives HELLO from Router %d\n", i, j);
            }
        }
    }

    printf("\nDiscovered Neighbors:\n");

    for (int i = 0; i < n; i++) {
        printf("Router %d: ", i);
        int found = 0;

        for (int j = 0; j < n; j++) {
            if (adj[i][j]) {
                if (found) printf(", ");
                printf("%d", j);
                found = 1;
            }
        }

        if (!found) printf("None");
        printf("\n");
    }
}

void generateLSAs() {
    memset(lsdb, 0, sizeof(lsdb));

    for (int i = 0; i < n; i++) {
        generated[i].seq = 1;
        generated[i].count = 0;

        for (int j = 0; j < n; j++) {
            if (adj[i][j]) {
                int k = generated[i].count++;
                generated[i].neighbor[k] = j;
                generated[i].cost[k] = adj[i][j];
            }
        }

        lsdb[i][i] = generated[i];
    }

    lsaReady = 1;

    printf("\nGenerated LSAs:\n");

    for (int i = 0; i < n; i++) {
        printf("\nLSA from Router %d:\n", i);
        printf("Sequence Number: %d\n", generated[i].seq);
        printf("Neighbors: ");

        if (generated[i].count == 0) {
            printf("None");
        }

        for (int j = 0; j < generated[i].count; j++) {
            if (j) printf(", ");
            printf("(%d,%d)", generated[i].neighbor[j],
                   generated[i].cost[j]);
        }

        printf("\n");
    }
}

void floodLSAs() {
    if (!lsaReady) {
        printf("\nGenerate LSAs first.\n");
        return;
    }

    Event queue[MAXQ];
    int front = 0, rear = 0;

    memset(lsdb, 0, sizeof(lsdb));

    for (int i = 0; i < n; i++)
        lsdb[i][i] = generated[i];

    printf("\nLSA Flooding:\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j]) {
                printf("\nRouter %d sends LSA from Router %d to Router %d\n",
                       i, i, j);

                queue[rear++] = (Event){i, j, i, generated[i].seq};
            }
        }
    }

    while (front < rear) {
        Event e = queue[front++];

        printf("Router %d receives LSA from Router %d (Origin: %d, Sequence: %d)\n",
               e.to, e.from, e.origin, e.seq);

        if (e.seq > lsdb[e.to][e.origin].seq) {
            lsdb[e.to][e.origin] = generated[e.origin];

            for (int j = 0; j < n; j++) {
                if (adj[e.to][j] && j != e.from) {
                    printf("Router %d sends LSA from Router %d to Router %d\n",
                           e.to, e.origin, j);

                    if (rear < MAXQ)
                        queue[rear++] = (Event){
                            e.to, j, e.origin, e.seq
                        };
                }
            }
        } else {
            printf("Router %d rejects duplicate/older LSA from Router %d\n",
                   e.to, e.origin);
        }
    }

    printf("\nLSA Flooding Completed.\n");

    int complete = 1;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (lsdb[i][j].seq < generated[j].seq) {
                printf("Router %d is missing LSA from Router %d\n", i, j);
                complete = 0;
            }
        }
    }

    if (complete)
        printf("Every router has received all LSAs.\n");
}

void displayReceivedLSAs() {
    int r;

    printf("\nEnter Router ID: ");
    scanf("%d", &r);

    if (r < 0 || r >= n) {
        printf("Invalid Router ID.\n");
        return;
    }

    printf("\nRouter %d's received LSA records:\n", r);

    int found = 0;

    for (int i = 0; i < n; i++) {
        if (lsdb[r][i].seq > 0) {
            printf("Origin %d, Sequence %d: ",
                   i, lsdb[r][i].seq);

            if (lsdb[r][i].count == 0) {
                printf("None");
            }

            for (int j = 0; j < lsdb[r][i].count; j++) {
                if (j) printf(", ");
                printf("(%d,%d)", lsdb[r][i].neighbor[j],
                       lsdb[r][i].cost[j]);
            }

            printf("\n");
            found = 1;
        }
    }

    if (!found)
        printf("No LSAs received yet.\n");
}

int main() {
    printf("Number of Routers: ");
    scanf("%d", &n);

    printf("Number of Links: ");
    scanf("%d", &m);

    printf("Enter links (Router1 Router2 Cost):\n");

    for (int i = 0; i < m; i++) {
        int u, v, cost;
        scanf("%d %d %d", &u, &v, &cost);
        adj[u][v] = cost;
        adj[v][u] = cost;
    }

    int choice;

    do {
        printf("\nOSPF ROUTING SIMULATOR\n");
        printf("1. Display Network Topology\n");
        printf("2. Simulate Hello Packet Exchange\n");
        printf("3. Generate LSAs\n");
        printf("4. Simulate LSA Flooding\n");
        printf("5. Display Received LSAs at a Router\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayTopology();
                break;

            case 2:
                helloExchange();
                break;

            case 3:
                generateLSAs();
                break;

            case 4:
                floodLSAs();
                break;

            case 5:
                displayReceivedLSAs();
                break;

            case 6:
                printf("Exiting simulator.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

    } while (choice != 6);

    return 0;
}

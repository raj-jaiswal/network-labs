#include <stdio.h>
#include <string.h>

#define MAX 10
#define MAXQ 10000
#define INF 1000000000

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

LSA generated[MAX];
LSA lsdb[MAX][MAX];

int lsaReady = 0;
int flooded = 0;

void printLSA(int origin, LSA lsa) {
    printf("Origin %d, Sequence %d: ", origin, lsa.seq);

    if (lsa.count == 0)
        printf("None");

    for (int i = 0; i < lsa.count; i++) {
        if (i) printf(", ");

        printf("(%d,%d)", lsa.neighbor[i], lsa.cost[i]);
    }

    printf("\n");
}

void displayTopology() {
    printf("\nNetwork Topology:\n");

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (adj[i][j]) {
                printf("Router %d -- Router %d : Cost = %d\n",
                       i, j, adj[i][j]);
            }
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

        if (!found)
            printf("None");

        printf("\n");
    }
}

void createLSA(int router) {
    generated[router].count = 0;

    for (int j = 0; j < n; j++) {
        if (adj[router][j] > 0) {
            int k = generated[router].count++;

            generated[router].neighbor[k] = j;
            generated[router].cost[k] = adj[router][j];
        }
    }
}

void generateLSAs() {
    memset(lsdb, 0, sizeof(lsdb));

    for (int i = 0; i < n; i++) {
        generated[i].seq = 1;

        createLSA(i);

        lsdb[i][i] = generated[i];
    }

    lsaReady = 1;
    flooded = 0;

    printf("\nGenerated LSAs:\n");

    for (int i = 0; i < n; i++) {
        printf("\nLSA from Router %d:\n", i);
        printf("Sequence Number: %d\n", generated[i].seq);

        printf("Neighbors: ");

        if (generated[i].count == 0)
            printf("None");

        for (int j = 0; j < generated[i].count; j++) {
            if (j) printf(", ");

            printf("(%d,%d)",
                   generated[i].neighbor[j],
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

    int front = 0;
    int rear = 0;

    printf("\nLSA Flooding:\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j]) {
                printf("\nRouter %d sends LSA from Router %d to Router %d\n",
                       i, i, j);

                if (rear < MAXQ) {
                    queue[rear++] = (Event){
                        i, j, i, generated[i].seq
                    };
                }
            }
        }
    }

    while (front < rear) {
        Event e = queue[front++];

        printf("Router %d receives LSA from Router %d "
               "(Origin: %d, Sequence: %d)\n",
               e.to, e.from, e.origin, e.seq);

        if (e.seq > lsdb[e.to][e.origin].seq) {
            lsdb[e.to][e.origin] = generated[e.origin];

            for (int j = 0; j < n; j++) {
                if (adj[e.to][j] && j != e.from) {
                    printf("Router %d sends LSA from Router %d to Router %d\n",
                           e.to, e.origin, j);

                    if (rear < MAXQ) {
                        queue[rear++] = (Event){
                            e.to, j, e.origin, e.seq
                        };
                    }
                }
            }
        } else {
            printf("Router %d rejects duplicate/older LSA from Router %d\n",
                   e.to, e.origin);
        }
    }

    flooded = 1;

    printf("\nLSA Flooding Completed.\n");

    int complete = 1;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (lsdb[i][j].seq < generated[j].seq)
                complete = 0;
        }
    }

    if (complete)
        printf("Every router has received all LSAs.\n");
    else
        printf("Some routers have not received all LSAs.\n");
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
            printLSA(i, lsdb[r][i]);
            found = 1;
        }
    }

    if (!found)
        printf("No LSAs received yet.\n");
}

void displayRoutingTable(int src) {
    if (!lsaReady || !flooded) {
        printf("\nGenerate and flood LSAs first.\n");
        return;
    }

    int graph[MAX][MAX] = {0};

    for (int i = 0; i < n; i++) {
        if (lsdb[src][i].seq == 0)
            continue;

        LSA lsa = lsdb[src][i];

        for (int j = 0; j < lsa.count; j++) {
            int v = lsa.neighbor[j];

            graph[i][v] = lsa.cost[j];
        }
    }

    int dist[MAX];
    int visited[MAX] = {0};
    int nextHop[MAX];

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        nextHop[i] = -1;
    }

    dist[src] = 0;
    nextHop[src] = src;

    for (int count = 0; count < n; count++) {
        int u = -1;
        int minDist = INF;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (int v = 0; v < n; v++) {
            if (graph[u][v] <= 0 || visited[v])
                continue;

            int newDist = dist[u] + graph[u][v];

            int candidateHop = (u == src) ? v : nextHop[u];

            if (newDist < dist[v]) {
                dist[v] = newDist;
                nextHop[v] = candidateHop;
            } else if (newDist == dist[v] &&
                       candidateHop >= 0 &&
                       (nextHop[v] == -1 ||
                        candidateHop < nextHop[v])) {
                nextHop[v] = candidateHop;
            }
        }
    }

    printf("\nRouting Table for Router %d:\n\n", src);

    printf("%-15s %-15s %-15s\n",
           "Destination", "Next Hop", "Total Cost");

    for (int i = 0; i < n; i++) {
        if (dist[i] == INF) {
            printf("%-15d %-15s %-15s\n",
                   i, "Unreachable", "INF");
        } else {
            printf("%-15d %-15d %-15d\n",
                   i, nextHop[i], dist[i]);
        }
    }
}

void selectRoutingTable() {
    int r;

    if (!lsaReady || !flooded) {
        printf("\nGenerate and flood LSAs first.\n");
        return;
    }

    printf("\nEnter Router ID: ");
    scanf("%d", &r);

    if (r < 0 || r >= n) {
        printf("Invalid Router ID.\n");
        return;
    }

    displayRoutingTable(r);
}

void updateTopology() {
    if (!lsaReady || !flooded) {
        printf("\nGenerate and flood initial LSAs first.\n");
        return;
    }

    int choice, u, v, cost;

    printf("\nTopology Change Menu\n");
    printf("1. Modify Link Cost\n");
    printf("2. Remove Link\n");
    printf("Enter choice: ");

    scanf("%d", &choice);

    if (choice != 1 && choice != 2) {
        printf("Invalid choice.\n");
        return;
    }

    printf("Enter Router IDs (u v): ");
    scanf("%d %d", &u, &v);

    if (u < 0 || u >= n || v < 0 || v >= n || u == v) {
        printf("Invalid Router IDs.\n");
        return;
    }

    if (adj[u][v] == 0) {
        printf("Link does not exist.\n");
        return;
    }

    if (choice == 1) {
        printf("Enter new link cost: ");
        scanf("%d", &cost);

        if (cost <= 0) {
            printf("Invalid cost.\n");
            return;
        }

        if (adj[u][v] == cost) {
            printf("Link cost is unchanged.\n");
            return;
        }

        adj[u][v] = adj[v][u] = cost;

        printf("\nLink cost updated: %d -- %d = %d\n",
               u, v, cost);

    } else {
        adj[u][v] = adj[v][u] = 0;

        printf("\nLink removed: %d -- %d\n", u, v);
    }

    int affected[2] = {u, v};

    for (int i = 0; i < 2; i++) {
        int r = affected[i];

        generated[r].seq++;

        createLSA(r);

        printf("\nRouter %d generated updated LSA:\n", r);
        printf("Sequence Number: %d\n", generated[r].seq);

        printf("Neighbors: ");

        if (generated[r].count == 0)
            printf("None");

        for (int j = 0; j < generated[r].count; j++) {
            if (j) printf(", ");

            printf("(%d,%d)",
                   generated[r].neighbor[j],
                   generated[r].cost[j]);
        }

        printf("\n");
    }

    Event queue[MAXQ];

    int front = 0;
    int rear = 0;

    printf("\nUpdated LSA Flooding:\n");

    for (int i = 0; i < 2; i++) {
        int origin = affected[i];

        lsdb[origin][origin] = generated[origin];

        for (int j = 0; j < n; j++) {
            if (adj[origin][j]) {
                printf("\nRouter %d sends updated LSA from Router %d to Router %d\n",
                       origin, origin, j);

                if (rear < MAXQ) {
                    queue[rear++] = (Event){
                        origin, j, origin, generated[origin].seq
                    };
                }
            }
        }
    }

    while (front < rear) {
        Event e = queue[front++];

        printf("Router %d receives LSA from Router %d "
               "(Origin: %d, Sequence: %d)\n",
               e.to, e.from, e.origin, e.seq);

        if (e.seq > lsdb[e.to][e.origin].seq) {
            lsdb[e.to][e.origin] = generated[e.origin];

            for (int j = 0; j < n; j++) {
                if (adj[e.to][j] && j != e.from) {
                    printf("Router %d forwards LSA from Router %d to Router %d\n",
                           e.to, e.origin, j);

                    if (rear < MAXQ) {
                        queue[rear++] = (Event){
                            e.to, j, e.origin, e.seq
                        };
                    }
                }
            }
        } else {
            printf("Router %d rejects duplicate/older LSA from Router %d\n",
                   e.to, e.origin);
        }
    }

    printf("\nUpdated LSDB at all reachable routers.\n");

    printf("\nRouting Tables After Topology Change:\n");

    for (int i = 0; i < n; i++) {
        displayRoutingTable(i);
    }
}

int main() {
    printf("Number of Routers: ");
    scanf("%d", &n);

    if (n < 2 || n > MAX) {
        printf("Invalid number of routers.\n");
        return 1;
    }

    printf("Number of Links: ");
    scanf("%d", &m);

    if (m < 0 || m > n * (n - 1) / 2) {
        printf("Invalid number of links.\n");
        return 1;
    }

    printf("Enter links (Router1 Router2 Cost):\n");

    for (int i = 0; i < m; i++) {
        int u, v, cost;

        scanf("%d %d %d", &u, &v, &cost);

        if (u < 0 || u >= n ||
            v < 0 || v >= n ||
            u == v || cost <= 0 ||
            adj[u][v] != 0) {
            printf("Invalid link.\n");
            return 1;
        }

        adj[u][v] = adj[v][u] = cost;
    }

    int choice;

    do {
        printf("\n========== OSPF ROUTING SIMULATOR ==========\n");
        printf("1. Display Network Topology\n");
        printf("2. Simulate Hello Packet Exchange\n");
        printf("3. Generate LSAs\n");
        printf("4. Simulate LSA Flooding\n");
        printf("5. Display Received LSAs at a Router\n");
        printf("6. Display Routing Table (Dijkstra)\n");
        printf("7. Modify Topology / Remove Link and Recalculate Routes\n");
        printf("8. Exit\n");

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
                selectRoutingTable();
                break;

            case 7:
                updateTopology();
                break;

            case 8:
                printf("Exiting simulator.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

    } while (choice != 8);

    return 0;
}

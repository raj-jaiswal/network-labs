#include <stdio.h>
#include <limits.h>

#define V 5        // Number of vertices (0 to 4)
#define INF INT_MAX

// Bellman-Ford algorithm using a fixed adjacency matrix
void bellmanFord(int graph[V][V], int src) {
    int dist[V];

    // Step 1: Initialize distances from src to all other vertices as INFINITE
    for (int i = 0; i < V; i++) {
        dist[i] = INF;
    }
    dist[src] = 0;

    // Step 2: Relax all edges (V - 1) times
    for (int count = 1; count <= V - 1; count++) {
        // Check every pair (u, v) in the adjacency matrix
        for (int u = 0; u < V; u++) {
            for (int v = 0; v < V; v++) {
                // Proceed only if an edge exists (graph[u][v] != INF)
                if (graph[u][v] != INF && dist[u] != INF && dist[u] + graph[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph[u][v];
                }
            }
        }
    }

    // Step 3: Check for negative-weight cycles (1 extra pass)
    for (int u = 0; u < V; u++) {
        for (int v = 0; v < V; v++) {
            if (graph[u][v] != INF && dist[u] != INF && dist[u] + graph[u][v] < dist[v]) {
                printf("Graph contains a negative weight cycle!\n");
                return;
            }
        }
    }

    // Print the final shortest distances
    printf("Vertex   Distance from Source (%d)\n", src);
    for (int i = 0; i < V; i++) {
        if (dist[i] == INF)
            printf("%d \t\t INF\n", i);
        else
            printf("%d \t\t %d\n", i, dist[i]);
    }
}

int main() {
    // Fixed adjacency matrix (INF means no direct edge between u and v)
    // Uses the exact same directed weighted graph as the adjacency list version
    int graph[V][V] = {
        {   0,  -1,   4, INF, INF }, // Edges from 0: (0->1, -1), (0->2, 4)
        { INF,   0,   3,   2,   2 }, // Edges from 1: (1->2, 3), (1->3, 2), (1->4, 2)
        { INF, INF,   0, INF, INF }, // Edges from 2: none
        { INF,   1,   5,   0, INF }, // Edges from 3: (3->1, 1), (3->2, 5)
        { INF, INF, INF,  -3,   0 }  // Edges from 4: (4->3, -3)
    };

    int source = 0;
    bellmanFord(graph, source);

    return 0;
}
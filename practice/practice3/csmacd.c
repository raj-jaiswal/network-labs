#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Global variables to store network config and stats across menu options
int nodes = 4, slots = 1000;
float prob = 0.25;

// Stats for Option 4
int last_mode = 0; // 1 for ALOHA, 2 for CSMA/CD
int stat_hit = 0, stat_collide = 0, stat_miss = 0, stat_dropped = 0;

// Part A: Your original Slotted ALOHA code wrapped in a function
void runSlottedAloha() {
    int miss = 0, hit = 0, collide = 0;

    for(int i = 0; i < slots; i++) {
        int collision = 0;
        
        for(int j = 0; j < nodes; j++) {
            float randNum = (float)rand() / RAND_MAX;
            if(randNum < prob) {
                collision++;
            }
        }

        if(collision > 1) {
            collide++;
        } else if(collision == 1) {
            hit++;
        } else {
            miss++;
        }
    }

    // Save stats for Option 4
    last_mode = 1;
    stat_hit = hit; stat_collide = collide; stat_miss = miss; stat_dropped = 0;

    printf("\n--- Slotted ALOHA Results ---\n");
    printf("Nodes: %d | Probability: %.2f | Total slots: %d\n", nodes, prob, slots);
    printf("Successful slots: %d\n", hit);
    printf("Collision slots: %d\n", collide);
    printf("Idle slots: %d\n", miss);
    printf("Observed efficiency: %.2f%%\n", (float)hit / slots * 100);
    printf("Theoretical efficiency: %.2f%%\n", (float)nodes * prob * pow(1 - prob, nodes - 1) * 100);
}

// Part B: CSMA/CD with Binary Exponential Backoff
void runCSMACD() {
    int has_packet[100];    // 1 if node has a packet to send, 0 if done
    int backoff[100];       // Remaining contention slots to wait
    int collisions[100];    // Collision counter (k) for each node
    
    int hit = 0, collide = 0, miss = 0, dropped = 0;
    int max_attempts = 16;  // M = 16 limit before dropping packet
    int channel_busy = 0;   // 0 = idle, 1 = busy (simulates packet transmission duration)

    // Initialize all nodes with 1 packet ready to transmit immediately (backoff = 0)
    for(int j = 0; j < nodes; j++) {
        has_packet[j] = 1;
        backoff[j] = 0;
        collisions[j] = 0;
    }

    printf("\n--- CSMA/CD Simulation Trace ---\n");

    for(int i = 0; i < slots; i++) {
        // Stop early if all nodes have either sent or dropped their packets
        int active_nodes = 0;
        for(int j = 0; j < nodes; j++) {
            if(has_packet[j] == 1) active_nodes++;
        }
        if(active_nodes == 0) break;

        // 1. Check if channel is busy (e.g., ongoing transmission from previous slot)
        if(channel_busy > 0) {
            channel_busy--; // Channel becomes free after transmission finishes
            continue;
        }

        // 2. Channel is IDLE: Find which nodes have expired backoff (backoff == 0)
        int transmitters[100];
        int tx_count = 0;

        for(int j = 0; j < nodes; j++) {
            if(has_packet[j] == 1 && backoff[j] == 0) {
                transmitters[tx_count] = j;
                tx_count++;
            }
        }

        // 3. Evaluate transmission outcome for this slot
        if(tx_count == 0) {
            // Idle slot: No one transmitted, so decrement backoff counters for waiting nodes
            miss++;
            for(int j = 0; j < nodes; j++) {
                if(has_packet[j] == 1 && backoff[j] > 0) {
                    backoff[j]--;
                }
            }
        }
        else if(tx_count == 1) {
            // Success: Exactly 1 node transmitted
            int winner = transmitters[0];
            hit++;
            has_packet[winner] = 0;   // Packet sent!
            collisions[winner] = 0;   // Reset collision counter
            channel_busy = 1;         // Occupy channel for 1 extra slot to model frame transmission
            printf("Slot %d: Node %d transmits successfully\n", i + 1, winner + 1);
        } 
        else {
            // Collision: 2 or more nodes transmitted simultaneously
            collide++;
            printf("Slot %d: Collision Detected between ", i + 1);
            for(int t = 0; t < tx_count; t++) {
                printf("Node %d ", transmitters[t] + 1);
            }
            printf("\n");

            // Apply Binary Exponential Backoff to all colliding nodes
            for(int t = 0; t < tx_count; t++) {
                int node_id = transmitters[t];
                collisions[node_id]++; // Increment collision count (k)

                // Check if max attempts (16) reached
                if(collisions[node_id] >= max_attempts) {
                    has_packet[node_id] = 0; // Drop packet
                    dropped++;
                    printf("  Node %d: Packet DROPPED after %d collisions\n", node_id + 1, max_attempts);
                } else {
                    // Calculate k = min(collisions, 10)
                    int k = (collisions[node_id] < 10) ? collisions[node_id] : 10;
                    int max_range = (int)pow(2, k); // 2^k
                    
                    // Pick random backoff in {0, 1, ..., 2^k - 1}
                    backoff[node_id] = rand() % max_range;
                    printf("  Node %d: Backoff = %d (Collision #%d)\n", node_id + 1, backoff[node_id], collisions[node_id]);
                }
            }
        }
    }

    // Save stats for Option 4
    last_mode = 2;
    stat_hit = hit; stat_collide = collide; stat_miss = miss; stat_dropped = dropped;

    printf("\nTotal successful transmissions: %d\n", hit);
    printf("Total collisions: %d\n", collide);
    printf("Total dropped packets: %d\n", dropped);
}

int main() {
    srand(time(NULL));
    int choice;

    while(1) {
        printf("\n=== MAC Protocol Simulator ===\n");
        printf("1. Configure Network\n");
        printf("2. Run Slotted ALOHA\n");
        printf("3. Run CSMA/CD\n");
        printf("4. Display Simulation Statistics\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                printf("Enter number of nodes (max 100): ");
                scanf("%d", &nodes);
                printf("Enter number of slots: ");
                scanf("%d", &slots);
                printf("Enter probability of transmission for ALOHA (0 < p < 1): ");
                scanf("%f", &prob);
                break;
            case 2:
                runSlottedAloha();
                break;
            case 3:
                runCSMACD();
                break;
            case 4:
                if(last_mode == 0) {
                    printf("\nNo simulation has been run yet!\n");
                } else {
                    printf("\n--- Last Simulation Statistics (%s) ---\n", last_mode == 1 ? "Slotted ALOHA" : "CSMA/CD");
                    printf("Nodes: %d | Configured Slots: %d\n", nodes, slots);
                    printf("Successful Transmissions: %d\n", stat_hit);
                    printf("Collisions: %d\n", stat_collide);
                    printf("Idle Slots: %d\n", stat_miss);
                    if(last_mode == 2) printf("Dropped Packets: %d\n", stat_dropped);
                }
                break;
            case 5:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}
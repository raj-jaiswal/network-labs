#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int pacLen;
double str_bw, rtd_bw;
double str_prop_delay, rtd_prop_delay, rproc_delay;
int rqueue_cap;
double packet_gen_rate;
int nPacket, seed;

typedef struct {
    double *departures;
    int head;
    int tail;
    int count;
    int capacity;
} Queue;

int push(Queue *q, double dep_time) {
    if (q->count == q->capacity) {
        return 0;
    }
    q->departures[q->tail] = dep_time;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    return 1;
}

int pop(Queue *q) {
    if (q->count == 0) {
        return 0;
    }
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return 1;
}

void getInput() {
    printf("1. Packet length (bytes): ");
    scanf("%d", &pacLen);

    printf("2. Source-to-router link bandwidth (bps): ");
    scanf("%lf", &str_bw);

    printf("3. Router-to-destination link bandwidth (bps): ");
    scanf("%lf", &rtd_bw);

    printf("4. Source-to-router propagation delay (seconds): ");
    scanf("%lf", &str_prop_delay);

    printf("5. Router-to-destination propagation delay (seconds): ");
    scanf("%lf", &rtd_prop_delay);

    printf("6. Router processing delay (seconds per packet): ");
    scanf("%lf", &rproc_delay);

    printf("7. Router queue capacity (number of packets): ");
    scanf("%d", &rqueue_cap);

    printf("8. Packet generation rate, lambda (packets/second): ");
    scanf("%lf", &packet_gen_rate);

    printf("9. Number of packets to simulate: ");
    scanf("%d", &nPacket);

    printf("10. Random number seed: ");
    scanf("%d", &seed);
}

int main() {
    printf("Simulation of Router Queueing Delay\n");
    getInput();
    srand(seed);

    double d_trans1 = (pacLen * 8.0) / str_bw;
    double d_trans2 = (pacLen * 8.0) / rtd_bw;

    double rhos[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 1.0, 1.1, 1.2};
    int num_experiments = 13;

    FILE *fp = fopen("simulation_results.csv", "w");
    if (fp == NULL) {
        return 1;
    }

    fprintf(fp, "Traffic Intensity (rho),Packet Gen Rate (lambda),Packets Generated,Packets Delivered,Packets Dropped,Drop Probability,Avg Queueing Delay,Avg End-to-End Delay,Max Queue Occupancy\n");

    for (int exp = 0; exp < num_experiments; exp++) {
        double rho = rhos[exp];
        double lambda = (rho * rtd_bw) / (pacLen * 8.0);

        int delivered = 0;
        int dropped = 0;
        int max_q_occupancy = 0;
        double total_q_delay = 0.0;
        double total_e2e_delay = 0.0;

        Queue router_queue;
        router_queue.capacity = rqueue_cap;
        router_queue.count = 0;
        router_queue.head = 0;
        router_queue.tail = 0;
        router_queue.departures = (double *)malloc(rqueue_cap * sizeof(double));

        double current_time = 0.0;
        double router_ready_time = 0.0;

        for (int i = 0; i < nPacket; i++) {
            double u = ((double)rand() / RAND_MAX);
            if (u == 0.0 || u == 1.0) {
                u = (0.999999 / RAND_MAX);
            }
            double inter_arrival = -log(u) / lambda;
            current_time += inter_arrival;

            while (router_queue.count > 0 && router_queue.departures[router_queue.head] <= current_time) {
                pop(&router_queue);
            }

            if (router_queue.count > max_q_occupancy) {
                max_q_occupancy = router_queue.count;
            }

            if (router_queue.count == router_queue.capacity) {
                dropped++;
            } 
            else{
                double t_start = (current_time > router_ready_time) ? current_time : router_ready_time;

                double q_delay = t_start - current_time;
                total_q_delay += q_delay;

                double e2e_delay = d_trans1 + str_prop_delay + q_delay + rproc_delay + d_trans2 + rtd_prop_delay;
                total_e2e_delay += e2e_delay;

                double t_dep = t_start + rproc_delay + d_trans2;
                router_ready_time = t_dep;

                push(&router_queue, t_dep);
                delivered++;

                if(router_queue.count > max_q_occupancy){
                    max_q_occupancy = router_queue.count;
                }
            }
        }

        double drop_prob = (double)dropped / nPacket;
        double avg_q_delay = (delivered > 0) ? (total_q_delay / delivered) : 0.0;
        double avg_e2e_delay = (delivered > 0) ? (total_e2e_delay / delivered) : 0.0;

        fprintf(fp, "%.2f,%.4f,%d,%d,%d,%.6f,%.6f,%.6f,%d\n",
                rho, lambda, nPacket, delivered, dropped, drop_prob, avg_q_delay, avg_e2e_delay, max_q_occupancy);
    }

    fclose(fp);
    printf("Simulation complete! Results saved.\n");

    return 0;
}

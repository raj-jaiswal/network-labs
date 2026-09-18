#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int pacLen;
double str_bw, rtd_bw;
double str_prop_delay, rtd_prop_delay, rproc_delay;
int rqueue_cap;
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

double get_random_uniform() {
    double u = ((double)rand() / RAND_MAX);
    if (u <= 0.0 || u >= 1.0) {
        u = 0.999;
    }
    return u;
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

    printf("8. Number of packets to simulate per experiment: ");
    scanf("%d", &nPacket);

    printf("9. Random number seed: ");
    scanf("%d", &seed);
}

void simulate_traffic(FILE *fp, int num_sources, double *rates, double rho_forced) {
    double d_trans1 = (pacLen * 8.0) / str_bw;
    double d_trans2 = (pacLen * 8.0) / rtd_bw;

    double agg_lambda = 0.0;
    for (int i = 0; i < num_sources; i++) {
        agg_lambda += rates[i];
    }

    double rho = (rho_forced > 0.0) ? rho_forced : (agg_lambda * pacLen * 8.0) / rtd_bw;

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

    double *next_gen_time = (double *)malloc(num_sources * sizeof(double));
    for (int i = 0; i < num_sources; i++) {
        next_gen_time[i] = -log(get_random_uniform()) / rates[i];
    }

    double router_ready_time = 0.0;

    for (int i = 0; i < nPacket; i++) {
        int min_src = 0;
        double min_time = next_gen_time[0];
        for (int s = 1; s < num_sources; s++) {
            if (next_gen_time[s] < min_time) {
                min_time = next_gen_time[s];
                min_src = s;
            }
        }

        double gen_time = min_time;
        next_gen_time[min_src] += -log(get_random_uniform()) / rates[min_src];

        double arr_router_time = gen_time + d_trans1 + str_prop_delay;

        while (router_queue.count > 0 && router_queue.departures[router_queue.head] <= arr_router_time) {
            pop(&router_queue);
        }

        if (router_queue.count > max_q_occupancy) {
            max_q_occupancy = router_queue.count;
        }

        if (router_queue.count == router_queue.capacity) {
            dropped++;
        } else {
            double t_start = (arr_router_time > router_ready_time) ? arr_router_time : router_ready_time;
            double q_delay = t_start - arr_router_time;
            total_q_delay += q_delay;

            double e2e_delay = d_trans1 + str_prop_delay + q_delay + rproc_delay + d_trans2 + rtd_prop_delay;
            total_e2e_delay += e2e_delay;

            double t_dep = t_start + rproc_delay + d_trans2;
            router_ready_time = t_dep;

            push(&router_queue, t_dep);
            delivered++;

            if (router_queue.count > max_q_occupancy) {
                max_q_occupancy = router_queue.count;
            }
        }
    }

    double drop_prob = (double)dropped / nPacket;
    double avg_q_delay = (delivered > 0) ? (total_q_delay / delivered) : 0.0;
    double avg_e2e_delay = (delivered > 0) ? (total_e2e_delay / delivered) : 0.0;

    fprintf(fp, "%.4f,%.4f,%d,%d,%d,%.6f,%.6f,%.6f,%d\n",
            rho, agg_lambda, nPacket, delivered, dropped, drop_prob, avg_q_delay, avg_e2e_delay, max_q_occupancy);

    free(router_queue.departures);
    free(next_gen_time);
}

int main() {
    printf("Simulation of Multi-Source Router Queueing Delay\n");
    getInput();
    srand(seed);

    FILE *fp = fopen("simulation_results.csv", "w");
    if (fp == NULL) {
        printf("Error opening file for writing.\n");
        return 1;
    }

    fprintf(fp, "Traffic Intensity (rho),Aggregate Packet Gen Rate (lambda),Packets Generated,Packets Delivered,Packets Dropped,Drop Probability,Avg Queueing Delay,Avg End-to-End Delay,Max Queue Occupancy\n");

    double rhos[] = {0.2, 0.4, 0.6, 0.8, 0.9, 1.0, 1.2};
    int num_rho_exp = sizeof(rhos) / sizeof(rhos[0]);
    int nSources_prob1 = 4;
    double rates_prob1[4];

    for (int i = 0; i < num_rho_exp; i++) {
        double current_rho = rhos[i];
        double total_lambda = (current_rho * rtd_bw) / (pacLen * 8.0);
        for (int s = 0; s < nSources_prob1; s++) {
            rates_prob1[s] = total_lambda / nSources_prob1;
        }
        simulate_traffic(fp, nSources_prob1, rates_prob1, current_rho);
    }

    double prob2_rates[4][4] = {
        {100.0, 100.0, 100.0, 700.0},
        {150.0, 150.0, 250.0, 450.0},
        {250.0, 250.0, 250.0, 250.0},
        {400.0, 300.0, 200.0, 100.0}
    };

    for (int i = 0; i < 4; i++) {
        simulate_traffic(fp, 4, prob2_rates[i], -1.0);
    }

    fclose(fp);
    printf("Simulation complete. Results written to simulation_results.csv\n");

    return 0;
}

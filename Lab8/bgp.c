#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAX_ROUTES 100
#define MAX_AS 20
#define LOCAL_AS 100

typedef struct {
    uint32_t network;
    uint32_t next_hop;
    int prefix_len;
    int local_pref;
    int as_path[MAX_AS];
    int as_len;
} Route;

Route routes[MAX_ROUTES];
int route_count = 0;

uint32_t get_mask(int len) {
    if (len == 0)
        return 0;
    return 0xFFFFFFFFu << (32 - len);
}

int parse_ip(char *text, uint32_t *ip) {
    struct in_addr addr;

    if (inet_pton(AF_INET, text, &addr) != 1)
        return 0;

    *ip = ntohl(addr.s_addr);
    return 1;
}

int parse_prefix(char *text, uint32_t *network, int *len) {
    char ip[INET_ADDRSTRLEN];
    struct in_addr addr;

    if (sscanf(text, "%15[^/]/%d", ip, len) != 2)
        return 0;

    if (*len < 0 || *len > 32)
        return 0;

    if (inet_pton(AF_INET, ip, &addr) != 1)
        return 0;

    *network = ntohl(addr.s_addr) & get_mask(*len);
    return 1;
}

void ip_to_string(uint32_t ip, char *buffer) {
    struct in_addr addr;

    addr.s_addr = htonl(ip);
    inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN);
}

void prefix_to_string(uint32_t network, int len, char *buffer) {
    char ip[INET_ADDRSTRLEN];

    ip_to_string(network, ip);
    sprintf(buffer, "%s/%d", ip, len);
}

int same_prefix(Route a, Route b) {
    return a.network == b.network && a.prefix_len == b.prefix_len;
}

int is_better(Route a, Route b) {
    if (a.local_pref != b.local_pref)
        return a.local_pref > b.local_pref;

    if (a.as_len != b.as_len)
        return a.as_len < b.as_len;

    return a.next_hop < b.next_hop;
}

void print_as_path(Route r) {
    if (r.as_len == 0) {
        printf("-");
        return;
    }

    for (int i = 0; i < r.as_len; i++) {
        printf("%d", r.as_path[i]);

        if (i < r.as_len - 1)
            printf(" ");
    }
}

void print_route(Route r) {
    char prefix[30], next_hop[INET_ADDRSTRLEN];

    prefix_to_string(r.network, r.prefix_len, prefix);
    ip_to_string(r.next_hop, next_hop);

    printf("Prefix: %s\n", prefix);
    printf("Next Hop: %s\n", next_hop);
    printf("LOCAL_PREF: %d\n", r.local_pref);
    printf("AS_PATH: ");
    print_as_path(r);
    printf("\n");
}

void add_route() {
    char prefix[32], next_hop[INET_ADDRSTRLEN];
    char line[256];
    Route r;

    if (route_count >= MAX_ROUTES) {
        printf("Routing table is full.\n");
        return;
    }

    printf("Prefix: ");
    scanf("%31s", prefix);

    if (!parse_prefix(prefix, &r.network, &r.prefix_len)) {
        printf("Invalid prefix.\n");
        return;
    }

    printf("Next Hop: ");
    scanf("%15s", next_hop);

    if (!parse_ip(next_hop, &r.next_hop)) {
        printf("Invalid next-hop IP.\n");
        return;
    }

    printf("LOCAL_PREF: ");
    if (scanf("%d", &r.local_pref) != 1 || r.local_pref <= 0) {
        printf("Invalid LOCAL_PREF.\n");
        return;
    }

    printf("AS_PATH (space-separated AS numbers): ");
    getchar();

    if (fgets(line, sizeof(line), stdin) == NULL)
        return;

    r.as_len = 0;

    char *token = strtok(line, " \t\n");

    while (token != NULL) {
        if (r.as_len >= MAX_AS) {
            printf("AS_PATH is too long.\n");
            return;
        }

        r.as_path[r.as_len++] = atoi(token);
        token = strtok(NULL, " \t\n");
    }

    for (int i = 0; i < r.as_len; i++) {
        if (r.as_path[i] == LOCAL_AS) {
            printf("Route rejected: AS_PATH contains local AS100.\n");
            return;
        }
    }

    routes[route_count++] = r;
    printf("Route added successfully.\n");
}

void display_all_routes() {
    if (route_count == 0) {
        printf("Routing table is empty.\n");
        return;
    }

    printf("\n--- All BGP Advertisements ---\n");

    for (int i = 0; i < route_count; i++) {
        printf("\nRoute %d:\n", i + 1);
        print_route(routes[i]);
    }
}

int get_best_route(uint32_t network, int len) {
    int best = -1;

    for (int i = 0; i < route_count; i++) {
        if (routes[i].network == network &&
            routes[i].prefix_len == len) {

            if (best == -1 || is_better(routes[i], routes[best]))
                best = i;
        }
    }

    return best;
}

void display_best_routes() {
    if (route_count == 0) {
        printf("No routes available.\n");
        return;
    }

    printf("\n--- Best BGP Routes ---\n");

    int number = 1;

    for (int i = 0; i < route_count; i++) {
        int already_displayed = 0;

        for (int j = 0; j < i; j++) {
            if (same_prefix(routes[i], routes[j])) {
                already_displayed = 1;
                break;
            }
        }

        if (already_displayed)
            continue;

        int best = get_best_route(routes[i].network,
                                  routes[i].prefix_len);

        int count = 0;
        int pref_diff = 0;
        int len_diff = 0;
        int hop_diff = 0;

        for (int j = 0; j < route_count; j++) {
            if (same_prefix(routes[i], routes[j])) {
                count++;

                if (routes[j].local_pref != routes[best].local_pref)
                    pref_diff = 1;
                else if (routes[j].as_len != routes[best].as_len)
                    len_diff = 1;

                if (routes[j].next_hop != routes[best].next_hop)
                    hop_diff = 1;
            }
        }

        char prefix[30], next_hop[INET_ADDRSTRLEN];

        prefix_to_string(routes[best].network,
                         routes[best].prefix_len, prefix);
        ip_to_string(routes[best].next_hop, next_hop);

        printf("\n%d. Prefix: %s\n", number++, prefix);
        printf("Best Next Hop: %s\n", next_hop);
        printf("LOCAL_PREF: %d\n", routes[best].local_pref);
        printf("AS_PATH: ");
        print_as_path(routes[best]);
        printf("\nReason: ");

        if (count == 1)
            printf("Only available route.\n");
        else if (pref_diff)
            printf("Higher LOCAL_PREF.\n");
        else if (len_diff)
            printf("Shorter AS_PATH.\n");
        else if (hop_diff)
            printf("Numerically smaller next-hop IP address.\n");
        else
            printf("All attributes are equal.\n");
    }
}

void forward_packet() {
    char destination[INET_ADDRSTRLEN];
    uint32_t dest_ip;

    printf("Destination IP: ");
    scanf("%15s", destination);

    if (!parse_ip(destination, &dest_ip)) {
        printf("Invalid destination IP.\n");
        return;
    }

    int best = -1;

    for (int i = 0; i < route_count; i++) {
        uint32_t mask = get_mask(routes[i].prefix_len);

        if ((dest_ip & mask) == routes[i].network) {
            if (best == -1 ||
                routes[i].prefix_len > routes[best].prefix_len ||
                (routes[i].prefix_len == routes[best].prefix_len &&
                 is_better(routes[i], routes[best]))) {
                best = i;
            }
        }
    }

    if (best == -1) {
        printf("No route found.\n");
        return;
    }

    printf("\n--- Packet Forwarding Result ---\n");
    print_route(routes[best]);
}

void withdraw_route() {
    char prefix[32], next_hop[INET_ADDRSTRLEN];
    uint32_t network, hop;
    int len;

    printf("Prefix: ");
    scanf("%31s", prefix);

    if (!parse_prefix(prefix, &network, &len)) {
        printf("Invalid prefix.\n");
        return;
    }

    printf("Next Hop: ");
    scanf("%15s", next_hop);

    if (!parse_ip(next_hop, &hop)) {
        printf("Invalid next-hop IP.\n");
        return;
    }

    for (int i = 0; i < route_count; i++) {
        if (routes[i].network == network &&
            routes[i].prefix_len == len &&
            routes[i].next_hop == hop) {

            for (int j = i; j < route_count - 1; j++)
                routes[j] = routes[j + 1];

            route_count--;
            printf("Route withdrawn successfully.\n");
            return;
        }
    }

    printf("Route not found.\n");
}

int main() {
    int choice;

    while (1) {
        printf("\nBGP ROUTING SIMULATOR (AS100)\n");
        printf("1. Add BGP Route\n");
        printf("2. Display All Routes\n");
        printf("3. Display Best Routes\n");
        printf("4. Forward Packet\n");
        printf("5. Withdraw Route\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
            break;

        switch (choice) {
            case 1:
                add_route();
                break;

            case 2:
                display_all_routes();
                break;

            case 3:
                display_best_routes();
                break;

            case 4:
                forward_packet();
                break;

            case 5:
                withdraw_route();
                break;

            case 6:
                printf("Exiting program.\n");
                return 0;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}

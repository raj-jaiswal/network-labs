#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAX_ROUTES 50
#define MAX_AS 20
#define LOCAL_AS 100

typedef struct {
    uint32_t net;
    int len;
    uint32_t next_hop;
    int local_pref;
    int as_path[MAX_AS];
    int as_len;
} Route;

Route routes[MAX_ROUTES];
int route_count = 0;

uint32_t parse_ip(const char *s) {
    unsigned int a, b, c, d;
    sscanf(s, "%u.%u.%u.%u", &a, &b, &c, &d);
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | d;
}

void print_ip(uint32_t ip) {
    printf("%u.%u.%u.%u", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
}

uint32_t get_mask(int len) {
    return len == 0 ? 0 : 0xFFFFFFFFu << (32 - len);
}

int is_better(Route a, Route b) {
    if (a.local_pref != b.local_pref) return a.local_pref > b.local_pref;
    if (a.as_len != b.as_len) return a.as_len < b.as_len;
    return a.next_hop < b.next_hop;
}

void print_route_details(Route r) {
    printf("Prefix: "); print_ip(r.net); printf("/%d\n", r.len);
    printf("Next Hop: "); print_ip(r.next_hop); printf("\n");
    printf("LOCAL_PREF: %d\nAS_PATH: ", r.local_pref);
    for (int i = 0; i < r.as_len; i++) printf("%d%s", r.as_path[i], (i == r.as_len - 1) ? "" : " ");
    printf("\n");
}

int get_best_for_prefix(uint32_t net, int len) {
    int best = -1;
    for (int i = 0; i < route_count; i++) {
        if (routes[i].net == net && routes[i].len == len) {
            if (best == -1 || is_better(routes[i], routes[best]))
                best = i;
        }
    }
    return best;
}

void add_route() {
    char p_str[32], hop_str[16], line[128];
    Route r = {0};

    printf("Prefix: "); scanf("%s", p_str);
    char ip_part[16];
    sscanf(p_str, "%[^/]/%d", ip_part, &r.len);
    r.net = parse_ip(ip_part) & get_mask(r.len);

    printf("Next Hop: "); scanf("%s", hop_str);
    r.next_hop = parse_ip(hop_str);

    printf("AS_PATH: ");
    while (getchar() != '\n'); // clear newline
    fgets(line, sizeof(line), stdin);

    char *token = strtok(line, " \r\n");
    while (token) {
        int as_num = 0;
        sscanf(token, "%d", &as_num);
        if (as_num == LOCAL_AS) {
            printf("Route rejected: AS_PATH contains local AS%d.\n", LOCAL_AS);
            return;
        }
        r.as_path[r.as_len++] = as_num;
        token = strtok(NULL, " \r\n");
    }

    printf("LOCAL_PREF: "); scanf("%d", &r.local_pref);

    routes[route_count++] = r;
    printf("Route added successfully.\n");
}

void display_all() {
    if (route_count == 0) { printf("Routing table is empty.\n"); return; }
    for (int i = 0; i < route_count; i++) {
        printf("\nRoute %d:\n", i + 1);
        print_route_details(routes[i]);
    }
}

void display_best() {
    if (route_count == 0) { printf("No routes available.\n"); return; }
    int count = 1;

    for (int i = 0; i < route_count; i++) {
        int seen = 0;
        for (int j = 0; j < i; j++)
            if (routes[i].net == routes[j].net && routes[i].len == routes[j].len) seen = 1;
        if (seen) continue;

        int b = get_best_for_prefix(routes[i].net, routes[i].len);
        int total = 0, diff_pref = 0, diff_as = 0;

        for (int j = 0; j < route_count; j++) {
            if (routes[j].net == routes[b].net && routes[j].len == routes[b].len) {
                total++;
                if (routes[j].local_pref != routes[b].local_pref) diff_pref = 1;
                else if (routes[j].as_len != routes[b].as_len) diff_as = 1;
            }
        }

        printf("\n%d. Prefix: ", count++);
        print_ip(routes[b].net); printf("/%d\nBest Next Hop: ", routes[b].len);
        print_ip(routes[b].next_hop);
        printf("\nReason: ");
        if (total == 1) printf("Only available route.\n");
        else if (diff_pref) printf("Higher LOCAL_PREF.\n");
        else if (diff_as) printf("Shorter AS_PATH.\n");
        else printf("Numerically smaller next-hop IP address.\n");
    }
}

void forward_packet() {
    char ip_str[16];
    printf("Destination IP: "); scanf("%s", ip_str);
    uint32_t dest = parse_ip(ip_str);

    int best = -1;
    for (int i = 0; i < route_count; i++) {
        if ((dest & get_mask(routes[i].len)) == routes[i].net) {
            if (best == -1 || routes[i].len > routes[best].len ||
               (routes[i].len == routes[best].len && is_better(routes[i], routes[best]))) {
                best = i;
            }
        }
    }

    if (best == -1) {
        printf("No route found.\n");
    } else {
        printf("Matched Prefix: "); print_ip(routes[best].net); printf("/%d\n", routes[best].len);
        printf("Selected Next Hop: "); print_ip(routes[best].next_hop); printf("\n");
        printf("LOCAL_PREF: %d\nAS_PATH: ", routes[best].local_pref);
        for (int i = 0; i < routes[best].as_len; i++) 
            printf("%d%s", routes[best].as_path[i], (i == routes[best].as_len - 1) ? "" : " ");
        printf("\n");
    }
}

void withdraw_route() {
    char p_str[32], hop_str[16], ip_part[16];
    int len;
    printf("Destination Prefix: "); scanf("%s", p_str);
    sscanf(p_str, "%[^/]/%d", ip_part, &len);
    uint32_t net = parse_ip(ip_part) & get_mask(len);

    printf("Next Hop: "); scanf("%s", hop_str);
    uint32_t hop = parse_ip(hop_str);

    for (int i = 0; i < route_count; i++) {
        if (routes[i].net == net && routes[i].len == len && routes[i].next_hop == hop) {
            for (int j = i; j < route_count - 1; j++) routes[j] = routes[j + 1];
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
        printf("\n1. Add BGP Route\n2. Display All Routes\n3. Display Best Routes\n");
        printf("4. Forward Packet\n5. Withdraw Route\n6. Exit\nEnter your choice: ");
        if (scanf("%d", &choice) != 1 || choice == 6) break;

        switch (choice) {
            case 1: add_route(); break;
            case 2: display_all(); break;
            case 3: display_best(); break;
            case 4: forward_packet(); break;
            case 5: withdraw_route(); break;
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}

#include <stdio.h>
#include <string.h>

#define MAX_ENTRIES 50

struct RouteEntry {
    unsigned int network;
    int prefix_len;
    int interface;
};

struct RouteEntry table[MAX_ENTRIES];
int table_size = 0;

unsigned int get_mask(int len) {
    if (len == 0) return 0;
    return 0xFFFFFFFFU << (32 - len);
}

int parse_ip(const char *str, unsigned int *ip) {
    unsigned int o1, o2, o3, o4;
    char extra;
    if (sscanf(str, "%u.%u.%u.%u%c", &o1, &o2, &o3, &o4, &extra) != 4) {
        return 0;
    }
    if (o1 > 255 || o2 > 255 || o3 > 255 || o4 > 255) {
        return 0;
    }
    *ip = (o1 << 24) | (o2 << 16) | (o3 << 8) | o4;
    return 1;
}

int parse_cidr(const char *str, unsigned int *ip, int *len) {
    unsigned int o1, o2, o3, o4;
    int p_len;
    char extra;
    if (sscanf(str, "%u.%u.%u.%u/%d%c", &o1, &o2, &o3, &o4, &p_len, &extra) != 5) {
        return 0;
    }
    if (o1 > 255 || o2 > 255 || o3 > 255 || o4 > 255 || p_len < 0 || p_len > 32) {
        return 0;
    }
    *ip = (o1 << 24) | (o2 << 16) | (o3 << 8) | o4;
    *len = p_len;
    return 1;
}

void print_ip(unsigned int ip) {
    printf("%u.%u.%u.%u", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
}

void add_initial_entry(const char *cidr, int interface) {
    unsigned int net;
    int len;
    parse_cidr(cidr, &net, &len);
    net = net & get_mask(len);
    table[table_size].network = net;
    table[table_size].prefix_len = len;
    table[table_size].interface = interface;
    table_size++;
}

void display_table() {
    printf("\n%-25s %s\n", "Network prefix (CIDR)", "Output interface");
    for (int i = 0; i < table_size; i++) {
        print_ip(table[i].network);
        printf("/%d", table[i].prefix_len);
        printf("\t\t\t  %d\n", table[i].interface);
    }
}

void lookup_destination() {
    char ip_str[50];
    unsigned int dest_ip;

    printf("Enter destination IP: ");
    scanf("%s", ip_str);

    if (!parse_ip(ip_str, &dest_ip)) {
        printf("Invalid IPv4 address.\n");
        return;
    }

    int best_idx = -1;
    int max_len = -1;
    int match_found = 0;

    printf("Matching entries:\n");
    for (int i = 0; i < table_size; i++) {
        unsigned int mask = get_mask(table[i].prefix_len);
        if ((dest_ip & mask) == table[i].network) {
            match_found = 1;
            print_ip(table[i].network);
            printf("/%d Interface %d\n", table[i].prefix_len, table[i].interface);

            if (table[i].prefix_len > max_len) {
                max_len = table[i].prefix_len;
                best_idx = i;
            }
        }
    }

    if (match_found && best_idx != -1) {
        printf("Longest matching prefix: ");
        print_ip(table[best_idx].network);
        printf("/%d\n", table[best_idx].prefix_len);
        printf("Selected output interface: %d\n", table[best_idx].interface);
    } else {
        printf("No matching route found.\n");
    }
}

void insert_entry() {
    if (table_size >= MAX_ENTRIES) {
        printf("Forwarding table is full.\n");
        return;
    }

    char cidr_str[50];
    int interface;
    unsigned int net;
    int len;

    printf("Enter network prefix in CIDR notation: ");
    scanf("%s", cidr_str);

    if (!parse_cidr(cidr_str, &net, &len)) {
        printf("Invalid CIDR notation or prefix length.\n");
        return;
    }

    printf("Enter output interface number: ");
    if (scanf("%d", &interface) != 1) {
        printf("Invalid interface number.\n");
        return;
    }

    net = net & get_mask(len);

    for (int i = 0; i < table_size; i++) {
        if (table[i].network == net && table[i].prefix_len == len) {
            table[i].interface = interface;
            printf("Existing entry updated successfully.\n");
            return;
        }
    }

    table[table_size].network = net;
    table[table_size].prefix_len = len;
    table[table_size].interface = interface;
    table_size++;
    printf("Entry inserted successfully.\n");
}

void delete_entry() {
    char cidr_str[50];
    unsigned int net;
    int len;

    printf("Enter network prefix in CIDR notation to delete: ");
    scanf("%s", cidr_str);

    if (!parse_cidr(cidr_str, &net, &len)) {
        printf("Invalid CIDR notation.\n");
        return;
    }

    net = net & get_mask(len);

    for (int i = 0; i < table_size; i++) {
        if (table[i].network == net && table[i].prefix_len == len) {
            for (int j = i; j < table_size - 1; j++) {
                table[j] = table[j + 1];
            }
            table_size--;
            printf("Entry deleted successfully.\n");
            return;
        }
    }

    printf("Entry not found.\n");
}

int main() {
    add_initial_entry("192.168.0.0/16", 1);
    add_initial_entry("192.168.10.0/24", 2);
    add_initial_entry("192.168.10.128/25", 3);
    add_initial_entry("10.0.0.0/8", 4);
    add_initial_entry("0.0.0.0/0", 5);

    int choice;
    while (1) {
        printf("\n1. Display forwarding table\n");
        printf("2. Lookup destination IP\n");
        printf("3. Insert forwarding entry\n");
        printf("4. Delete forwarding entry\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
            break;

        if (choice == 1)
            display_table();
        else if (choice == 2)
            lookup_destination();
        else if (choice == 3)
            insert_entry();
        else if (choice == 4)
            delete_entry();
        else if (choice == 5)
            break;
        else
            printf("Invalid choice. Try again.\n");
    }

    return 0;
}

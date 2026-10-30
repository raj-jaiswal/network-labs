#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF 1024

int main() {
    char ip[50];
    int tcp_port, udp_port, choice;

    printf("Enter server IP address: ");
    scanf("%49s", ip);

    printf("Enter TCP server port: ");
    scanf("%d", &tcp_port);

    printf("Enter UDP status port: ");
    scanf("%d", &udp_port);

    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) { perror("TCP Socket"); exit(1); }

    struct sockaddr_in tcp_addr;
    memset(&tcp_addr, 0, sizeof(tcp_addr));

    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(tcp_port);

    if (inet_pton(AF_INET, ip, &tcp_addr.sin_addr) <= 0) {
        perror("Invalid IP");
        close(tcp_fd);
        exit(1);
    }

    if (connect(tcp_fd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0) {
        perror("Connection failed");
        close(tcp_fd);
        exit(1);
    }

    printf("Connected to TCP server.\n");

    while (1) {
        printf("\n1. DNS Lookup\n2. Server Status\n3. Exit\nEnter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            char host[100], type[10], query[BUF], response[BUF];

            printf("Enter domain: ");
            scanf("%99s", host);

            printf("Enter record type (A/CNAME/MX/NS): ");
            scanf("%9s", type);

            snprintf(query, sizeof(query), "%s %s", host, type);
            send(tcp_fd, query, strlen(query) + 1, 0);

            int bytes = recv(tcp_fd, response, BUF - 1, 0);
            if (bytes <= 0) {
                printf("Server disconnected.\n");
                break;
            }

            response[bytes] = '\0';
            printf("Server response: %s\n", response);
        }
        else if (choice == 2) {
            int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (udp_fd < 0) { perror("UDP Socket"); continue; }

            struct sockaddr_in udp_addr;
            memset(&udp_addr, 0, sizeof(udp_addr));

            udp_addr.sin_family = AF_INET;
            udp_addr.sin_port = htons(udp_port);
            inet_pton(AF_INET, ip, &udp_addr.sin_addr);

            char request[] = "STATUS", response[BUF];
            sendto(udp_fd, request, strlen(request) + 1, 0, (struct sockaddr *)&udp_addr, sizeof(udp_addr));

            socklen_t len = sizeof(udp_addr);
            int bytes = recvfrom(udp_fd, response, BUF - 1, 0, (struct sockaddr *)&udp_addr, &len);

            if (bytes < 0) perror("UDP Receive");
            else {
                response[bytes] = '\0';
                printf("UDP Server response: %s\n", response);
            }

            close(udp_fd);
        }
        else if (choice == 3) {
            send(tcp_fd, "EXIT", 5, 0);
            break;
        }
        else {
            printf("Invalid choice.\n");
        }
    }

    close(tcp_fd);
    printf("Disconnected from server.\n");

    return 0;
}

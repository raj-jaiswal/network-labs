#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF 1024

int main() {
    int sock_fd, port;
    char ip[50], hostname[100], type[10], query[BUF], response[BUF];
    struct sockaddr_in server_addr;

    printf("Enter server IP address: ");
    scanf("%49s", ip);

    printf("Enter server port number: ");
    scanf("%d", &port);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("Socket"); exit(1); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP");
        close(sock_fd);
        exit(1);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock_fd);
        exit(1);
    }

    printf("Connected to server.\n");

    while (1) {
        printf("\nEnter domain: ");
        scanf("%99s", hostname);

        if (!strcmp(hostname, "EXIT")) {
            send(sock_fd, "EXIT", 5, 0);
            break;
        }

        printf("Enter record type (A/CNAME/MX/NS): ");
        scanf("%9s", type);

        snprintf(query, sizeof(query), "%s %s", hostname, type);
        send(sock_fd, query, strlen(query) + 1, 0);

        int bytes = recv(sock_fd, response, BUF - 1, 0);
        if (bytes <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        response[bytes] = '\0';
        printf("Server response: %s\n", response);
    }

    close(sock_fd);
    printf("Disconnected from server.\n");

    return 0;
}


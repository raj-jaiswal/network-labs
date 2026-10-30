#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF 1024

typedef struct {
    char host[100], type[10], value[100];
} Record;

Record records[] = {
    {"www.example.com", "A", "192.168.1.10"},
    {"mail.example.com", "A", "192.168.1.20"},
    {"web.example.com", "CNAME", "www.example.com"},
    {"example.com", "MX", "mail.example.com"},
    {"example.com", "NS", "ns1.example.com"}
};

int n = sizeof(records) / sizeof(records[0]);

void lookup(char *host, char *type, char *response) {
    for (int i = 0; i < n; i++)
        if (!strcmp(records[i].host, host) && !strcmp(records[i].type, type)) {
            strcpy(response, records[i].value);
            return;
        }
    strcpy(response, "Record not found");
}

void *client_handler(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(fd, (struct sockaddr *)&addr, &len);

    printf("\nClient connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    char buf[BUF], response[BUF];

    while (1) {
        int bytes = recv(fd, buf, BUF - 1, 0);
        if (bytes <= 0) break;

        buf[bytes] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        if (!strcmp(buf, "EXIT")) break;

        char host[100], type[10];

        if (sscanf(buf, "%99s %9s", host, type) != 2)
            strcpy(response, "Invalid query");
        else {
            printf("Query: %s %s\n", host, type);
            lookup(host, type, response);
        }

        send(fd, response, strlen(response) + 1, 0);
    }

    close(fd);
    printf("Client disconnected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    return NULL;
}

int main() {
    int server_fd, port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);

    printf("Enter port number: ");
    scanf("%d", &port);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("Socket"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind"); exit(1);
    }

    listen(server_fd, 5);
    printf("Server listening on port %d...\n", port);

    while (1) {
        int fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (fd < 0) continue;

        int *p = malloc(sizeof(int));
        *p = fd;

        pthread_t t;
        pthread_create(&t, NULL, client_handler, p);
        pthread_detach(t);
    }

    close(server_fd);
    return 0;
}


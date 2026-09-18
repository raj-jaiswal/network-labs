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
int connected_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void lookup(char *host, char *type, char *response) {
    for (int i = 0; i < n; i++)
        if (!strcmp(records[i].host, host) && !strcmp(records[i].type, type)) {
            strcpy(response, records[i].value);
            return;
        }
    strcpy(response, "Record not found");
}

void *handle_client(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(fd, (struct sockaddr *)&addr, &len);

    printf("\nClient connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    pthread_mutex_lock(&mutex);
    connected_clients++;
    pthread_mutex_unlock(&mutex);

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

    pthread_mutex_lock(&mutex);
    connected_clients--;
    pthread_mutex_unlock(&mutex);

    printf("Client disconnected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return NULL;
}

void *udp_server(void *arg) {
    int port = *(int *)arg;
    free(arg);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("UDP Socket"); return NULL; }

    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("UDP Bind");
        close(fd);
        return NULL;
    }

    printf("UDP server listening on port %d...\n", port);

    char buf[BUF], response[BUF];
    while (1) {
        int bytes = recvfrom(fd, buf, BUF - 1, 0, (struct sockaddr *)&client, &len);
        if (bytes < 0) continue;

        buf[bytes] = '\0';
        printf("UDP request: %s\n", buf);

        if (!strcmp(buf, "STATUS")) {
            pthread_mutex_lock(&mutex);
            int count = connected_clients;
            pthread_mutex_unlock(&mutex);
            snprintf(response, BUF, "Server active. Connected clients: %d", count);
        } else
            strcpy(response, "Invalid request. Send STATUS");

        sendto(fd, response, strlen(response) + 1, 0, (struct sockaddr *)&client, len);
    }
}

int main() {
    int tcp_port, udp_port;

    printf("Enter TCP port number: ");
    scanf("%d", &tcp_port);

    printf("Enter UDP port number: ");
    scanf("%d", &udp_port);

    int *p = malloc(sizeof(int));
    *p = udp_port;

    pthread_t udp_thread;
    pthread_create(&udp_thread, NULL, udp_server, p);
    pthread_detach(udp_thread);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("TCP Socket"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(tcp_port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("TCP Bind");
        exit(1);
    }

    listen(server_fd, 5);

    printf("TCP server listening on port %d...\n", tcp_port);

    while (1) {
        int fd = accept(server_fd, (struct sockaddr *)&client, &len);
        if (fd < 0) continue;

        int *p = malloc(sizeof(int));
        *p = fd;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, p);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}

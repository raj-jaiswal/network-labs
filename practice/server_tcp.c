#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

int clients = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle(void *arg) {
    int s = *(int *)arg;
    char buf[100];

    pthread_mutex_lock(&lock);
    printf("Clients: %d\n", ++clients);
    pthread_mutex_unlock(&lock);

    read(s, buf, sizeof(buf));
    printf("Client: %s", buf);

    write(s, "Hello from Server\n", 19);
    close(s);

    pthread_mutex_lock(&lock);
    printf("Clients: %d\n", --clients);
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main() {
    int port, server, client;
    struct sockaddr_in addr;
    pthread_t t;

    printf("Port: ");
    scanf("%d", &port);

    server = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 10);

    while (1) {
        client = accept(server, NULL, NULL);
        pthread_create(&t, NULL, handle, &client);
        pthread_detach(t);
    }
}

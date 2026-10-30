#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

int clients = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char cname[100];
    int val;
} DNS;

DNS dns[5] = {{"ABC", 34}, {"www.gooogle.com", 64}, {"google", 23}, {"RAJ", 69}, {"WOW", 67}};

int findDNS(char buffer[100]){
    for(int i=0; i<5; i++){
        if(strcmp(dns[i].cname, buffer) == 0){
            return dns[i].val;
        }
    }

    return -1;
}

void *handle(void *arg){
    int s = *(int*) arg;
    free(arg);
    char buffer[100];
    memset(buffer, 0, 100);

    pthread_mutex_lock(&lock);
    printf("Clients: %d\n", ++clients);
    pthread_mutex_unlock(&lock);

    while(strcmp(buffer, "EXIT") != 0){
        read(s, buffer, 100);
        printf("Client Request: %s | ", buffer);

        if(strcmp(buffer, "EXIT") == 0){
            break;
        }

        int result = -1;
        result = findDNS(buffer);
        printf("Response: %d\n", result);
        write(s, &result, sizeof(int));
    }

    close(s);
    pthread_mutex_lock(&lock);
    printf("Disconnected. Clients: %d\n", --clients);
    pthread_mutex_unlock(&lock);

    fflush(stdout);
    return NULL;
}

void *status(void *args){
    int port, s;
    int res;
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);

    printf("UDP Port: ");
    scanf("%d", &port);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    while(1){
        recvfrom(s, &res, sizeof(int), 0, (struct sockaddr *)&client, &len);
        printf("Status Request\n");
        sendto(s, &clients, sizeof(int), 0, (struct sockaddr *)&client, len);
    }
}

int main(){
    int port, server;
    struct sockaddr_in addr;
    pthread_t t;

    printf("TCP Port: ");
    scanf("%d", &port);

    pthread_create(&t, NULL, status, NULL);
    pthread_detach(t);

    server = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 10);

    while(1){
        int *client = malloc(sizeof(int));
        *client = accept(server, NULL, NULL);
        pthread_create(&t, NULL, handle, client);
        pthread_detach(t);
    }
}

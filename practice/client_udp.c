#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int port, s;
    char ip[50], buf[100];
    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    printf("IP: ");
    scanf("%s", ip);
    printf("Port: ");
    scanf("%d", &port);

    s = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server.sin_addr);

    sendto(s, "Hi from Client\n", 15, 0,
           (struct sockaddr *)&server, len);

    recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);

    printf("Server: %s", buf);

    close(s);
}

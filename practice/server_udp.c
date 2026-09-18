#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int port, s;
    char buf[100];
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);

    printf("Port: ");
    scanf("%d", &port);

    s = socket(AF_INET, SOCK_DGRAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    while (1) {
        recvfrom(s, buf, sizeof(buf), 0,
                 (struct sockaddr *)&client, &len);

        printf("Client: %s", buf);

        sendto(s, "Hello from Server\n", 19, 0,
               (struct sockaddr *)&client, len);
    }
}

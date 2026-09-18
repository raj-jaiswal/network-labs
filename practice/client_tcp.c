#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    char ip[50], buf[100];
    int port, s;
    struct sockaddr_in addr;

    printf("IP: ");
    scanf("%s", ip);
    printf("Port: ");
    scanf("%d", &port);

    s = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    connect(s, (struct sockaddr *)&addr, sizeof(addr));

    write(s, "Hi from Client\n", 15);
    read(s, buf, sizeof(buf));

    printf("Server: %s", buf);
    close(s);
}

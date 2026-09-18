#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock_fd;
    char ip[50];
    int port, n, i;
    int values[1000];
    int min, max;
    long long sum;
    double average;
    struct sockaddr_in server_addr;

    printf("Enter server IP address: ");
    scanf("%s", ip);

    printf("Enter server port number: ");
    scanf("%d", &port);

    printf("Enter number of integers: ");
    scanf("%d", &n);

    printf("Enter integers: ");
    for (i = 0; i < n; i++)
        scanf("%d", &values[i]);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Connected to server.\n");

    int temp_n = htonl(n);
    send(sock_fd, &temp_n, sizeof(temp_n), 0);

    for (i = 0; i < n; i++)
        values[i] = htonl(values[i]);

    send(sock_fd, values, n * sizeof(int), 0);

    recv(sock_fd, &min, sizeof(min), 0);
    recv(sock_fd, &max, sizeof(max), 0);
    recv(sock_fd, &sum, sizeof(sum), 0);
    recv(sock_fd, &average, sizeof(average), 0);

    printf("Minimum = %d\n", min);
    printf("Maximum = %d\n", max);
    printf("Sum = %lld\n", sum);
    printf("Average = %.2f\n", average);

    close(sock_fd);

    return 0;
}

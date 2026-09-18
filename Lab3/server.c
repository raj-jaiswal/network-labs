#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int server_fd, client_fd;
    int port, n, i;
    int values[1000];
    int min, max;
    long long sum;
    double average;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Enter port number: ");
    scanf("%d", &port);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("Server listening on port %d...\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        recv(client_fd, &n, sizeof(n), 0);

        n = ntohl(n);

        recv(client_fd, values, n * sizeof(int), 0);

        for (i = 0; i < n; i++)
            values[i] = ntohl(values[i]);

        printf("Integers received: ");
        for (i = 0; i < n; i++)
            printf("%d ", values[i]);
        printf("\n");

        min = max = values[0];
        sum = 0;

        for (i = 0; i < n; i++) {
            if (values[i] < min)
                min = values[i];

            if (values[i] > max)
                max = values[i];

            sum += values[i];
        }

        average = (double)sum / n;

        printf("Minimum = %d\n", min);
        printf("Maximum = %d\n", max);
        printf("Sum = %lld\n", sum);
        printf("Average = %.2f\n", average);

        send(client_fd, &min, sizeof(min), 0);
        send(client_fd, &max, sizeof(max), 0);
        send(client_fd, &sum, sizeof(sum), 0);
        send(client_fd, &average, sizeof(average), 0);

        close(client_fd);

        printf("Client disconnected.\n");
    }

    close(server_fd);

    return 0;
}

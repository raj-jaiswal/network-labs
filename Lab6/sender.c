#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define TIMEOUT 2

int main() {
    int sockfd;
    struct sockaddr_in receiverAddr;
    socklen_t receiverLen = sizeof(receiverAddr);

    char buffer[BUFFER_SIZE];
    char ack[BUFFER_SIZE];

    char ip[50];
    int port;
    int numMessages;

    printf("Enter receiver IP address: ");
    scanf("%49s", ip);

    printf("Enter receiver port number: ");
    scanf("%d", &port);

    printf("Enter number of messages: ");
    scanf("%d", &numMessages);

    getchar();

    char messages[numMessages][BUFFER_SIZE];
    for (int i = 0; i < numMessages; i++) {
        printf("Enter message %d: ", i + 1);
        fgets(messages[i], BUFFER_SIZE, stdin);

        messages[i][strcspn(messages[i], "\n")] = '\0';
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&receiverAddr, 0, sizeof(receiverAddr));

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(port);

    inet_pton(AF_INET, ip, &receiverAddr.sin_addr);
    printf("\nUDP RELIABLE DATA TRANSFER\n\n");

    int seq = 0;
    for (int i = 0; i < numMessages; i++) {
        sprintf(buffer, "%d|%s", seq, messages[i]);

        int acknowledged = 0;
        while (!acknowledged) {
            printf("Sending Packet: Seq=%d, Data=\"%s\"\n", seq, messages[i]);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&receiverAddr, receiverLen);
            printf("Packet sequence number transmitted: %d\n", seq);

            fd_set readfds;
            struct timeval timeout;

            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);

            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = 0;

            int result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

            if (result == 0) {
                printf("TIMEOUT: ACK for Seq=%d not received\n", seq);
                printf("RETRANSMITTING Packet Seq=%d\n\n", seq);
                continue;
            }

            memset(ack, 0, BUFFER_SIZE);
            int n = recvfrom(sockfd, ack, BUFFER_SIZE - 1, 0, NULL, NULL);

            ack[n] = '\0';
            printf("ACK received: %s\n", ack);

            int ackSeq;
            if (sscanf(ack, "ACK|%d", &ackSeq) == 1) {
                if (ackSeq == seq) {
                    printf("Correct ACK received for Seq=%d\n", seq);
                    acknowledged = 1;
                } else {
                    printf("Wrong ACK received: Seq=%d\n", ackSeq);
                    printf("Waiting for correct ACK...\n");
                }
            }

            printf("\n");
        }

        seq = 1 - seq;
    }

    printf("All messages transmitted successfully.\n");
    close(sockfd);

    return 0;
}

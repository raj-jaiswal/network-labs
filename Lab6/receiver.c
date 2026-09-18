#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    char buffer[BUFFER_SIZE];
    char ack[BUFFER_SIZE];

    int port;
    int expectedSeq = 0;
    int ackCount = 0;

    printf("Enter receiver port number: ");
    scanf("%d", &port);
    getchar();

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    printf("\nReceiver started on port %d...\n", port);
    printf("Waiting for packets...\n\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&clientAddr, &clientLen);

        buffer[n] = '\0';

        int seq;
        char data[BUFFER_SIZE];

        sscanf(buffer, "%d|%[^\n]", &seq, data);
        printf("Packet received: Seq=%d, Data=\"%s\"\n", seq, data);

        ackCount++;
        if (seq == expectedSeq) {
	    printf("Correct packet. Delivering data: \"%s\"\n", data);
            printf("Message delivered successfully.\n");

            sprintf(ack, "ACK|%d", seq);

            if(ackCount % 5 == 0) {
                printf("SIMULATED ACK LOSS: ACK %d discarded\n",
                       seq);
            }
	    else {
                sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&clientAddr, clientLen);
                printf("ACK sent: ACK|%d\n", seq);
                expectedSeq = 1 - expectedSeq;
            }
        }
	else {
            printf("DUPLICATE PACKET DETECTED: Seq=%d\n", seq);
            printf("Data NOT delivered again.\n");

            sprintf(ack, "ACK|%d", seq);
            if (ackCount % 5 == 0) {
                printf("SIMULATED ACK LOSS: ACK %d discarded\n", seq);
            }
	    else {
                sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&clientAddr, clientLen);
                printf("ACK resent: ACK|%d\n", seq);
            }
        }

        printf("\n");
    }

    close(sockfd);
    return 0;
}

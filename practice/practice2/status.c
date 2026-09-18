#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    char ip[50];
    int res;
    int port, s;

    struct sockaddr_in addr;

    printf("IP: ");
    scanf("%s", ip);
    printf("UDP Port: ");
    scanf("%d", &port);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int idx = 0;
    sendto(s, &idx, sizeof(int), 0, (struct sockaddr *)&addr, sizeof(addr));

    int count = 0;
    while(res != -1){
        recvfrom(s, &res, sizeof(int), 0, NULL, NULL);
        printf("Connected Clients: %d\n", res);

        idx = 1;
        if(count % 5 != 0) sendto(s, &idx, sizeof(int), 0, (struct sockaddr *)&addr, sizeof(addr));
        count++;
    }

    close(s);
}

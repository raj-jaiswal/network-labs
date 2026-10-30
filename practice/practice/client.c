#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main(){
    char ip[50];
    int res;
    int port, s;

    struct sockaddr_in addr;

    printf("IP: ");
    scanf("%s", ip);
    printf("TCP Port: ");
    scanf("%d", &port);

    s = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    connect(s, (struct sockaddr *)&addr, sizeof(addr));

    char buffer[100];
    memset(buffer, 0, 100);
    while(strcmp(buffer, "EXIT") != 0){
        printf("Enter CNAME: ");
        scanf(" %[^\n]", buffer);

        if(strcmp(buffer, "EXIT") == 0){
            write(s, buffer, 100);
            break;
        }

        write(s, buffer, 100);
        read(s, &res, sizeof(int));
        printf("Server: %d\n", res);
    }

    close(s);
}

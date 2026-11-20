// BGP Longest Prefix Match Simulation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char CIDR[50];
    u_int32_t prefix;
    int subnet_mask;
    char nextHop[50];
    int ASPath[50];
    int nASPath;
    int localPref;
} AS;

typedef struct {
    char CIDR[50];
    u_int32_t prefix;
    int subnet_mask;
    char nextHop[50];
    char reason[50];
} bestRoute;

AS bgpTable[100];
int nBGP = 0;

bestRoute bestRoutes[100];
int nBest = 0;

void addBGP(){
    if (nBGP >= 100) {
        printf("BGP table is full. Cannot insert new entry.\n");
        return;
    }

    printf("Destination prefix (CIDR notation): ");
    char cidr[50];
    scanf("%s", cidr);

    int a, b, c, d, mask;
    sscanf(cidr, "%d.%d.%d.%d/%d", &a, &b, &c, &d, &mask);

    bgpTable[nBGP].subnet_mask = mask;
    u_int32_t prefix = (a << 24) | (b << 16) | (c << 8) | d;
    prefix &= ((mask == 0) ? 0 : 0xFFFFFFFF << (32 - mask));

    bgpTable[nBGP].prefix = prefix;
    sprintf(bgpTable[nBGP].CIDR, "%d.%d.%d.%d/%d", prefix >> 24, (prefix >> 16) & 0xFF, (prefix >> 8) & 0xFF, prefix & 0xFF, mask);

    printf("Next hop IP address: ");
    scanf("%s", bgpTable[nBGP].nextHop);

    char asPath[100];
    printf("AS path (space-separated AS numbers: ");
    scanf(" %[^\n]", asPath);

    int val = 0;
    bgpTable[nBGP].nASPath = 0;

    for(int i=0; i<strlen(asPath); i++){
        if(asPath[i] == ' '){
            bgpTable[nBGP].ASPath[bgpTable[nBGP].nASPath++] = val;
            if(val == 100) {
                printf("Error: AS number 100 is reserved and cannot be used in the AS path.\n");
                return;
            }
            val = 0;
            continue;
        }
        
        val = val * 10 + (asPath[i] - '0');
    }
    bgpTable[nBGP].ASPath[bgpTable[nBGP].nASPath++] = val;
    if(val == 100) {
        printf("Error: AS number 100 is reserved and cannot be used in the AS path.\n");
        return;
    }

    printf("LOCAL_PREF: ");
    scanf("%d", &bgpTable[nBGP].localPref);

    nBGP++;
}

void displayBGP(){
    printf("BGP Table:\n");
    for(int i=0; i<nBGP; i++){
        printf("%s, Next Hop: %s, AS Path: ", bgpTable[i].CIDR, bgpTable[i].nextHop);
        for(int j=0; j<bgpTable[i].nASPath; j++){
            printf("%d", bgpTable[i].ASPath[j]);
            if(j < bgpTable[i].nASPath - 1) printf(" ");
        }
        printf(", LOCAL_PREF: %d\n", bgpTable[i].localPref);
    }
}

void computeBestRoutes() {
    u_int32_t uniquePrefixes[100];
    int uniqueCount = 0;
    for(int i=0; i<nBGP; i++){
        int found = 0;
        for(int j=0; j<uniqueCount; j++){
            if(bgpTable[i].prefix == uniquePrefixes[j]){
                found = 1;
                break;
            }
        }
        if(!found){
            uniquePrefixes[uniqueCount++] = bgpTable[i].prefix;
        }
    }

    nBest = 0;
    for(int i=0; i<uniqueCount; i++){
        int bestIndex = -1;
        for(int j=0; j<nBGP; j++){
            if(bgpTable[j].prefix == uniquePrefixes[i]){
                if(bestIndex == -1 || bgpTable[j].localPref > bgpTable[bestIndex].localPref || 
                   (bgpTable[j].localPref == bgpTable[bestIndex].localPref && bgpTable[j].nASPath < bgpTable[bestIndex].nASPath) || 
                   (bgpTable[j].localPref == bgpTable[bestIndex].localPref && bgpTable[j].nASPath == bgpTable[bestIndex].nASPath && strcmp(bgpTable[j].nextHop, bgpTable[bestIndex].nextHop) < 0)){
                    bestIndex = j;
                }
            }
        }
        if(bestIndex != -1){
            bestRoutes[nBest].prefix = bgpTable[bestIndex].prefix;
            bestRoutes[nBest].subnet_mask = bgpTable[bestIndex].subnet_mask;
            strcpy(bestRoutes[nBest].CIDR, bgpTable[bestIndex].CIDR);
            strcpy(bestRoutes[nBest].nextHop, bgpTable[bestIndex].nextHop);
            nBest++;
        }
    }
}

void displayBestRoutes() {
    printf("Best Routes:\n");
    for(int i=0; i<nBest; i++){
        printf("%s, Next Hop: %s\n", bestRoutes[i].CIDR, bestRoutes[i].nextHop);
    }
}

void deleteBGP() {
    char cidr[50];
    char nextHop[50];
    printf("Enter the CIDR of the route to withdraw: ");
    scanf("%s", cidr);

    printf("Enter the next hop of the route to withdraw: ");
    scanf("%s", nextHop);

    int found = 0;
    for(int i=0; i<nBGP; i++){
        if(strcmp(bgpTable[i].CIDR, cidr) == 0 && strcmp(bgpTable[i].nextHop, nextHop) == 0){
            for(int j=i; j<nBGP-1; j++){
                bgpTable[j] = bgpTable[j+1];
            }
            nBGP--;
            found = 1;
            printf("Route %s with next hop %s withdrawn successfully.\n", cidr, nextHop);
            break;
        }
    }
    if(!found){
        printf("Route %s with next hop %s not found in BGP table.\n", cidr, nextHop);
    }
}

void forwardPacket(){
    char destIP[50];
    printf("Enter the destination IP address: ");
    scanf("%s", destIP);

    int a, b, c, d;
    sscanf(destIP, "%d.%d.%d.%d", &a, &b, &c, &d);
    u_int32_t dest = (a << 24) | (b << 16) | (c << 8) | d;

    int bestIndex = -1;
    for(int i=0; i<nBest; i++){
        u_int32_t mask = (bestRoutes[i].subnet_mask == 0) ? 0 : 0xFFFFFFFF << (32 - bestRoutes[i].subnet_mask);
        if((dest & mask) == bestRoutes[i].prefix){
            if(bestIndex == -1 || bestRoutes[i].subnet_mask > bestRoutes[bestIndex].subnet_mask){
                bestIndex = i;
            }
        }
    }

    if(bestIndex != -1){
        printf("Matched Prefix: %s\n", bestRoutes[bestIndex].CIDR);
        printf("Packet forwarded to next hop: %s\n", bestRoutes[bestIndex].nextHop);
    } else {
        printf("No matching route found for destination IP: %s\n", destIP);
    }
}

int main() {
    printf("Choices: \n");
    printf("1. Add BGP Route\n");
    printf("2. Display All Routes\n");
    printf("3. Display Best Routes\n");
    printf("4. Forward Packet\n");
    printf("5. Withdraw Route\n");
    printf("6. Exit\n");

    int choice;
    printf("Enter Choice: ");
    scanf("%d", &choice);

    while (choice != 6) {
        switch (choice) {
            case 1:
                // Add BGP Route
                addBGP();
                break;
            case 2:
                // Display All Routes
                displayBGP();
                break;
            case 3:
                // Display Best Routes
                computeBestRoutes();
                displayBestRoutes();
                break;
            case 4:
                // Forward Packet
                forwardPacket();
                break;
            case 5:
                // Withdraw Route
                deleteBGP();
                computeBestRoutes();
                break;
            case 6:
                // Exit
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }

        printf("\nEnter Choice: ");
        scanf("%d", &choice);
    }
    return 0;
}
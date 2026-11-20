#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char ip[50];
    u_int32_t prefix;
    int subnet_mask;
    int interface;
} Entry;

Entry forwardingTable[100];
int nEntry = 0;

void insertEntry() {
    if (nEntry >= 100) {
        printf("Forwarding table is full. Cannot insert new entry.\n");
        return;
    }

    printf("Enter CIDR notation: ");
    char ip[50];
    scanf("%s", ip);

    int val = 0;
    for(int i=0; i<strlen(ip); i++){
        if(ip[i] == '.'){
            forwardingTable[nEntry].prefix = (forwardingTable[nEntry].prefix << 8);
            forwardingTable[nEntry].prefix |= val;

            val = 0;
            continue;
        }

        if(ip[i] == '/'){
            forwardingTable[nEntry].prefix = (forwardingTable[nEntry].prefix << 8);
            forwardingTable[nEntry].prefix |= val;

            val = 0;
            continue;
        }

        val = val * 10 +(ip[i] - '0');
        if(val > 255){
            printf("Invalid CIDR notation. Please try again.\n");
            return;
        }
    }

    forwardingTable[nEntry].subnet_mask = val;
    
    u_int32_t mask;
    if(forwardingTable[nEntry].subnet_mask == 0){
        mask = 0;
    } else {
        mask = ((u_int32_t) 0xFFFFFFFF) << (32 - forwardingTable[nEntry].subnet_mask);
    }
    
    forwardingTable[nEntry].prefix &= mask;

    u_int32_t prefix = forwardingTable[nEntry].prefix;
    int idx = 0;
    int octet;
    for(int i=0; i<4; i++){
        octet = prefix >> (24 - i * 8) & 0xFF;
        if(octet == 0) {
            forwardingTable[nEntry].ip[idx++] = '0';
        }
        char buffer[4];
        int bufferIdx = 0;

        while(octet > 0) {
            buffer[bufferIdx++] = (octet % 10) + '0';
            octet /= 10;
        }

        for(int j = bufferIdx - 1; j >= 0; j--) {
            forwardingTable[nEntry].ip[idx++] = buffer[j];
        }

        if(i < 3) {
            forwardingTable[nEntry].ip[idx++] = '.';
        }
    }

    forwardingTable[nEntry].ip[idx++] = '/';
    octet = forwardingTable[nEntry].subnet_mask;
    
    char buffer[4];
    int bufferIdx = 0;

    if(octet == 0) {
        forwardingTable[nEntry].ip[idx++] = '0';
    }

    while(octet > 0) {
        buffer[bufferIdx++] = (octet % 10) + '0';
        octet /= 10;
    }

    for(int j = bufferIdx - 1; j >= 0; j--) {
        forwardingTable[nEntry].ip[idx++] = buffer[j];
    }

    forwardingTable[nEntry].ip[idx] = '\0';

    printf("Enter Interface: ");
    scanf("%d", &forwardingTable[nEntry].interface);

    nEntry++;
    printf("Entry inserted successfully.\n");
}

void displayTable() {
    printf("Forwarding Table:\n");
    printf("IP Address\t\tPrefix\t\tSubnet Mask\tInterface\n");
    for (int i = 0; i < nEntry; i++) {
        printf("%s\t\t%u\t\t%d\t\t%d\n", forwardingTable[i].ip, forwardingTable[i].prefix, forwardingTable[i].subnet_mask, forwardingTable[i].interface);
    }
}

void deleteEntry() {
    if (nEntry == 0) {
        printf("Forwarding table is empty. Cannot delete entry.\n");
        return;
    }

    printf("Enter CIDR notation to delete: ");
    char ip[50];
    scanf("%s", ip);

    u_int32_t prefixToDelete = 0;
    int val = 0;
    for(int i=0; i<strlen(ip); i++){
        if(ip[i] == '.'){
            prefixToDelete = (prefixToDelete << 8);
            prefixToDelete |= val;

            val = 0;
            continue;
        }

        if(ip[i] == '/'){
            prefixToDelete = (prefixToDelete << 8);
            prefixToDelete |= val;

            val = 0;
            continue;
        }

        val = val * 10 + (ip[i] - '0');
        if(val > 255){
            printf("Invalid CIDR notation. Please try again.\n");
            return;
        }
    }



    int foundIndex = -1;
    for (int i = 0; i < nEntry; i++) {
        if (forwardingTable[i].prefix == prefixToDelete) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        for (int i = foundIndex; i < nEntry - 1; i++) {
            forwardingTable[i] = forwardingTable[i + 1];
        }
        nEntry--;
        printf("Entry deleted successfully.\n");
    } else {
        printf("Entry not found in the forwarding table.\n");
    }
}

void lookUp(){
    printf("Enter destination IP: ");
    char ip[50];
    scanf("%s", ip);

    u_int32_t prefixToLookup = 0;
    int val = 0;
    for(int i=0; i<strlen(ip); i++){
        if(ip[i] == '.'){
            prefixToLookup = (prefixToLookup << 8);
            prefixToLookup |= val;

            val = 0;
            continue;
        }
        
        val = val * 10 + (ip[i] - '0');
        if(val > 255){
            printf("Invalid IP address. Please try again.\n");
            return;
        }
    }

    prefixToLookup = (prefixToLookup << 8) | val;

    int foundIndex = -1;
    int longestPrefixLength = -1;

    printf("Matching Entries: \n");
    for (int i = 0; i < nEntry; i++) {
        u_int32_t mask;
        if(forwardingTable[i].subnet_mask == 0){
            mask = 0;
        } else {
            mask = ((u_int32_t) 0xFFFFFFFF) << (32 - forwardingTable[i].subnet_mask);
        }

        if ((prefixToLookup & mask) == forwardingTable[i].prefix) {

            printf("%s\tInterface: %d\n", forwardingTable[i].ip, forwardingTable[i].interface);
            if(forwardingTable[i].subnet_mask > longestPrefixLength) {
                longestPrefixLength = forwardingTable[i].subnet_mask;
                foundIndex = i;
            }
        }
    }

    if(foundIndex != -1) {
        printf("Longest Matching Prefix: %s/%d\tInterface: %d\n", forwardingTable[foundIndex].ip, forwardingTable[foundIndex].subnet_mask, forwardingTable[foundIndex].interface);
    } else {
        printf("No matching entry found.\n");
    }
}

int main(){
    int choice;
    printf("Choices:\n1. Display forwarding table\n2. Lookup destination IP\n3. Insert forwarding entry\n4. Delete forwarding entry\n5. Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    while(choice != 5){
        switch (choice){
            case 1:
                // Display forwarding table
                displayTable();
                break;
            case 2:
                // Lookup destination IP
                lookUp();
                break;
            case 3:
                // Insert forwarding entry
                insertEntry();
                break;
            case 4:
                // Delete forwarding entry
                deleteEntry();
                break;
            case 5:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }

        printf("Choice: ");
        scanf("%d", &choice);
    }

    return 0;
}
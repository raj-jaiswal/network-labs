#include <stdio.h>
#include <string.h>

// Function to perform Modulo-2 (XOR) division
void computeCRC(char data[], char poly[], char remainder[]) {
    int dataLen = strlen(data);
    int polyLen = strlen(poly);
    
    // Create a temporary array to hold data + appended zeros
    char temp[200];
    strcpy(temp, data);
    
    // Append (polyLen - 1) zeros to the end of data
    for (int i = 0; i < polyLen - 1; i++) {
        temp[dataLen + i] = '0';
    }
    temp[dataLen + polyLen - 1] = '\0';

    // Perform XOR division bit by bit
    for (int i = 0; i < dataLen; i++) {
        // Only divide if the current leading bit is '1'
        if (temp[i] == '1') {
            for (int j = 0; j < polyLen; j++) {
                // XOR operation: same bits -> '0', different bits -> '1'
                temp[i + j] = (temp[i + j] == poly[j]) ? '0' : '1';
            }
        }
    }

    // The last (polyLen - 1) bits of temp form the CRC remainder
    strncpy(remainder, temp + dataLen, polyLen - 1);
    remainder[polyLen - 1] = '\0';
}

int main() {
    char data[100], poly[50], crc[50], transmitted[150];

    printf("Enter binary data: ");
    scanf("%s", data);
    
    printf("Enter generator polynomial (binary): ");
    scanf("%s", poly);

    // 1. Sender Side: Calculate CRC bits
    computeCRC(data, poly, crc);

    // Append CRC bits to original data to form the transmitted frame
    strcpy(transmitted, data);
    strcat(transmitted, crc);

    printf("\n--- Sender Side ---\n");
    printf("CRC bits: %s\n", crc);
    printf("Transmitted frame (Data + CRC): %s\n", transmitted);

    // 2. Receiver Side (Optional Verification)
    char rxFrame[150], rxRemainder[50];
    printf("\n--- Receiver Side ---\n");
    printf("Enter received frame to check for errors: ");
    scanf("%s", rxFrame);

    computeCRC(rxFrame, poly, rxRemainder);

    // Check if the remainder contains any '1'
    int error = 0;
    for (int i = 0; rxRemainder[i] != '\0'; i++) {
        if (rxRemainder[i] == '1') {
            error = 1;
            break;
        }
    }

    printf("Remainder at receiver: %s\n", rxRemainder);
    if (error) {
        printf("Status: Error detected in received frame!\n");
    } else {
        printf("Status: No error detected (Frame accepted).\n");
    }

    return 0;
}
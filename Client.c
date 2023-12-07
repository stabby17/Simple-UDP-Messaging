#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(WIN32)
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>  // gethostbyname()
#endif

#include "protocol.h"

// Function to handle errors
void errorHandler(const char* errorMessage) {
    printf("%s\n", errorMessage);
}

// Function to clean up WinSock (if applicable)
void clearWinSock() {
#if defined(WIN32)
    WSACleanup();
#endif
}

void connectionSettings_input(char* input_host, char* input_port) {
    printf("Enter the server address or name: ");
    fgets(input_host, ECHOMAX, stdin);
    input_host[strcspn(input_host, "\n")] = 0; // Remove trailing newline character

    // Get the server port from the user
    printf("Enter the server port (input ! for default port): ");
    fgets(input_port, ECHOMAX, stdin);
    input_port[strcspn(input_port, "\n")] = 0; // Remove trailing newline character
    if (strcmp(input_port, "!") == 0) {
        sprintf(input_port, "%d", PORT);
    }
}

int main() {
#if defined(WIN32)
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("Error at WSAStartup\n");
        return EXIT_FAILURE;
    }
#endif

    int clientSocket;
    struct sockaddr_in serverAddress;   // server address structure
    struct in_addr* ina;                // hostname address
    char messageBuffer[ECHOMAX];
    int messageLength;

    // Get the server name and port from the user
    char input_host[ECHOMAX];
    char input_port[ECHOMAX];
    connectionSettings_input(input_host, input_port);

    // Get the server address
    struct hostent* hostname = gethostbyname(input_host);
    ina = (struct in_addr*)hostname->h_addr_list[0]; 

    // Create socket
    if ((clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        errorHandler("socket() failed");

    // Set up server address (Connections settings)
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;     // IPv4 address family
    serverAddress.sin_port = htons(atoi(input_port));   // Server port
    serverAddress.sin_addr = *ina;   // Server address

    // Print connection settings
    printf("Connecting to %s:%s\n", inet_ntoa(serverAddress.sin_addr), input_port);

    // Get user input for the message to send                                   
    printf("Enter the message to send to the server\n");
    fgets(messageBuffer, ECHOMAX, stdin);
    messageLength = strlen(messageBuffer);

    // Send the message to the server
    if (sendto(clientSocket, messageBuffer, messageLength, 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != messageLength)
        errorHandler("sendto() sent a different number of bytes than expected");

    // Receive the message from the server
    int serverAddressLength = sizeof(serverAddress);
    if ((messageLength = recvfrom(clientSocket, messageBuffer, ECHOMAX, 0, (struct sockaddr*)&serverAddress, &serverAddressLength)) != messageLength)
        errorHandler("recvfrom() failed");

    if (serverAddress.sin_addr.s_addr != serverAddress.sin_addr.s_addr)
        printf("Error: received a packet from unknown source.\n");

    // Output the received message
    messageBuffer[messageLength] = '\0';
    printf("Received back: %s\n", messageBuffer);

    closesocket(clientSocket);
    clearWinSock();

    return EXIT_SUCCESS;
}
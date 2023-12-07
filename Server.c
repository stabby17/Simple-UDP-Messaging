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

int main() {
#if defined(WIN32)
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("Error at WSAStartup\n");
        return EXIT_FAILURE;
    }
#endif

    int serverSocket;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    unsigned int clientAddressLength;
    char messageBuffer[ECHOMAX];
    int receivedMessageSize;

    // Create socket
    if ((serverSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        errorHandler("socket() failed");

    // Set up server address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // Print the server address
    char* ipAddress = inet_ntoa(serverAddress.sin_addr);
    if (serverAddress.sin_addr.s_addr == htonl(INADDR_ANY))
    {
        ipAddress = "[ANY ADDRESS]";
    }
    printf("Listening on %s:%d\n", ipAddress, ntohs(serverAddress.sin_port));

    // Bind the socket
    if ((bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) < 0)
        errorHandler("bind() failed");

    // Receive and echo messages from clients
    while (1) {
        printf("Waiting for client...\n");

        clientAddressLength = sizeof(clientAddress); // Set the size of the in-out parameter

        // The recvfrom() function receives a message from a connection-mode or connectionless-mode socket.
        receivedMessageSize = recvfrom(serverSocket, messageBuffer, ECHOMAX, 0, (struct sockaddr*)&clientAddress, &clientAddressLength);

        printf("Handling client %s\n", inet_ntoa(clientAddress.sin_addr));
        printf("Received: %s\n", messageBuffer);

        // Send the received message back to the client
        if (sendto(serverSocket, messageBuffer, receivedMessageSize, 0, (struct sockaddr*)&clientAddress,
            sizeof(clientAddress)) != receivedMessageSize)
        {
            errorHandler("sendto() sent a different number of bytes than expected");
        }

    }

    closesocket(serverSocket);
    clearWinSock();

    return EXIT_SUCCESS;
}
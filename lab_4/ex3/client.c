#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#define BUFSIZE 32
#define PORT 8080

void errExit(char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create a TCP socket
    if (sock == -1)
        errExit("socket");

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int c = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (c == -1)
        errExit("Error while establishing connection\n");

    printf("Enter message (max 32 characters): ");
    char msg[BUFSIZE];
    scanf(" %s", msg);
    int bytesSent = send(sock, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
        errExit("Error while sending the message");

    printf("Waiting for reply...\n");
    char recvBuffer[BUFSIZE];
    int bytesRecvd = recv(sock, recvBuffer, BUFSIZE - 1, 0);
    if (bytesRecvd <= 0)
        errExit("Error while receiving data from server");

    printf("Received from server: %s\n", recvBuffer);
    close(sock);
    return 0;
}
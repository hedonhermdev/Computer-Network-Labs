#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#define BUFSIZE 32
#define PORT 8080
#define MAXPENDING 5

void errExit(char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create a TCP socket
    if (sock == -1)
        errExit("socket error\n");

    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        errExit("Error while binding\n");

    if (listen(sock, MAXPENDING) == -1)
        errExit("Error in listen");

    printf("Listening for connections...\n");

    char msg[BUFSIZE];
    int clientLength = sizeof(clientAddr);
    int clientSocket = accept(sock, (struct sockaddr *)&clientAddr, (socklen_t *)&clientLength);
    if (clientSocket == -1)
        errExit("Error in client socket");

    printf("Handling Client %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    int n;
    if ((n = recv(clientSocket, msg, BUFSIZ, 0)) <= 0)
        errExit("recv error");

    msg[n] = '\0';
    printf("Message received: %s\n", msg);

    printf("Enter reply for client: ");
    scanf(" %s", msg);

    int bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
        errExit("Error while sending message to client");

    close(clientSocket);
    close(sock);
    return 0;
}
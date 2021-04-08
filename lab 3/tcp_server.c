#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#define BUFSIZE 32
#define PORT 8080
#define MAXPENDING 5

int main()
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // create a TCP socket
    if (sock < 0)
    {
        printf("Error\n");
        exit(0);
    }
    printf("Server socket created!\n");

    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");

    int temp = bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (temp < 0)
    {
        printf("Error while binding\n");
        exit(0);
    }
    printf("Binding successful\n");

    int temp1 = listen(sock, MAXPENDING);
    if (temp1 < 0)
    {
        printf("Error in listen");
        exit(0);
    }
    printf("Now Listening\n");

    char msg[BUFSIZE];
    int clientLength = sizeof(clientAddr);
    int clientSocket = accept(sock, (struct sockaddr *)&clientAddr, (socklen_t *)&clientLength);
    if (clientLength < 0)
    {
        printf("Error in client socket");
        exit(0);
    }
    printf("Handling Client %s\n", inet_ntoa(clientAddr.sin_addr));

    int temp2 = recv(clientSocket, msg, BUFSIZ, 0);
    if (temp2 < 0)
    {
        printf("problem in temp 2");
        exit(0);
    }
    printf("%s\n", msg);
    printf("ENTER MESSAGE FOR CLIENT\n");
    gets(msg);
    int bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
    {
        printf("Error while sending message to client");
        exit(0);
    }

    close(clientSocket);
    close(sock);
    return 0;
}
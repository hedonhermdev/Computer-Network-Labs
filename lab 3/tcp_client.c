#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#define BUFSIZE 32
#define PORT 8080

int main()
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // create a TCP socket
    if (sock < 0)
    {
        printf("Error\n");
        exit(0);
    }
    printf("Client socket created!\n");

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.29.202");
    printf("Address assigned\n");

    printf("Sending connection request\n");
    int c = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    printf("%d\n", c);
    if (c < 0)
    {
        printf("Error while establishing connection\n");
        exit(0);
    }
    printf("Connection Established\n");

    printf("ENTER MESSAGE FOR SERVER with max 32 characters\n");
    char msg[BUFSIZE];
    gets(msg);
    int bytesSent = send(sock, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
    {
        printf("Error while sending the message");
        exit(0);
    }
    printf("Data Sent\n");

    char recvBuffer[BUFSIZE];
    int bytesRecvd = recv(sock, recvBuffer, BUFSIZE - 1, 0);
    if (bytesRecvd < 0)
    {
        printf("Error while receiving data from server");
        exit(0);
    }
    recvBuffer[bytesRecvd] = '\0';
    printf("%s\n", recvBuffer);
    close(sock);
    return 0;
}
/* Client program Broken FTP */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

void errExit(const char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[256];
    char buff_offset[10];
    char buff_command[2];
    int offset;  // required to get the user input for offset in case of partial file command
    int command; // required to get the user input for command memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;
    int saddr_len = sizeof(serv_addr);
    /* Create a socket first */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        errExit("Error : Could not create socket \n");

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5001); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen("destination_file.txt", "ab");
    if (fp == NULL)
        errExit("Error opening file");

    fseek(fp, 0, SEEK_END);
    offset = ftell(fp);
    fclose(fp);

    fp = fopen("destination_file.txt", "ab");
    if (NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }
    printf("Enter (0) to get complete file, (1) to specify offset, (2) calculate the offset value from local file\n");
    scanf("%d", &command);
    sprintf(buff_command, "%d", command);

    // send command to server
    sendto(sockfd, buff_command, 2, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (command == 1 || command == 2) // We need to specify the offset
    {
        if (command == 1) // get the offset from the user
        {
            printf("Enter the value of File offset\n");
            scanf("%d", &offset);
        }
        // otherwise offset = size of local partial file, that we have already calculated
        sprintf(buff_offset, "%d", offset);
        /* sending the value of file offset */
        sendto(sockfd, buff_offset, 10, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    }

    // Else { command = 0 then no need to send the value of offset }
    /* Receive data in chunks of 256 bytes */
    while ((bytesReceived = recvfrom(sockfd, recvBuff, 256, 0, (struct sockaddr *)&serv_addr, (socklen_t *)&saddr_len)) > 0)
    {
        printf("Bytes received %d\n", bytesReceived);
        fwrite(recvBuff, 1, bytesReceived, fp);
        if (bytesReceived < 256)
            break;
    }
    if (bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }
    return 0;
}
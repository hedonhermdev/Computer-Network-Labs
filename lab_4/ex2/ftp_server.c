/* Server program for broken ftp using UDP */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

void errExit(const char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int listenfd = 0;
    struct sockaddr_in serv_addr, client_addr;
    int client_len = sizeof(client_addr);
    char sendBuff[1025];
    int numrv;
    listenfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (listenfd == -1)
        errExit("socket()");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5001);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        errExit("bind()");

    while (1)
    {
        char offset_buffer[10] = {'\0'};
        char command_buffer[2] = {'\0'};
        int offset;
        int command;

        printf("Waiting for client to send the command: Full File (0) or Partial File (1/2)...\n");
        while (recvfrom(listenfd, command_buffer, 2, 0, (struct sockaddr *)&client_addr, (socklen_t *)&client_len) == 0)
            ;
        sscanf(command_buffer, "%d", &command);
        if (command == 0)
            offset = 0;
        else
        {
            printf("Waiting for client to send the offset...\n");
            while (recvfrom(listenfd, command_buffer, 2, 0, (struct sockaddr *)&client_addr, (socklen_t *)&client_len) == 0)
                ;
            sscanf(offset_buffer, "%d", &offset);
        }

        /* Open the file that we wish to transfer */
        FILE *fp = fopen("source_file.txt", "rb");
        if (fp == NULL)
            errExit("fopen() error");

        /* Read data from file and send it */
        fseek(fp, offset, SEEK_SET);
        while (1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[256] = {0};
            int nread = fread(buff, 1, 256, fp);
            printf("Bytes read %d \n", nread);
            /* If read was success, send data. */
            if (nread > 0)
            {
                printf("Sending \n");
                sendto(listenfd, buff, nread, 0, (struct sockaddr *)&client_addr, client_len);
            }

            /*
            * There is something tricky going on with read ..
            * Either there was error, or we reached end of file.
            */
            if (nread < 256)
            {
                if (feof(fp))
                    printf("Reached end of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }

        sleep(1);
    }
    return 0;
}

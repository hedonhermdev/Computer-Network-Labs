#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#include <stdbool.h>
#include <time.h>
#define PACKET_SZ 100
#define PORT 8080
#define MAXPENDING 5
#define MAX_DATA_SIZE 1024

struct pkt
{
  int size;
  int seq_no;
  bool last_pkt;
  bool TYPE;
  char data[PACKET_SZ];
};

void errExit(char *err)
{
  perror(err);
  exit(EXIT_FAILURE);
}

bool dropPacket()
{
  return (rand() % 10 == 0);
}

int main()
{
  srand(time(0));
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

  char msg[PACKET_SZ * 2];
  int clientLength = sizeof(clientAddr);
  int clientSocket = accept(sock, (struct sockaddr *)&clientAddr, (socklen_t *)&clientLength);
  char *data_rcvd = (char *)calloc(MAX_DATA_SIZE, sizeof(char));
  memset(data_rcvd, '\0', MAX_DATA_SIZE);
  int offset = 0;

  FILE *fp = fopen("output.txt", "w");
  if (fp == NULL)
    errExit("fopen()");

  if (clientSocket == -1)
    errExit("Error in client socket");

  printf("Handling Client %s:%d\n\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

  for (;;)
  {

    int n;
    if ((n = recv(clientSocket, msg, BUFSIZ, 0)) <= 0)
      errExit("recv error");

    struct pkt *recv_pkt = (struct pkt *)msg;

    // drop the packet if true
    if (dropPacket())
    {
      printf("~ Dropping packet ~\n");
      continue;
    }

    printf("RCVD PACKET: Seq. No. = %d, Size = %d Bytes\n", recv_pkt->seq_no, recv_pkt->size);

    for (int i = 0; i < recv_pkt->size; i++)
    {
      data_rcvd[offset + i] = (recv_pkt->data)[i];
    }
    offset += recv_pkt->size;

    struct pkt ack_pkt;
    memset(&ack_pkt, 0, sizeof(ack_pkt));
    ack_pkt.seq_no = recv_pkt->seq_no;
    ack_pkt.TYPE = 1;

    int bytesSent = send(clientSocket, (char *)&ack_pkt, sizeof(ack_pkt), 0);
    if (bytesSent != sizeof(struct pkt))
      errExit("Error while sending message to client");

    printf("SENT ACK: Seq. No. = %d\n", ack_pkt.seq_no);

    if (recv_pkt->last_pkt)
    {
      if (fwrite(data_rcvd, sizeof(char), offset, fp) == -1)
        errExit("fwrite");
      break;
    }
  }

  free(data_rcvd);
  close(clientSocket);
  close(sock);
  fclose(fp);
  return 0;
}
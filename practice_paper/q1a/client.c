#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#include <stdbool.h>
#include <signal.h>
#define PACKET_SZ 100
#define PORT 8080
#define RTO 2

struct pkt
{
  int size;
  int seq_no;
  bool last_pkt;
  bool TYPE;
  char data[PACKET_SZ];
};

struct pkt my_pkt;
int sock;

void errExit(char *err)
{
  perror(err);
  exit(EXIT_FAILURE);
}

int sendPacket()
{
  int bytesSent = send(sock, (char *)&my_pkt, sizeof(my_pkt), 0);
  if (bytesSent <= 0)
    errExit("Error while sending the message");
  alarm(RTO);
  return bytesSent;
}

void alarmHandler(int sigNum)
{
  if (sigNum != SIGALRM)
    errExit("alarmHandler");
  sendPacket();
  printf("RE-TRANSMIT PKT: Seq. No. = %d, Size = %d Bytes\n", my_pkt.seq_no, my_pkt.size);
  alarm(RTO);
}

int main()
{
  signal(SIGALRM, alarmHandler);
  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create a TCP socket
  if (sock == -1)
    errExit("socket");

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  FILE *fp = fopen("input.txt", "r");
  if (fp == NULL)
    errExit("fopen()");

  char bytes_to_read = PACKET_SZ;

  int c = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  if (c == -1)
    errExit("Error while establishing connection\n");

  my_pkt.TYPE = 0;
  int bytes_sent = 0;

  for (;;)
  {
    int nread = fread(&(my_pkt.data), 1, bytes_to_read, fp);

    my_pkt.size = nread * sizeof(char);
    my_pkt.last_pkt = false;
    my_pkt.seq_no = bytes_sent;

    if (nread == 0)
    {
      break;
    }

    if (nread < bytes_to_read)
    {
      my_pkt.last_pkt = true;
    }

    int bytesSent = sendPacket();

    bytes_sent += nread;
    printf("SENT PKT: Seq. No. = %d, Size = %d Bytes\n", my_pkt.seq_no, my_pkt.size);

    char recvBuffer[PACKET_SZ * 2];
    int bytesRecvd = recv(sock, recvBuffer, PACKET_SZ * 2, 0);
    struct pkt *recv_pkt = (struct pkt *)&recvBuffer;

    alarm(0); // reset alarm
    printf("RCVD ACK: Seq. No. = %d\n", recv_pkt->seq_no);

    usleep(100000); // 100ms delay or 10^5 micro sec delay
  }

  close(sock);
  fclose(fp);
  return 0;
}
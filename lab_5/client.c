/*
 Simple udp client with stop and wait functionality
*/

#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h> // close
#define BUFLEN 512  // Max length of buffer
#define PORT 8882   // The port on which to send data
#define RTO 5       // round trip timeout in seconds

typedef struct packet1
{
  int sq_no;
} ACK_PKT;

typedef struct packet2
{
  int sq_no;
  char data[BUFLEN];
} DATA_PKT;

void die(char *s)
{
  perror(s);
  exit(1);
}

// if on, randomly discard ACK received
// that is, simulate data packet loss
int DEBUG = 1;
int flag = 0;
struct sockaddr_in si_other;
DATA_PKT send_pkt;
int s;

void sendPacket(int s, DATA_PKT send_pkt, struct sockaddr_in si_other)
{
  if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sizeof(si_other)) == -1)
  {
    die("sendto()");
  }
  alarm(RTO);
}

void sigAlrmHandler(int sig_num)
{
  if (sig_num != SIGALRM)
    die("unexpected signal");

  printf("<< Timed out, retransmitting packet... >>\n");
  sendPacket(s, send_pkt, si_other); // retransmit last sent packet
  alarm(5);                          // reset timeout alarm
  flag = 1;
}

int main(void)
{
  srand(time(NULL));
  signal(SIGALRM, sigAlrmHandler);

  int i, slen = sizeof(si_other);
  char buf[BUFLEN];
  char message[BUFLEN];
  ACK_PKT rcv_ack;
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    die("socket");
  }
  memset((char *)&si_other, 0, sizeof(si_other));

  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(PORT);
  si_other.sin_addr.s_addr = inet_addr("127.0.0.1");

  int state = 0;
  while (1)
  {
    switch (state)
    {
    case 0: // wait for sending packet with seq. no. 0
      printf("\nEnter message 0: ");
      fgets(send_pkt.data, sizeof(send_pkt), stdin);
      send_pkt.sq_no = 0;
      sendPacket(s, send_pkt, si_other);
      state = 1;
      break;

    case 1: // waiting for ACK 0
      if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, (socklen_t *)&slen) == -1)
      {
        if (errno == EINTR && flag == 1)
        {
          flag = 0;
          state = 1;
          break;
        }
        die("recvfrom()");
      }

      if (DEBUG && rand() % 5 == 0)
      { // randomly discard received data to emulate packet loss
        printf("~ Discarded received ACK to emulate packet loss. ~\n");
        state = 1;
        break;
      }

      alarm(0); // data received, reset alarm
      if (rcv_ack.sq_no == 0)
      {
        printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
        state = 2;
      }
      break;

    case 2: // wait for sending packet with seq. no. 1
      printf("\nEnter message 1: ");
      fgets(send_pkt.data, sizeof(send_pkt), stdin);
      send_pkt.sq_no = 1;
      sendPacket(s, send_pkt, si_other);
      state = 3;
      break;

    case 3: // waiting for ACK 1
      if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, (socklen_t *)&slen) == -1)
      {
        if (errno == EINTR && flag == 1)
        {
          flag = 0;
          state = 3;
          break;
        }
        die("recvfrom()");
      }

      if (DEBUG && rand() % 5 == 0)
      { // randomly discard received data to emulate packet loss
        printf("~ Discarded received ACK to emulate packet loss. ~\n");
        state = 3;
        break;
      }

      alarm(0); // data received, reset alarm
      if (rcv_ack.sq_no == 1)
      {
        printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
        state = 0;
      }
      break;
    }
  }
  close(s);
  return 0;
}
/*  Simple udp server */
#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // close() function
#include <time.h>
#define BUFLEN 512 //Max length of buffer
#define PORT 8888  //The port on which to listen for incoming data

void die(char *s)
{
  perror(s);
  exit(1);
}

int getRand(int low, int up)
{
  return rand() % (up - low + 1) + low;
}

int main(void)
{
  srand(time(0));
  struct sockaddr_in si_me, si_other;
  int s, i, slen = sizeof(si_other), recv_len;
  char buf[BUFLEN];
  //create a UDP socket
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    die("socket");

  // zero out the structure
  memset(&si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(PORT);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind socket to port
  if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
    die("bind");

  //keep listening for data
  while (1)
  {
    int num = getRand(1, 6);
    printf("Guessed number: %d\n", num);

    //try to receive some data, this is a blocking call
    if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1)
      die("recvfrom()");

    int client_num = atoi(buf);
    printf("Client guessed %d\n\n", client_num);
    memset(buf, '\0', BUFLEN);
    if (client_num == num)
      sprintf(buf, "You Won!");
    else
      sprintf(buf, "You Lost!");

    //now reply the client with the same data
    if (sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&si_other, slen) == -1)
      die("sendto()");
  }
  close(s);
  return 0;
}
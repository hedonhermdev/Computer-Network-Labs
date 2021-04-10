#include "utils.h"

int read_till_now = 0;

/*
* Returns -1 when end of file has been reached already (before reading any data)
* Else, reads from file and sends packet. Returns 0.
*/
int sendPkt(int fd, int channel, FILE *fp)
{
  if (feof(fp))
    return -1;

  struct pkt send_pkt;
  memset(&send_pkt, 0, sizeof(send_pkt));
  int read_sz = fread(&(send_pkt.data), sizeof(char), PACKET_SZ, fp);

  if (read_sz < PACKET_SZ)
  {
    if (feof(fp))
    {
      send_pkt.last_pkt = true;
    }
    if (ferror(fp))
    {
      errExit("fread()");
    }
  }
  send_pkt.channel = channel;
  send_pkt.size = read_sz;
  send_pkt.seq_no = read_till_now;
  send_pkt.type = 0;
  read_till_now += read_sz;

  if (send(fd, (char *)&send_pkt, sizeof(struct pkt), 0) < 0)
    errExit("send()");

  printf("SENT PKT: Seq. No. = %d, Size = %d Bytes, CH = %d\n", send_pkt.seq_no, send_pkt.size, channel);

  return 0;
}

int main()
{
  int cfd[2];
  struct sockaddr_in saddr;
  saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(PORT);

  FILE *fp = fopen("input.txt", "r");
  if (fp == NULL)
    errExit("fopen()");

  /* Connect the two clients */
  for (int i = 0; i < 2; i++)
  {
    cfd[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd[i] == -1)
      errExit("socket()");
    if (connect(cfd[i], (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
      errExit("connect()");
  }

  fd_set monitor_fds;
  int terminate_cnt = 0;

  char recv_msg[MAX_DATA_SIZE];

  /* Send first two packets */
  sendPkt(cfd[0], 0, fp);
  sendPkt(cfd[1], 1, fp);

  while (terminate_cnt < 2)
  {
    FD_ZERO(&monitor_fds);
    FD_SET(cfd[0], &monitor_fds);
    FD_SET(cfd[1], &monitor_fds);

    int ready = select(max(cfd[0], cfd[1]) + 1, &monitor_fds, NULL, NULL, NULL);
    if (ready == -1)
    {
      if (errno == EINTR)
        continue;
      errExit("select()");
    }

    for (int i = 0; i < 2; i++)
    {
      if (FD_ISSET(cfd[i], &monitor_fds))
      {
        memset(&recv_msg, 0, sizeof(recv_msg));
        int read_bytes = recv(cfd[i], &recv_msg, MAX_DATA_SIZE, 0);
        if (read_bytes == -1)
        {
          errExit("recv()");
        }

        struct pkt *ack_pkt = (struct pkt *)recv_msg;
        printf("RCVD ACK: Seq. No. = %d, CH = %d\n", ack_pkt->seq_no, i);

        if (sendPkt(cfd[i], i, fp) == -1)
        {
          /*
          * If file has been read completely, no need to send more packets from this channel.
          * Note that the other channel might still be waiting for an ACK.
          */
          terminate_cnt++;
        }
      }
    }
  }

  close(cfd[0]);
  close(cfd[1]);
  fclose(fp);
}
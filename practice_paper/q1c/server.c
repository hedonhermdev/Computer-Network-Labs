#include "utils.h"

int main()
{
  int cfd[2];
  int sfd;
  struct sockaddr_in saddr;
  struct sockaddr_in caddr[2];
  int client_len[2];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1)
    errExit("socket()");

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(PORT);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);

  FILE *fp = fopen("output.txt", "w");
  if (fp == NULL)
    errExit("fopen()");

  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    errExit("bind()\n");

  if (listen(sfd, MAXPENDING) == -1)
    errExit("listen()");

  printf("Listening for connections...\n");

  /* connect with both clients */
  for (int i = 0; i < 2; i++)
  {
    cfd[i] = accept(sfd, (struct sockaddr *)&caddr[i], (socklen_t *)&client_len[i]);
    if (cfd[i] == -1)
      errExit("accept()");

    printf("Connected with client %d\n", i + 1);
  }

  /*
  * next_seq: the expected sequence number of next **in-order** packet
  * bytes_rcvd: total bytes received till now
  * out_of_order_cnt: since there are 2 clients, there can be a scenario like: xPPP 
  *   where x denotes missing packet and P denotes received packets. Thus, 3 packets have been
  *   received out of order (out_of_order_cnt = 3). Note that there cannot be multiple missing
  *   packets, that is, something like xPPPxP is impossible because there are only 2 clients.
  */
  int next_seq = 0, bytes_rcvd = 0, out_of_order_cnt = 0;

  char recv_msg[MAX_DATA_SIZE];

  /*
  * buffer: buffer for out of order data
  * buff_data_sz: size of each out of order payload received
  */
  char **buffer = (char **)calloc(MAX_OUT_OF_ORDER, sizeof(char *));
  int buff_data_sz[MAX_OUT_OF_ORDER];

  /* terminate select loop if true */
  bool terminate = false;

  /* true when last packet has been received */
  bool got_last_pkt = false;

  fd_set monitor_fds;
  struct pkt ack_pkt;
  ack_pkt.type = 1;

  while (!terminate)
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
        memset(recv_msg, 0, MAX_DATA_SIZE);
        int n = recv(cfd[i], recv_msg, MAX_DATA_SIZE, 0);
        if (n == -1)
        {
          errExit("recv()");
        }

        struct pkt *recv_pkt = (struct pkt *)recv_msg;
        printf("RCVD PACKET: Seq. No. = %d, Size = %d Bytes, CH = %d\n", recv_pkt->seq_no, recv_pkt->size, i);

        bytes_rcvd += recv_pkt->size;

        if (recv_pkt->seq_no == next_seq)
        {
          // in order packet
          fwrite(recv_pkt->data, sizeof(char), recv_pkt->size, fp);

          /* loop through out of order packets, if any, and write data to file */
          for (int i = 0; i < out_of_order_cnt; i++)
          {
            fwrite(buffer[i], sizeof(char), buff_data_sz[i], fp);
            free(buffer[i]);
            buff_data_sz[i] = 0;
          }

          /* reset out of order count */
          out_of_order_cnt = 0;

          if (recv_pkt->last_pkt)
            got_last_pkt = true;

          /* if we have seen the last packet, terminate */
          if (got_last_pkt)
            terminate = true;

          /* the next expected seq num would be the total number of bytes received yet */
          next_seq = bytes_rcvd;
        }
        else
        {
          // out of order packet
          buffer[out_of_order_cnt] = strdup(recv_pkt->data);
          buff_data_sz[out_of_order_cnt] = recv_pkt->size;
          out_of_order_cnt++;

          /* note that here we do not terminate on getting the last packet as it is out of order */
          if (recv_pkt->last_pkt)
            got_last_pkt = true;
        }

        usleep(100000); /* 100ms delay */
        ack_pkt.channel = i;
        ack_pkt.seq_no = recv_pkt->seq_no;
        n = send(cfd[i], (char *)&ack_pkt, sizeof(ack_pkt), 0);
        if (n < 0)
        {
          errExit("send()");
        }
        printf("SENT ACK: Seq. No. = %d, CH = %d\n", ack_pkt.seq_no, ack_pkt.channel);
      }
    }
  }

  close(cfd[0]);
  close(cfd[1]);
  close(sfd);
  free(buffer);
  fclose(fp);
}
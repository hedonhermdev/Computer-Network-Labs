/*
* ##### NOTE ####
* In this question, we need to have both connections on different threads
* as they should have independent timeouts. We create two threads and call
* select() on each thread to manage timeouts.
*/
#include "utils.h"

int read_till_now = 0;
int cfd[2];
FILE *fp;

/*
* Returns NULL when end of file has been reached already (before reading any data)
* Else, reads from file and sends packet. Returns the packet sent.
*/
struct pkt *sendPkt(int fd, int channel, FILE *fp)
{
  if (feof(fp))
    return NULL;

  struct pkt *send_pkt = (struct pkt *)calloc(1, sizeof(struct pkt));
  memset(send_pkt, 0, sizeof(struct pkt));
  int read_sz = fread(&(send_pkt->data), sizeof(char), PACKET_SZ, fp);

  if (read_sz < PACKET_SZ)
  {
    if (feof(fp))
    {
      send_pkt->last_pkt = true;
    }
    if (ferror(fp))
    {
      errExit("fread()");
    }
  }
  send_pkt->channel = channel;
  send_pkt->size = read_sz;
  send_pkt->seq_no = read_till_now;
  send_pkt->type = 0;
  read_till_now += read_sz;

  if (send(fd, (char *)send_pkt, sizeof(struct pkt), 0) < 0)
    errExit("send()");

  printf("SENT PKT: Seq. No. = %d, Size = %d Bytes, CH = %d\n", send_pkt->seq_no, send_pkt->size, channel);

  return send_pkt;
}

void *connectionHandler(void *args)
{
  int channel_id = ((struct thread_arg *)args)->channel_id;

  fd_set monitor_fds;
  bool terminate = false;
  struct pkt *last_pkt_sent = ((struct thread_arg *)args)->last_pkt_sent;

  /* return if no packet was sent */
  if (last_pkt_sent == NULL)
  {
    return NULL;
  }

  char recv_msg[MAX_DATA_SIZE];

  while (!terminate)
  {
    FD_ZERO(&monitor_fds);
    FD_SET(cfd[channel_id], &monitor_fds);

    struct timeval tv;
    tv.tv_sec = RTO;
    tv.tv_usec = 0;

    int ready = select(cfd[channel_id] + 1, &monitor_fds, NULL, NULL, &tv);
    if (ready == -1)
    {
      if (errno == EINTR)
        continue;
      errExit("select()");
    }

    if (FD_ISSET(cfd[channel_id], &monitor_fds))
    {
      memset(&recv_msg, 0, sizeof(recv_msg));
      int read_bytes = recv(cfd[channel_id], &recv_msg, MAX_DATA_SIZE, 0);
      if (read_bytes == -1)
      {
        errExit("recv()");
      }

      struct pkt *ack_pkt = (struct pkt *)recv_msg;
      printf("RCVD ACK: Seq. No. = %d, CH = %d\n", ack_pkt->seq_no, channel_id);

      free(last_pkt_sent);
      last_pkt_sent = sendPkt(cfd[channel_id], channel_id, fp);
      if (last_pkt_sent == NULL)
      {
        terminate = true;
      }
    }
    else
    {
      // select call timed out, retransmit previous packet
      if (send(cfd[channel_id], (char *)last_pkt_sent, sizeof(struct pkt), 0) < 0)
        errExit("send() while retransmission");
      printf("RETRANSMIT PKT: Seq. No. = %d, Size = %d Bytes, CH = %d\n", last_pkt_sent->seq_no, last_pkt_sent->size, channel_id);
    }
  }

  close(cfd[channel_id]);
  return NULL;
}

int main()
{
  struct sockaddr_in saddr;
  saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(PORT);

  fp = fopen("input.txt", "r");
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

  pthread_t channel_0, channel_1;
  int channel_id_0 = 0, channel_id_1 = 1;

  struct thread_arg *arg_0, *arg_1;
  arg_0 = (struct thread_arg *)calloc(1, sizeof(struct thread_arg));
  arg_1 = (struct thread_arg *)calloc(1, sizeof(struct thread_arg));
  if (arg_0 == NULL || arg_1 == NULL)
    errExit("calloc()");

  arg_0->channel_id = 0;
  arg_1->channel_id = 1;

  /* Send first two packets */
  arg_0->last_pkt_sent = sendPkt(cfd[0], 0, fp);
  arg_1->last_pkt_sent = sendPkt(cfd[1], 1, fp);

  if (pthread_create(&channel_0, NULL, connectionHandler, arg_0) != 0)
    errExit("pthread_create()");
  if (pthread_create(&channel_1, NULL, connectionHandler, arg_1) != 0)
    errExit("pthread_create()");

  /* wait for threads */
  pthread_join(channel_0, NULL);
  pthread_join(channel_1, NULL);

  free(arg_0);
  free(arg_1);
  fclose(fp);
}
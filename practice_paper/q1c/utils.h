#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer #include <string.h>
#include <unistd.h>     // close() function
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#define PACKET_SZ 100
#define PORT 8080
#define MAXPENDING 5
#define MAX_DATA_SIZE 1024
#define MAX_OUT_OF_ORDER 10
#define RTO 5

struct pkt
{
  int size;
  int seq_no;
  bool last_pkt;
  bool type;
  char data[PACKET_SZ];
  int channel;
};

struct thread_arg
{
  int channel_id;
  struct pkt *last_pkt_sent;
};

void errExit(char *err)
{
  perror(err);
  exit(EXIT_FAILURE);
}

int max(int a, int b)
{
  return a > b ? a : b;
}
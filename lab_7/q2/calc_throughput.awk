BEGIN {
  tcp_start_time = 0
  udp_start_time = 5
  tcp_bytes_rcvd = 0
  udp_bytes_rcvd = 0
}

{
  action = $1;
  time = $2;
  from = $3;
  to = $4;
  type = $5;
  pktsize = $6;
  flow_id = $8;
  src= $9;
  dst = $10 ;
  seq_no = $11;
  packet_id= $12;

  if (from == 2 && to == 3 && type == "cbr") {
    udp_bytes_rcvd += pktsize
    printf ("%f %f\n", time, udp_bytes_rcvd/(time - udp_start_time) * 8/1000000) > udp_throughput.tr
  }

  if (from == 2 && to == 3 && type == "tcp") {
    tcp_bytes_rcvd += pktsize
    printf ("%f %f\n", time, tcp_bytes_rcvd/(time - tcp_prev_time) * 8/1000000) > tcp_throughput.tr
  }
}

END {
}
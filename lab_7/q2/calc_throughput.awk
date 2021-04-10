BEGIN {
  # first method to calc throughput (cumulative bytes / time)
  tcp_start_time = 0
  udp_start_time = 5
  tcp_bytes_rcvd = 0
  udp_bytes_rcvd = 0

  # second method to calc throughput (current bytes rcvd / curr_time - prev_time)
  tcp_prev_time = 0
  udp_prev_time = 5
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

  if (from == 2 && to == 3 && type == "cbr" && action=="r") {
    udp_bytes_rcvd += pktsize
    printf ("%f %f\n", time, udp_bytes_rcvd/(time - udp_start_time) * 8/1000000) > "udp_throughput.tr"

    # printf ("%f %f\n", time, pktsize/(time - udp_prev_time) * 8/1000000) > "udp_throughput.tr"
    # udp_prev_time = time
  }

  if (from == 2 && to == 3 && type == "tcp" && action=="r") {
    tcp_bytes_rcvd += pktsize
    printf ("%f %f\n", time, tcp_bytes_rcvd/(time - tcp_start_time) * 8/1000000) > "tcp_throughput.tr"

    # printf ("%f %f\n", time, pktsize/(time - tcp_prev_time) * 8/1000000) > "tcp_throughput.tr"
    # tcp_prev_time = time
  }
}

END {
}
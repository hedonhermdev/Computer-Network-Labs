# run awk files: awk -f filename.awk trace_file.tr
BEGIN {
  udp_drop_cnt = 0
  printf("0 0\n")
}

{
    event = $1
    time = $2
    from_node = $3
    to_node = $4
    pkt_type = $5 # tcp, cbr, ack, rtProtoDV
    pkt_size = $6
    flags = $7
    flow_id = $8
    src_addr = $9
    dst_addr = $10
    seq_num = $11
    pkt_id = $12

    # do your stuff here
    if (pkt_type == "cbr" && event=="d") {
      udp_drop_cnt++
      printf("%f %d\n", time, udp_drop_cnt)
    }
}

END {
}
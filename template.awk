# run awk files: awk -f filename.awk trace_file.tr
BEGIN {
    # print "begin"
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
}

END {
    # print "end"
}
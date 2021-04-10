# run awk files: awk -f filename.awk trace_file.tr
BEGIN {
    highest_pkt_id = -1
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

    # get the time when packet started off from node 1
    if (from_node == 1 && to_node == 2 && event == "+" && pkt_type == "tcp") {
        if (pkt_id > highest_pkt_id)
            highest_pkt_id = pkt_id
        start_time[pkt_id] = time
        end_time[pkt_id] = -1
    }

    if (from_node == 2 && to_node == 3 && event == "r" && pkt_type == "tcp") {
        if (pkt_id > highest_pkt_id)
            highest_pkt_id = pkt_id

        end_time[pkt_id] = time
    }
}

END {
    last_delay = 0

    for (id = 0; id <= highest_pkt_id; id ++ 1) {
        start = start_time[id]
        end = end_time[id]
        curr_delay = end - start

        if (curr_delay > 0) {
            printf("%f %f\n", start, curr_delay - last_delay)
            last_delay = curr_delay
        }
    }
}
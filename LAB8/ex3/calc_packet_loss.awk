BEGIN {
    tcp_loss = 0
    udp_loss = 0
}

{
    if ($1 == "d" && $5 == "tcp")
        tcp_loss++

    if ($1 == "d" && $5 == "cbr")
        tcp_loss++
}

END {
    printf "TCP packet lost: %d\n", tcp_loss
    printf "UDP packet lost: %d\n", udp_loss
}
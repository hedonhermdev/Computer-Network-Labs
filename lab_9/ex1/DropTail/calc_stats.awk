BEGIN {
    print "=== Stats for DropTail ==="
    recvd_sz = 0
    startTime = 1
    stopTime = 60
    drop_cnt = 0
}

{
    if ($1 == "r" && $4 == 2 && $5 == "tcp" && $6 >= 512) {
        if ($2 > stopTime)
            stopTime = $2
        recvd_sz+=$6;
    }
    if ($1 == "d" && $4 == 2 && $5 == "tcp") {
        drop_cnt++
    }
}

END {
    printf "dropped packets: %d\n", drop_cnt
    printf "average throughput (Mb/s): %f\n", recvd_sz/(stopTime - startTime) * (8/1000000)
}
# create simulator
set ns [new Simulator]

# setup tracing
set f0 [open throughput.tr w]
set f1 [open cwnd.tr w]
set nf [open out.nam w]
$ns namtrace-all $nf
set tf [open outall.tr w]
$ns trace-all $tf

# create nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]

# connect nodes
$ns duplex-link $n0 $n1 2Mb 100ms DropTail
$ns duplex-link $n1 $n2 2Mb 100ms DropTail

# introduce queue limit
$ns queue-limit $n1 $n2 5

# define finish procedure
proc finish {} {
    global f0 f1 nf tf ns
    $ns flush-trace
    close $f0
    close $f1
    close $nf
    close $tf
    exec xgraph throughput.tr -geometry 800x400 &
    exec xgraph cwnd.tr -geometry 800x400 &
    exec nam out.nam &
    exit 0
}

# record metrics procedure
proc record {} {
    global sink0 tcp0 f0 f1

    set ns [Simulator instance]
    # interval for recording
    set time .5
    # current time
    set now [$ns now]
    
    set bw [$sink0 set bytes_]
    set cwnd [$tcp0 set cwnd_]
    # put bandwidth in Mbit/s
    puts $f0 "$now [expr $bw/$time*8/1000000]"
    # put cwnd
    puts $f1 "$now $cwnd"

    $sink0 set bytes_ 0

    $ns at [expr $now+$time] "record"
}

# attach TCP agent & FTP traffic to 0->2
set tcp0 [new Agent/TCP]
$ns attach-agent $n0 $tcp0
set sink0 [new Agent/TCPSink]
$ns attach-agent $n2 $sink0
set ftp0 [new Application/FTP]
$ftp0 set packetSize_ 512
$ftp0 set interval_ 0.005
$ftp0 attach-agent $tcp0
$ns connect $tcp0 $sink0

# log & run
$ns at 0.0 "record"
$ns at 1.0 "$ftp0 start"
$ns at 30.0 "$ftp0 stop"
$ns at 35.0 "finish"
$ns run
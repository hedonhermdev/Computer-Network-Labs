# create simulator
set ns [new Simulator]

# define colors
$ns color 1 Blue
$ns color 2 Red

# setup tracing
set f0 [open throughput_tcp.tr w]
set f1 [open cwnd_tcp.tr w]
set f2 [open throughput_udp.tr w]
set nf [open out.nam w]
$ns namtrace-all $nf
set tf [open outall.tr w]
$ns trace-all $tf

# create nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]

# connect nodes
$ns duplex-link $n0 $n1 2Mb 100ms DropTail
$ns duplex-link $n1 $n2 2Mb 100ms DropTail
$ns duplex-link $n3 $n1 2Mb 100ms DropTail

# add queue limit
$ns queue-limit $n1 $n2 3

# define finish procedure
proc finish {} {
    global f0 f1 f2 nf tf ns
    $ns flush-trace
    close $f0
    close $f1
    close $f2
    close $nf
    close $tf
    exec awk -f calc_packet_loss.awk outall.tr > packet_losst_stats.txt
    exec xgraph throughput_tcp.tr throughput_udp.tr -geometry 800x400 &
    exec xgraph cwnd_tcp.tr -geometry 800x400 &
    exec nam out.nam &
    exit 0
}

# record metrics procedure
proc record { f0 f1 src sink is_udp } {
    set ns [Simulator instance]
    # interval for recording
    set time .5
    # current time
    set now [$ns now]
    
    set bw [$sink set bytes_]
    # put bandwidth in Mbit/s
    puts $f0 "$now [expr $bw/$time*8/1000000]"

    if {$is_udp == 0} {
        set cwnd [$src set cwnd_]
        # put cwnd
        puts $f1 "$now $cwnd"
    }

    $sink set bytes_ 0

    $ns at [expr $now+$time] "record $f0 $f1 $src $sink $is_udp"
}

proc setupTcp { srcNode sinkNode packetSize interval } {
    global ns
    set tcp [new Agent/TCP]
    $ns attach-agent $srcNode $tcp
    set sink [new Agent/TCPSink]
    $ns attach-agent $sinkNode $sink
    set ftp [new Application/FTP]
    $ftp set packetSize_ $packetSize
    $ftp set interval_ $interval
    $ftp attach-agent $tcp
    $ns connect $tcp $sink
    return [ list $tcp $sink $ftp ]
}

proc setupUdp { srcNode sinkNode packetSize interval } {
    global ns
    set udp [new Agent/UDP]
    $ns attach-agent $srcNode $udp
    set sink [new Agent/LossMonitor]
    $ns attach-agent $sinkNode $sink
    set cbr [new Application/Traffic/CBR]
    $cbr set packetSize_ $packetSize
    $cbr set interval_ $interval
    $cbr attach-agent $udp
    $ns connect $udp $sink
    return [ list $udp $sink $cbr ]
}

set tcpList0 [setupTcp $n0 $n2 800 0.005]
set udpList0 [setupUdp $n3 $n2 800 0.005]

set tcp0 [lindex $tcpList0 0]
set tcp_sink0 [lindex $tcpList0 1]
set ftp0 [lindex $tcpList0 2]

set udp0 [lindex $udpList0 0]
set udp_sink0 [lindex $udpList0 1]
set cbr0 [lindex $udpList0 2]

# assign colors
$tcp0 set fid_ 1
$udp0 set fid_ 2

# log & run
$ns at 0.0 "record $f0 $f1 $tcp0 $tcp_sink0 0"
$ns at 0.0 "record $f2 $f2 $udp0 $udp_sink0 1"
$ns at 1.0 "$ftp0 start"
$ns at 10.0 "$cbr0 start"
$ns at 30.0 "$ftp0 stop"
$ns at 40.0 "$cbr0 stop"
$ns at 45.0 "finish"
$ns run
# create simulator
set ns [new Simulator]

# define colors
$ns color 1 Blue
$ns color 2 Red
$ns color 3 Green

# setup tracing
set cwnd_file [open cwnd_tcp.tr w]
set nf [open out.nam w]
$ns namtrace-all $nf
set tf [open outall.tr w]
$ns trace-all $tf

# create nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]
set n6 [$ns node]
set n7 [$ns node]

# connect nodes
$ns duplex-link $n0 $n2 1Mb 2ms DropTail
$ns duplex-link $n1 $n2 1Mb 2ms DropTail
$ns duplex-link $n2 $n3 700Kb 2ms DropTail
$ns duplex-link $n3 $n4 1Mb 2ms DropTail
$ns duplex-link $n3 $n5 700Kb 2ms DropTail
$ns duplex-link $n5 $n6 1Mb 2ms DropTail
$ns duplex-link $n5 $n7 1Mb 2ms DropTail

# define finish procedure
proc finish {} {
    global cwnd_file nf tf ns
    $ns flush-trace
    close $cwnd_file
    close $nf
    close $tf
    exec awk -f clac_drop.awk outall.tr > tcp_drop_stats.txt
    exec xgraph cwnd_file.tr -geometry 800x400 &
    exec nam out.nam &
    exit 0
}

proc plotWindow {} {
  global cwnd_file n1
  set ns [Simulator instance]
  set time .1
  set now [$ns now]

  set cwnd [$n1 set cwnd_]
  
  puts $cwnd_file "$now $cwnd"

  $ns at [expr $now+$time] "plotWindow"
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

set tcpList0 [setupTcp $n1 $n6 500 0.005]
set udpList0 [setupUdp $n0 $n7 500 0.005]
set udpList1 [setupUdp $n4 $n7 500 0.005]

set tcp0 [lindex $tcpList0 0]
set tcp_sink0 [lindex $tcpList0 1]
set ftp0 [lindex $tcpList0 2]

set udp0 [lindex $udpList0 0]
set udp_sink0 [lindex $udpList0 1]
set cbr0 [lindex $udpList0 2]

set udp1 [lindex $udpList1 0]
set udp_sink1 [lindex $udpList1 1]
set cbr1 [lindex $udpList1 2]

# assign colors
$tcp0 set fid_ 1
$udp0 set fid_ 2
$udp1 set fid_ 3

# log & run
$ns at 0.0 "plotWindow"
$ns at 1.0 "$ftp0 start"
$ns at 8.0 "$cbr0 start"
$ns at 8.0 "$cbr1 start"
$ns at 13.0 "$cbr0 stop"
$ns at 13.0 "$cbr1 stop"
$ns at 19.0 "$ftp0 stop"
$ns at 20.0 "finish"
$ns run
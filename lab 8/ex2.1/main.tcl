# create simulator
set ns [new Simulator]

# define colors
$ns color 1 Blue
$ns color 2 Red

# setup tracing
set f0 [open throughput1.tr w]
set f1 [open cwnd1.tr w]
set f2 [open throughput2.tr w]
set f3 [open cwnd2.tr w]
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

# connect nodes
$ns duplex-link $n0 $n1 2Mb 100ms DropTail
$ns duplex-link $n1 $n2 2Mb 100ms DropTail
$ns duplex-link $n3 $n1 2Mb 100ms DropTail
$ns duplex-link $n4 $n0 2Mb 100ms DropTail

# add queue limit
$ns queue-limit $n1 $n2 5

# define finish procedure
proc finish {} {
    global f0 f1 f2 f3 nf tf ns
    $ns flush-trace
    close $f0
    close $f1
    close $f2
    close $f3
    close $nf
    close $tf
    exec xgraph throughput1.tr throughput2.tr -geometry 800x400 &
    exec xgraph cwnd1.tr cwnd2.tr -geometry 800x400 &
    exec nam out.nam &
    exit 0
}

# record metrics procedure
proc record { f0 f1 tcp sink } {
    set ns [Simulator instance]
    # interval for recording
    set time .5
    # current time
    set now [$ns now]
    
    set bw [$sink set bytes_]
    set cwnd [$tcp set cwnd_]
    # put bandwidth in Mbit/s
    puts $f0 "$now [expr $bw/$time*8/1000000]"
    # put cwnd
    puts $f1 "$now $cwnd"

    $sink set bytes_ 0

    $ns at [expr $now+$time] "record $f0 $f1 $tcp $sink"
}

proc setupTcp { srcNode sinkNode } {
    global ns
    set tcp [new Agent/TCP]
    $ns attach-agent $srcNode $tcp
    set sink [new Agent/TCPSink]
    $ns attach-agent $sinkNode $sink
    set ftp [new Application/FTP]
    $ftp set packetSize_ 512
    $ftp set interval_ 0.005
    $ftp attach-agent $tcp
    $ns connect $tcp $sink
    return [ list $tcp $sink $ftp ]
}

set tcpList0 [setupTcp $n4 $n2]
set tcpList1 [setupTcp $n3 $n2]

set tcp0 [lindex $tcpList0 0]
set sink0 [lindex $tcpList0 1]
set ftp0 [lindex $tcpList0 2]

set tcp1 [lindex $tcpList1 0]
set sink1 [lindex $tcpList1 1]
set ftp1 [lindex $tcpList1 2]

# assign colors
$tcp0 set fid_ 1
$tcp1 set fid_ 2

# log & run
$ns at 0.0 "record $f0 $f1 $tcp0 $sink0"
$ns at 0.0 "record $f2 $f3 $tcp1 $sink1"
$ns at 1.0 "$ftp0 start"
$ns at 10.0 "$ftp1 start"
$ns at 30.0 "$ftp0 stop"
$ns at 40.0 "$ftp1 stop"
$ns at 45.0 "finish"
$ns run
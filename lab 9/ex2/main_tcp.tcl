# create simulator
set ns [new Simulator]

# define colors
$ns color 1 Blue
$ns color 2 Red

# setup tracing
set outall [open outall.tr w]
$ns trace-all $outall
set nf [open out.nam w]
$ns namtrace-all $nf

# create nodes
for {set i 0} {$i < 7} {incr i} {
    set n($i) [$ns node]
}

# connect nodes
for {set i 0} {$i < 7} {incr i} {
    $ns duplex-link $n($i) $n([expr ($i+1)%7]) 1Mb 10ms DropTail
}

# define finish procedure
proc finish {} {
    global outall nf ns
    $ns flush-trace
    exec nam out.nam &
    exit 0
}

proc setupTcp { srcNode sinkNode pcktSz interval } {
    global ns
    set tcp [new Agent/TCP]
    $ns attach-agent $srcNode $tcp
    set sink [new Agent/TCPSink]
    $ns attach-agent $sinkNode $sink
    set ftp [new Application/FTP]
    $ftp set packetSize_ $pcktSz
    $ftp set interval_ $interval
    $ftp attach-agent $tcp
    $ns connect $tcp $sink
    return [ list $tcp $sink $ftp ]
}

set tcpList0 [setupTcp $n(0) $n(3) 512 0.005]
set tcp0 [lindex $tcpList0 0]
set sink0 [lindex $tcpList0 1]
set ftp0 [lindex $tcpList0 2]

# assign colors
$tcp0 set fid_ 1

# log & run
$ns at 0.0 "$ftp0 start"
$ns rtmodel-at 1.0 down $n(1) $n(2)
$ns rtmodel-at 2.0 up $n(1) $n(2)
$ns at 5.0 "$ftp0 stop"
$ns at 8.0 "finish"
$ns run
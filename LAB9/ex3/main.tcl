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

# set DV routing on all nodes
$ns rtproto DV
# $ns rtproto DV [list $n(0) $n(1) $n(2) $n(3) $n(4) $n(5) $n(6)]

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

# define routing info dump procedure
proc rtdump {} {
    global ns
    set now [$ns now]
    puts "Routing table at time $now"
    # Table in terms of next hops
    $ns dump-routelogic-nh

    # Table in terms of distance
    # $ns dump-routelogic-distance
}

# define interval for periodic route updates
Agent/rtProto/DV set advertInterval 4

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

set udpList0 [setupUdp $n(0) $n(3) 512 0.005]
set udp0 [lindex $udpList0 0]
set sink0 [lindex $udpList0 1]
set cbr0 [lindex $udpList0 2]

# assign colors
$udp0 set fid_ 1

# log & run
$ns at 0.0 "$cbr0 start"
$ns at 0.9 "rtdump"
$ns rtmodel-at 1.0 down $n(1) $n(2)
$ns at 1.5 "rtdump"
$ns rtmodel-at 2.0 up $n(1) $n(2)
$ns at 5.0 "$cbr0 stop"
$ns at 8.0 "finish"
$ns run
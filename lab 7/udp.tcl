set ns [new Simulator]

set nf [open out.nam w]
$ns namtrace-all $nf

set n0 [$ns node]
set n1 [$ns node]
# connect the nodes with 1Mb bandwidth, 10ms delay and DropTail queue
$ns duplex-link $n0 $n1 1Mb 10ms DropTail 

proc finish {} {
    global ns nf
    # flush buffer to file before exiting
    $ns flush-trace
    close $nf
    exec nam out.nam &
    exit 0
}

# create a UDP agent and attach it to node 0
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

# Create a CBR traffic source and attach it to agent
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
# send a packet every 0.005 seconds => 200 packets per second
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

# Create a traffic sink UDP agent
set udp1 [new Agent/Null]
$ns attach-agent $n1 $udp1

# Connect the 2 agents
$ns connect $udp0 $udp1

# data sending commands
$ns at 0.5 "$cbr0 start"
$ns at 4.5 "$cbr0 stop"
$ns at 5.0 "finish"
$ns run
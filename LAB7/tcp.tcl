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

# create a TCP agent and attach it to node 0
set tcp0 [new Agent/TCP]
$ns attach-agent $n0 $tcp0

# Create a FTP traffic source and attach it to agent
set ftp0 [new Application/FTP]
$ftp0 set packetSize_ 512
# send a packet every 0.005 seconds => 200 packets per second
$ftp0 set interval_ 0.005
$ftp0 attach-agent $tcp0

# Create a traffic sink UDP agent
set tcp1 [new Agent/TCPSink]
$ns attach-agent $n1 $tcp1

# Connect the 2 agents
$ns connect $tcp0 $tcp1

# data sending commands
$ns at 0.5 "$ftp0 start"
$ns at 4.5 "$ftp0 stop"
$ns at 5.0 "finish"
$ns run
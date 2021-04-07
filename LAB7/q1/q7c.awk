BEGIN {
    printf "---BEGIN---\n"
    cnt1 = 0
    cnt2 = 0
}

$1~"\+" && $3~3 {++cnt1}
$1~"\+" && $3~3 && $4!~4 {++cnt2}

END {
    print "Count = ", cnt1
    print "Count except to node 4 = ", cnt2
    printf "---END---\n"
}
BEGIN {
    printf "---BEGIN---\n"
    cnt = 0
    tot_cnt = 0
}
{
    ++tot_cnt;
    if ($1 == "r" && $3==1 && $4==3) {
        ++ cnt;
    } 
}

END {
    print "Count = ", tot_cnt - cnt
    printf "---END---\n"
}
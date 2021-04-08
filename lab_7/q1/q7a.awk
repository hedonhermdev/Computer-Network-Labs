BEGIN {
    printf "---BEGIN---\n"
    cnt = 0
}

{
    if ($1 == "r" && $3 == 0 && $4 == 3) {
        ++cnt;
    }
}

END {
    print "Count = ", cnt
    printf "---END---\n"
}
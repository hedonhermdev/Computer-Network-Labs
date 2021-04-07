BEGIN {
    prev_time = -1
}

{
    if ($1 == "r" && $3 == 2 && $4 == 3 && $5 == "tcp") {
        if (prev_time==-1) prev_time = $2
        else {
            printf "%f %f\n", $2, $2 - prev_time
            prev_time = $2
        }
    }
}
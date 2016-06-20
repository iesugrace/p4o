#!/bin/bash
#
# Author:   Joshua Chen <iesugrace@gmail.com>
# Date:     2016-06-19 15:15:59
# Location: Shenzhen
# Desc:     Get all parent processes information
#

getparents() {
    local pid=$1 idx=0 gens ppid name
    lines=$(ps -eo pid,ppid,comm | tail -n +2)
    while true
    do
        while read _pid ppid name
        do
            test "$_pid" = "$pid" && break
        done <<< "$lines"
        gens[$idx]="$pid ($name)"
        idx=$((idx + 1))
        test $ppid = 0 && break
        pid=$ppid
    done
    idx=$((idx - 1))
    while test "$idx" -ge 0
    do
        echo "${gens[$idx]}"
        idx=$((idx - 1))
    done
}

if test $# = 0; then
    pid=$BASHPID
elif test $# = 1; then
    pid=$1
    if ! ps -eo pid | grep -qw $pid; then
        echo "$pid not exists" >&2
        exit 1
    fi
else
    bname=$(basename $0)
    echo "Usage: $bname [pid]" >&2
    exit 1
fi
if ! grep -qE '^[0-9]+$' <<< "$pid"; then
    echo "invalid pid: $pid" >&2
    exit 1
fi

for i in {1..200}
do
    getparents $pid
done

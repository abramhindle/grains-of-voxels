#!/bin/bash
killall csound
killall csound
killall -9 csound
for file in $*
do
    csound $file &
    sleep 1
done
cat > /dev/null

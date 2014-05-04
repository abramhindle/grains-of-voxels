#!/bin/bash
killall csound
killall csound
killall -9 csound
for file in $*
do
    csound $file &
    sleep 1
done
jack_disconnect csoundGrain:output1 system:playback_1
jack_disconnect csoundGrain-01:output1 system:playback_1
jack_connect csoundGrain:output1 system:playback_2
jack_connect csoundGrain-01:output1 system:playback_2

cat > /dev/null

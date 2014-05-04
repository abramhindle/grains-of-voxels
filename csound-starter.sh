#!/bin/bash
killall csound
killall csound
killall -9 csound
for file in $*
do
    csound $file &
    sleep 1
done
sleep 1
jack_disconnect csoundGrain-02:output1 system:playback_1
jack_disconnect csoundGrain-03:output1 system:playback_1
jack_connect csoundGrain-02:output1 system:playback_2
jack_connect csoundGrain-03:output1 system:playback_2

cat > /dev/null

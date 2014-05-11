#!/bin/bash
killall csound
killall csound
killall -9 csound
(sleep 330; echo 666 | osd_cat -p middle -A center -f '-*-helvetica-*-r-*-*-34-*-*-*-*-*-*-*' -d 10 -s 2 -S green) &
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
jack_connect csoundGrain:output1 system:playback_1
jack_connect csoundGrain-01:output1 system:playback_1

cat > /dev/null

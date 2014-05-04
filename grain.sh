rm grain.mkv
(sleep 2; jack_connect "csoundGrain:output1" ffmpeg:input_1) &
(sleep 2; jack_connect "alsa_in:capture_1" ffmpeg:input_1) &
#
#(sleep 2; jack_connect SuperCollider:out_1 
#(sleep 2; jack_connect SuperCollider:out_2 
sleep 1
ffmpeg -f jack -isync -ac 1 -i ffmpeg -f x11grab -r 30 -s 1920x1080 -i :0.0 -acodec pcm_s16le -vcodec libx264 -vpre faster -threads 0 -y grain.mkv

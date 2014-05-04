use Net::OpenSoundControl::Client;
use strict;
my $oschost = "127.0.0.1";
my $oscport = 4103;
{
  my $client = undef;
  sub client {
    if (!$client) {
      $client = Net::OpenSoundControl::Client->new(Host => $oschost, Port => $oscport ) or die "Could not start client: $@\n";

    }
    return $client;
  }
}

#client()->send(['/grain/gkPhase','f',0]);
#client()->send(['/grain/gkPhaseMix','f',0]);
#client()->send(['/grain/gkDur','f',0]);
#client()->send(['/grain/gkDens','f',0.00000000000000000001]);
#client()->send(['/grain/gkFreq','f',0]);
#client()->send(['/grain/gkFreqRand','f',0]);
for (1..100) {
	client()->send(['/grain/trigger','i',550,'f',0,'f',0.1,'f',rand()]);
	client()->send(['/grain/trigger','i',550,'f',0,'f',0.1,'f',rand()]);
}


#push @elms,['/'.$command, @args];

use strict;
use Time::HiRes qw(usleep);
use List::Util qw(min max);
use Net::OpenSoundControl::Client;
my @ports = qw(4101 4102 4103 4104);
my @clients = map {
    Net::OpenSoundControl::Client->new(Host => "127.0.0.1", Port => $_ ) or die "Couldn't make 127.0.0.1 $_ [$@]";
} @ports;

$| = 1;
my $cur = 0.0;
my $i = 0;
my $v = 0;
my $str = "";
my @vals;
my @ovals;
my $range;
while(<STDIN>) {
    chomp;
    @vals = split(/,/,$_);
    $range = (@vals - 1) * 1024;
    my $n = $vals[0];
    my @buffers = map { [] } @clients;
    for ($i = 1; $i < @vals; $i++) {
        $v = $vals[$i];
        #next if ($v == $ovals[$i]);
        next if $v == 0;
        next if $v == 1024;
        $v = ($v + ($i-1) * 1024) / $range;
        #$str = join(" ",("i556",(rand()*0.1),0.05+rand()*0.15, rand() * 5000, 1.0+1.0*rand(), max(0.0,min(1.0,$v)), 1.0,$/));
        my $instr = ($n + 700);
        $str = join(" ",("i$instr",(rand()*0.1),0.1, max(0.0,min(1.0,$v)),$/));
        my $osc = ["/grain/trigger",
                   "i", $instr, "f", (rand()*0.1), "f", 0.1, "f", max(0.0,min(1.0,$v))
                  ];
        push @{$buffers[$n % 4]}, $osc;
        #$str = join(" ",("i556",(rand()*0.01),0.1, 5000, 4.0, max(0.0,min(1.0,$v)), 1.0,$/));
        warn $str;
        #print $str;
    }
    my $cli = 0;
    foreach my $buffer (@buffers) {
        my @buf = @$buffer;
        if (@buf) {
            $clients[$cli]->send(['#bundle', 0, @buf]);
        }
        $cli++;
    }
    @ovals = @vals;
}

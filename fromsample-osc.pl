#!/usr/bin/perl
use strict;
use Time::HiRes qw(usleep);
use List::Util qw(min max);
use Net::OpenSoundControl::Client;
my @ports = qw(4101 4102 4103 4104);
$| = 1;
my $cur = 0.0;
my $i = 0;
my $v = 0;
my $str = "";
my @vals;
my @ovals;
my $range;
my $distmax = sqrt(640*640.0 + 480*480);
my @clients = map {
    Net::OpenSoundControl::Client->new(Host => "127.0.0.1", Port => $_ ) or die "Couldn't make 127.0.0.1 $_ [$@]";
} @ports;
while(<STDIN>) {
    chomp;
    my ($x,$y,$s,$v) = split(/,/,$_);
    next if $s == 0;
    # 90% depth 10% distance from center
    $v = 0.9 * ($s/1024.0) + 0.1 * (1 - sqrt($x*$x + $y*$y)/$distmax);
    # my @data = (550, (rand()*0.01), 0.1, max(0.0,min(1.0,$v)));
    my $osc = ["/grain/trigger",
               "i", 550, "f", (rand()*0.01), "f", 0.1, "f", max(0.0,min(1.0,$v))
              ];
    #$str = join(" ",("i550",(rand()*0.01),0.1, max(0.0,min(1.0,$v)),$/));
    warn join(" ", @$osc);
    # print $str;
    $clients[choose($x,$y)]->send($osc);
}

sub choose {
    my ($x,$y) = @_;
    if ($x > 640/2) {
        if ($y > 480/2) {
            return 0;
        } else {
            return 1;
        }
    } else {
        if ($y > 480/2) {
            return 2;
        } else {
            return 3;
        }
    }
}

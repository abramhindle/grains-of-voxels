#!/usr/bin/perl
use strict;
use Time::HiRes qw(usleep);
use List::Util qw(min max);
$| = 1;
my $cur = 0.0;
my $i = 0;
my $v = 0;
my $str = "";
my @vals;
my @ovals;
my $range;
my $distmax = sqrt(640*640.0 + 480*480);
while(<STDIN>) {
    chomp;
    my ($x,$y,$s,$v) = split(/,/,$_);
    next if $s == 0;
    # 90% depth 10% distance from center
    $v = 0.9 * ($s/1024.0) + 0.1 * (1 - sqrt($x*$x + $y*$y)/$distmax);
    $str = join(" ",("i550",(rand()*0.01),0.1, max(0.0,min(1.0,$v)),$/));
    warn $str;
    print $str;
}

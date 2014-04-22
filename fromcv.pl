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
while(<STDIN>) {
    chomp;
    @vals = split(/,/,$_);
    $range = @vals * 1024;
    for ($i = 0; $i < @vals; $i++) {
        $v = $vals[$i];
        #next if ($v == $ovals[$i]);
        next if $v == 0;
        next if $v == 1024;
        $v = ($v + $i * 1024) / $range;
        #$str = join(" ",("i556",(rand()*0.1),0.05+rand()*0.15, rand() * 5000, 1.0+1.0*rand(), max(0.0,min(1.0,$v)), 1.0,$/));
        $str = join(" ",("i550",(rand()*0.01),0.1, max(0.0,min(1.0,$v)),$/));
        #$str = join(" ",("i556",(rand()*0.01),0.1, 5000, 4.0, max(0.0,min(1.0,$v)), 1.0,$/));
        warn $str;
        print $str;
    }
    @ovals = @vals;
}

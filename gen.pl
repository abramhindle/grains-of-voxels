use Time::HiRes qw(usleep);
use List::Util qw(min max);
$| = 0;
my $cur = 0.0;
while(1) {
    my $str = join(" ",("i556",(rand()*0.1),rand()*0.2, rand() * 5000, 1.0+rand(), min(1.0,$cur + 0.00001 * rand()), 1.0,$/));
    warn $str;
    print $str;
    $cur += 0.00001*rand();
    $cur = ($cur > 1.0)?0:$cur;
    usleep(1000*rand());
}

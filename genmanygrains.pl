#!/usr/bin/perl
use strict;
use HTML::Template;
my @ports = qw(4101 4102 4103 4104);

my $template = HTML::Template->new(filename => 'manygrain.csd.tmpl',
                                   "die_on_bad_params"=>0 );
for my $four (0,1) {
    for my $port (@ports) {
        $template->param(PORT=>$port);
        $template->param(FOUR=>$four);
        open(my $fd, ">", "manygrain-$port-$four.csd");
        print $fd $template->output();
        close($fd);
    }
}


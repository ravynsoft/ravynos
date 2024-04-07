#!perl

use strict;
use warnings;

use Test::More tests => 5;
use HTTP::Tiny;

# Just make sure timeout is handled correctly as a constructor param,
# and that it works as expected as an "attribute".

my $default = 60;

{
    my $ua = HTTP::Tiny->new();
    is $ua->timeout, $default, 'default timeout is as expected';
}

{
    my $ua = HTTP::Tiny->new(timeout => 10);
    is $ua->timeout, 10, 'timeout is handled as a constructor param';
}

{
    my $ua = HTTP::Tiny->new(timeout => 0);
    is $ua->timeout, 0, 'constructor arg of timeout=0 is passed through';
}

{
    my $ua = HTTP::Tiny->new(timeout => undef);
    is $ua->timeout, $default, 'constructor arg of timeout=undef is ignored';
}

{
    my $ua = HTTP::Tiny->new();
    $ua->timeout(15);
    is $ua->timeout, 15, 'timeout works as expected as a r/w attribute';
}

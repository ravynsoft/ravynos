#!perl

use strict;
use warnings;

use Test::More tests => 8;
use HTTP::Tiny;

# a couple tests to ensure that we get the default agent expected, the correct
# agent when specified, and the correct agent when specified with a space at
# the end of the string (as LWP::UserAgent does)


my $default = 'HTTP-Tiny/' . (HTTP::Tiny->VERSION || 0);

{
    my $ua = HTTP::Tiny->new();
    is $ua->agent, $default, 'default agent string is as expected';
}

{
    my $ua = HTTP::Tiny->new(agent => 'something else');
    is $ua->agent, 'something else', 'agent string is as expected';
}

{
    my $ua = HTTP::Tiny->new(agent => 'something else ');
    is
        $ua->agent,
        "something else $default",
        'agent string is as properly appended to',
        ;
}

{
    my $ua = HTTP::Tiny->new();

    is( HTTP::Tiny->_agent(), $default, 'check _agent on class' );
    is $ua->_agent(), $default, 'check _agent on object';

    $ua->agent(undef);
    is $ua->agent, undef, 'agent string is empty';

    $ua->agent('something else');
    is $ua->agent, 'something else', 'agent string is as expected';

    $ua->agent('something else ');
    is
        $ua->agent,
        "something else $default",
        'agent string is as properly appended to',
        ;
}

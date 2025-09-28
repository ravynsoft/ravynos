#!perl

BEGIN {
    require Config;
    import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/) {
	print "1..0\n";
	exit 0;
    }
}

use strict;
use warnings;
use Test::More;
use Safe;
plan(tests => 1);

my $c = new Safe;

{
    package My::Controller;
    sub jopa { return "jopa" }
}

$c->reval(q{
    package My::Controller;
    sub jopa { return "hacked" }

    My::Controller->jopa; # let it cache package
});

is(My::Controller->jopa, "jopa", "outside packages cannot be overriden");

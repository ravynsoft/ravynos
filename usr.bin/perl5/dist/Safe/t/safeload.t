#!perl

BEGIN {
    require Config;
    import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/) {
	print "1..0\n";
	exit 0;
    }
    # Can we load the version module ?
    eval { require version; 1 } or do {
	print "1..0 # no version.pm\n";
	exit 0;
    };
    delete $INC{"version.pm"};
}

use strict;
use Test::More;
use Safe;
plan(tests => 3);

my $c = new Safe;
$c->permit(qw(require caller entereval unpack rand));
my $r = $c->reval(q{ use version; 1 });
ok( defined $r, "Can load version.pm in a Safe compartment" ) or diag $@;

$r = $c->reval(q{ version->new(1.2) });
is(ref $r, "Safe::Root0::version", "version objects rerooted");
$r or diag $@;

# Does this test really belong here?  We are testing the "loading" of
# a perl version number.
# This should died because of strictures under 5.12+ and because of the
# perl version in 5.10-.
ok !$c->reval(q{use 5.012; $undeclared; 1}),
   'reval does not prevent use 5.012 from enabling strict';

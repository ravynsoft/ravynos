#!perl -w

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/) {
        print "1..0\n";
        exit 0;
    }
}

use Test::More tests => 3;
use Safe;

my $c; my $r;
my $snippet = q{
    my $foo = qr/foo/;
    ref $foo;
};
$c = new Safe;
$r = $c->reval($snippet);
is( $r, "Safe::Root0::Regexp" );
$r or diag $@;

# once more with the same compartment
# (where DESTROY has been cleaned up)
$r = $c->reval($snippet);
is( $r, "Safe::Root0::Regexp" );
$r or diag $@;

# try with a new compartment
$c = new Safe;
$r = $c->reval($snippet);
is( $r, "Safe::Root1::Regexp" );
$r or diag $@;

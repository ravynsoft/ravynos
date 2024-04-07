#!./perl

use strict;
use warnings;

use Scalar::Util qw(readonly);
use Test::More tests => 11;

ok( readonly(1), 'number constant');

my $var = 2;

ok( !readonly($var), 'number variable');
is( $var, 2, 'no change to number variable');

ok( readonly("fred"), 'string constant');

$var = "fred";

ok( !readonly($var),  'string variable');
is( $var, 'fred', 'no change to string variable');

$var = \2;

ok( !readonly($var), 'reference to constant');
ok( readonly($$var), 'de-reference to constant');

ok( !readonly(*STDOUT), 'glob');

sub try
{
    my $v = \$_[0];
    return readonly $$v;
}

$var = 123;
{
    # This used not to work with ithreads, but seems to be working since 5.19.3
    local $TODO = ( $Config::Config{useithreads} && $] < 5.019003 ) ?
      "doesn't work with threads" : undef;
    ok( try ("abc"), 'reference a constant in a sub');
}
ok( !try ($var), 'reference a non-constant in a sub');

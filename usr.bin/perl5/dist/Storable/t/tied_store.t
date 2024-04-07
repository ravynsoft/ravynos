#!./perl

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Storable ();
use Test::More tests => 3;

our $f;

package TIED_HASH;

sub TIEHASH { bless({}, $_[0]) }

sub STORE {
	$f = Storable::freeze(\$_[2]);
	1;
}

package TIED_ARRAY;

sub TIEARRAY { bless({}, $_[0]) }

sub STORE {
	$f = Storable::freeze(\$_[2]);
	1;
}

package TIED_SCALAR;

sub TIESCALAR { bless({}, $_[0]) }

sub STORE {
	$f = Storable::freeze(\$_[1]);
	1;
}

package main;

my($s, @a, %h);
tie $s, "TIED_SCALAR";
tie @a, "TIED_ARRAY";
tie %h, "TIED_HASH";

$f = undef;
$s = 111;
is $f, Storable::freeze(\111);

$f = undef;
$a[3] = 222;
is $f, Storable::freeze(\222);

$f = undef;
$h{foo} = 333;
is $f, Storable::freeze(\333);

1;

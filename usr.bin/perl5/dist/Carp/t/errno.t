use warnings;
use strict;

use Test::More tests => 20;

use Carp ();

sub AA::CARP_TRACE { $! = 42; $^E = 42; "Tracy" }
my $aa = bless({}, "AA");

my($m, $errno, $exterr);

$! = 69; $^E = 69;
sub lmm { Carp::longmess("x") }
sub lm { lmm() }
$m = lm($aa);
$errno = 0+$!; $exterr = 0+$^E;
like $m, qr/Tracy/;
is $errno, 69;
is $exterr, 69;

$! = 69; $^E = 69;
sub sm { Carp::shortmess("x") }
$m = sm($aa);
$errno = 0+$!; $exterr = 0+$^E;
like $m, qr/Tracy/;
is $errno, 69;
is $exterr, 69;

$SIG{__WARN__} = sub { $m = $_[0]; $errno = 0+$!; $exterr = 0+$^E; };

$! = 69; $^E = 69;
$m = $errno = $exterr = undef;
sub cl { Carp::cluck("x") }
cl($aa);
like $m, qr/Tracy/;
is $errno, 69;
is $exterr, 69;

$! = 69; $^E = 69;
$m = $errno = $exterr = undef;
sub cp { Carp::carp("x") }
cp($aa);
like $m, qr/Tracy/;
is $errno, 69;
is $exterr, 69;

$SIG{__DIE__} = $SIG{__WARN__};
delete $SIG{__WARN__};

$! = 69; $^E = 69;
$m = $errno = $exterr = undef;
sub cf { Carp::confess("x") }
eval { cf($aa) };
like $@, qr/Tracy/;
like $m, qr/Tracy/;
is $errno, 69;
is $exterr, 69;

$! = 69; $^E = 69;
$m = $errno = $exterr = undef;
sub ck { Carp::croak("x") }
eval { ck($aa) };
like $@, qr/Tracy/;
like $m, qr/Tracy/;
is $errno, 69;
is $exterr, 69;

delete $SIG{__DIE__};

1;

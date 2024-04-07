#!perl -w
use strict;

use Test::More tests => 5;
use XS::APItest;

{
    use feature "unicode_eval";
    my $unfiltered_foo = "foo";
    BEGIN {
      eval "BEGIN { filter() }";
      like $@, qr/^Source filters apply only to byte streams at /,
	'filters die under unicode_eval';
    }
    is "foo", $unfiltered_foo, 'filters leak not out of unicode evals';

    use feature "evalbytes";
    our $thingy;
    BEGIN { evalbytes "BEGIN { filter() }\n\$thingy = 'foo'" }
    is $thingy, "fee",
	"source filters apply to evalbytten strings";
    is "foo", $unfiltered_foo, 'filters leak not out of byte evals';
}

BEGIN { eval "BEGIN{ filter() }" }

is "foo", "fee", "evals share filters with the currently compiling scope";
# See [perl #87064].

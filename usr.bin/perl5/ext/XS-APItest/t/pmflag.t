#!perl
use strict;
use Test::More 'tests' => 2;

ok(!eval q{use XS::APItest 'pmflag'; 1}, "Perl_pmflag\(\) removed");
like($@, qr{\Wpmflag\W\s+is\s+not\s+exported\b}, "pmflag not exported");


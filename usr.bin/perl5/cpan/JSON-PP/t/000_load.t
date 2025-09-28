# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;

my $loaded;
BEGIN { $| = 1; print "1..1\n"; }
END {print "not ok 1\n" unless $loaded;}
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;
$loaded = 1;
print "ok 1\n";

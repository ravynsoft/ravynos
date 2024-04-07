use strict;
use warnings;
use Test2::Tools::Tiny;

BEGIN {
    skip_all "Not testing before 5.18 or after 5.37.10"
        if $] < 5.018 or $] >= 5.037010;
}

require Test::More;
*cmp_ok = \&Test::More::cmp_ok;

no warnings "experimental::smartmatch";
no if !exists $warnings::Offsets{"experimental::smartmatch"}, warnings => 'deprecated';

my $warnings = warnings { cmp_ok(1, "~~", 1) };

ok(!@$warnings, "Did not get any warnings");

done_testing;

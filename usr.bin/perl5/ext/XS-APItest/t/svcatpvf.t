use strict;
use warnings;

use Test::More tests => 6;

use XS::APItest;

my @cases = (
    [field     => '%2$d'],
    [precision => '%.*2$d'],
    [vector    => '%2$vd'],
    [width     => '%*2$d'],
);

for my $case (@cases) {
    my ($what, $format) = @$case;
    my $got = eval { test_sv_catpvf($format); 1 };
    my $exn = $got ? undef : $@;
    like($exn, qr/\b\QCannot yet reorder sv_vcatpvfn() arguments from va_list\E\b/,
         "explicit $what index forbidden in va_list arguments");
}

# these actually test newSVpvf() but it is the same underlying logic.
is(test_HvNAMEf(bless {}, "Whatever::You::Like"),
    "class='Whatever::You::Like'");
is(test_HvNAMEf_QUOTEDPREFIX(bless {}, "x" x 1000),
    'class="xxxxxxxxxxxxxxxxxxxxxxxxxx'.
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'.
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"..."xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'.
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'.
    'xxxxxxxxxxxxxxxxxxxxx"');

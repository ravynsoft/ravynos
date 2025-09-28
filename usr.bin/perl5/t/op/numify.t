#! ./perl

# Test string-to-number conversions.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

# Quick test if NV supports infinities.
# Note that this would be $Config{d_double_has_inf}, but this is only valid
# if NV is configured as double.
my $nv_has_inf = do { no warnings; 'inf' > 0 };

foreach ([' +3', 3, 0],
         ["10.\t", 10, 0],
         ['abc', 0, 1],
         ['- +3', 0, 1],        # GH 18584
         ['++4', 0, 1],
         ['0x123', 0, 1],
         ['1x123', 1, 1],
         ['+0x456', 0, 1],
         ['- 0x789', 0, 1],
         ['0b101', 0, 1],
         ['-3.14', -3.14, 0],
         ['- 3.14', 0, 1],
         ($nv_has_inf ?
          (['+infinity ', '+Inf', 0],
           ['  -infin', '-Inf', 1],
           ['+ inf', 0, 1],
           ['+-inf', 0, 1]) :
          ())
    ) {
    my ($str, $num, $warn) = @$_;

    my $code = sub {
        cmp_ok($str + 0, '==', $num, "numifying '$str'");
    };

    if ($warn) {
        warning_like($code, qr/^Argument ".*" isn't numeric/,
                     "numifying '$str' trigger a warning");
    }
    else {
        warning_is($code, undef,
                   "numifying '$str' does not trigger warnings");
    }
}

done_testing();

#! ./perl -w

# These tests are in a separate .t file, because they may change
# execution environment of the perl process.

use strict;
use warnings;

use Test::More;
use POSIX qw/:fenv_h :float_h/;

my $defmode;
plan skip_all => 'fegetround is unavailable'
    unless eval { $defmode = fegetround(); 1 };

ok(defined $defmode, 'fegetround');

SKIP: {
    skip 'default rounding mode is not FE_TONEAREST', 1
        unless eval { $defmode == FE_TONEAREST() };
    my $flt_rounds;
    skip 'FLT_ROUNDS is unavailable', 1
        unless eval { $flt_rounds = FLT_ROUNDS(); 1 };
    cmp_ok($flt_rounds, '==', 1, 'FLT_ROUNDS');
}

cmp_ok(fesetround($defmode), '==', 0, 'fesetround');
cmp_ok(fegetround(), '==', $defmode, 'fesetround/fegetround round-trip');

my @rounding = qw/TOWARDZERO TONEAREST UPWARD DOWNWARD/;

for (my $i = 0; $i < @rounding; $i++) {
  SKIP: {
      my $macro = "FE_$rounding[$i]";
      my $femode = eval "$macro()";
      skip "no support for FE_$rounding[$i]", 3
          unless defined $femode;

      cmp_ok(fesetround($femode), '==', 0, "fesetround($macro)");
      cmp_ok(fegetround(), '==', $femode, "fegetround() under $macro");
      cmp_ok(FLT_ROUNDS, '==', $i, "FLT_ROUNDS under $macro");
    }
}

# Revert to default rounding mode
fesetround($defmode);

done_testing();

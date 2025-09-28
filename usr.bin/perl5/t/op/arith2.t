#!./perl

# These Config-dependent tests were originally in t/opbasic/arith.t,
# but moved here because t/opbasic/* should not depend on sophisticated
# constructs like "use Config;".

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use Config;
use strict;

plan tests => 9;

my $vms_no_ieee;
if ($^O eq 'VMS') {
     $vms_no_ieee = 1 unless defined($Config{useieee});
}

SKIP:
{
    if ($^O eq 'vos') {
        skip "VOS raises SIGFPE instead of producing infinity", 1;
    }
    elsif ($vms_no_ieee || !$Config{d_double_has_inf}) {
        skip "the IEEE infinity model is unavailable in this configuration", 1;
    }
    # The computation of $v should overflow and produce "infinity"
    # on any system whose max exponent is less than 10**1506.
    # The exact string used to represent infinity varies by OS,
    # so we don't test for it; all we care is that we don't die.
    #
    # Perl considers it to be an error if SIGFPE is raised.
    # Chances are the interpreter will die, since it doesn't set
    # up a handler for SIGFPE.  That's why this test is last; to
    # minimize the number of test failures.  --PG

    my $n = 5000;
    my $v = 2;
    while (--$n) {
        $v *= 2;
    }
    pass("infinity");
}


# [perl #120426]
# small numbers shouldn't round to zero if they have extra floating digits

SKIP:
{
    skip "not IEEE", 8 unless $Config{d_double_style_ieee};
    ok 0.153e-305 != 0.0,              '0.153e-305';
    ok 0.1530e-305 != 0.0,             '0.1530e-305';
    ok 0.15300e-305 != 0.0,            '0.15300e-305';
    ok 0.153000e-305 != 0.0,           '0.153000e-305';
    ok 0.1530000e-305 != 0.0,          '0.1530000e-305';
    ok 0.1530001e-305 != 0.0,          '0.1530001e-305';
    ok 1.17549435100e-38 != 0.0,       'min single';
    # For flush-to-zero systems this may flush-to-zero, see PERL_SYS_FPU_INIT
    ok 2.2250738585072014e-308 != 0.0, 'min double';
}

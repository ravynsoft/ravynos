#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 24;

# not() tests
pass("logical negation of empty list") if not();
is(not(), 1, "logical negation of empty list in numeric comparison");
is(not(), not(0),
    "logical negation of empty list compared with logical negation of false value");

# test not(..) and !
note("parens needed around second argument in next two tests\nto preserve list context inside function call");
is(! 1, (not 1),
    "high- and low-precedence logical negation of true value");
is(! 0, (not 0),
    "high- and low-precedence logical negation of false value");
is(! (0, 0), not(0, 0),
    "high- and low-precedence logical negation of lists");

# test the return of !
{
    my $not0 = ! 0;
    my $not1 = ! 1;

    no warnings;
    ok($not1 == undef,
        "logical negation (high-precedence) of true value is numerically equal to undefined value");
    ok($not1 == (),
        "logical negation (high-precedence) of true value is numerically equal to empty list");

    use warnings;
    ok($not1 eq '',
        "logical negation (high-precedence) of true value in string context is equal to empty string");
    ok($not1 == 0,
        "logical negation (high-precedence) of true value is false in numeric context");
    ok($not0 == 1,
        "logical negation (high-precedence) of false value is true in numeric context");
}

# test the return of not
{
    my $not0 = not 0;
    my $not1 = not 1;

    no warnings;
    ok($not1 == undef,
        "logical negation (low-precedence) of true value is numerically equal to undefined value");
    ok($not1 == (),
        "logical negation (low-precedence) of true value is numerically equal to empty list");

    use warnings;
    ok($not1 eq '',
        "logical negation (low-precedence) of true value in string context is equal to empty string");
    ok($not1 == 0,
        "logical negation (low-precedence) of true value is false in numeric context");
    ok($not0 == 1,
        "logical negation (low-precedence) of false value is true in numeric context");
}

# test truth of dualvars
SKIP:
{
    my $got_dualvar;
    eval 'use Scalar::Util "dualvar"; $got_dualvar++';
    skip "No Scalar::Util::dualvar", 3 unless $got_dualvar;
    my $a = Scalar::Util::dualvar(3, "");
    is not($a), 1, 'not(dualvar) ignores int when string is false';
    my $b = Scalar::Util::dualvar(3.3,"");
    is not($b), 1, 'not(dualvar) ignores float when string is false';
    my $c = Scalar::Util::dualvar(0,"1");
    is not($c), "", 'not(dualvar) ignores false int when string is true';
}

# test truth of regexps
is not(${qr//}), "", 'dereferenced regexps are true';

# not’s return value should be read-only, as it is the same global scalar
# each time (and test that it is, too).
*yes = \not 0;
*no  = \not 1;
for (!0) { eval { $_ = 43 } }
like $@, qr/^Modification of a read-only value attempted at /,
   'not 0 is read-only';
for (!1) { eval { $_ = 43 } }
like $@, qr/^Modification of a read-only value attempted at /,
   'not 1 is read-only';
require Config;
is \!0, \$yes, '!0 returns the same value each time [perl #114838]';
is \!1, \$no,  '!1 returns the same value each time [perl #114838]';

#! perl -w

# Test integer constants (DBL_DIG, DBL_MAX_EXP, FP_*, ...) are IV, not NV.

use strict;
use Test::More;
use Devel::Peek;
use POSIX;
use Config;

# Capture output from Devel::Peek::Dump() into Perl string
sub capture_dump
{
    open my $olderr, '>&', *STDERR
        or die "Can't save STDERR: $!";
    my $str;
    my $result = eval {
        local $SIG{__DIE__};
        close STDERR;
        open STDERR, '>', \$str
            or die "Can't redirect STDERR: $!";
        Dump($_[0]);
        1;
    };
    my $reason = $@;
    open STDERR, '>&', $olderr
        or die "Can't restore STDERR: $!";
    $result or die $reason;
    $str;
}

# Avoid die() in a test harness.
sub capture_dump_in_test
{
    my $str;
    eval { $str = capture_dump($_[0]); 1 } or BAIL_OUT $@;
    $str;
}

sub is_iv ($$)
{
    # We would write "ok(SvIOK($_[0]), ...)",
    # but unfortunately SvIOK is not available in Perl.

    my $dump = capture_dump_in_test($_[0]);
    #note($dump);
    ok($dump =~ /^\h*FLAGS = .*\bIOK\b/m && $dump =~ /^\h*IV =/m, $_[1]);
}

my @tests = qw(EXIT_SUCCESS);

push @tests, qw(FLT_RADIX FP_NORMAL FP_ZERO FP_SUBNORMAL FP_INFINITE FP_NAN);

if ($Config{uselongdouble} ? $Config{d_ilogbl} : $Config{d_ilogb}) {
    push @tests, qw(FP_ILOGB0);
    push @tests, qw(FP_ILOGBNAN) if $Config{d_double_has_nan};
}

foreach my $flt ('FLT', 'DBL', ($Config{d_longdbl} ? ('LDBL') : ())) {
    push @tests, "${flt}_$_"
        foreach qw(DIG MANT_DIG MAX_10_EXP MAX_EXP MIN_10_EXP MIN_EXP);
}

push @tests, qw(FE_TONEAREST FE_TOWARDZERO FE_UPWARD FE_DOWNWARD)
    if $Config{d_fegetround};

is_iv(eval "POSIX::$_", "$_ is an integer") foreach @tests;

SKIP: {
    my $x;
    skip $@, 1 unless eval '$x = FLT_ROUNDS; 1';
    is_iv($x, 'FLT_ROUNDS is an integer');
}

done_testing();

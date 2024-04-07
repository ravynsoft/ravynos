#! ./perl

# Check conversions of PV to NV/IV/UV

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_without_dynamic_extension('Devel::Peek');
}

use strict;
use warnings;
use Devel::Peek;
use Config;

# Use Devel::Peek::Dump in order to investigate SV flags for checking
# conversion behavior precisely.
# But unfortunately Devel::Peek::Dump always outputs to stderr, so
# a small wrapper to capture stderr into Perl string is implemented here
# to automate the test.

package STDERRSaver {
    sub new {
        open my $old, '>&', *STDERR or die "Can't save STDERR: $!";
        close STDERR;
        open STDERR, $_[1], $_[2] or die "Can't redirect STDERR: $!";
        bless \$old, $_[0] || __PACKAGE__;
    }
    sub DESTROY {
        open STDERR, '>&', ${$_[0]} or die "Can't restore STDERR: $!";
        close ${$_[0]};
    }
}

# These functions use &sub form to minimize argument manipulation.

sub capture_dump
{
    my $str;
    my @warnings;
    eval {
        local $SIG{__WARN__} = sub { push @warnings, $_[0] };
        my $err = STDERRSaver->new('>', \$str);
        &Dump;
        !0;
    } or BAIL_OUT $@;           # Avoid die() under test.
    note(@warnings) if @warnings;
    $str;
}

# Implement Sv*OK in Perl.

sub sv_flags
{
    my $dump = &capture_dump;
    $dump =~ /^\h*FLAGS\h*=\h*\(\h*(.*?)\h*\)/m # be tolerant
        or note($dump), BAIL_OUT 'Cannot parse Devel::Peek::Dump output';
    +{ map { $_ => !0 } split /\h*,\h*/, $1 };
}

sub SvUOK
{
    my $flags = &sv_flags;
    $flags->{IOK} && $flags->{IsUV};
}

sub SvUOKp
{
    my $flags = &sv_flags;
    $flags->{pIOK} && $flags->{IsUV};
}

sub SvIOKp_notIOK_notUV
{
    my $flags = &sv_flags;
    $flags->{pIOK} && !$flags->{IOK} && !$flags->{IsUV};
}

sub SvIOK_notUV
{
    my $flags = &sv_flags;
    $flags->{IOK} && !$flags->{IsUV};
}

sub SvNOK
{
    (&sv_flags)->{NOK};
}

# This will be a quick test of Sv*OK* implemented here.
ok(SvIOK_notUV(2147483647), '2147483647 is not UV');

{
    my $x = '12345.67';
    my $y = $x;
    my $z = $y << 0;            # "<<" requires UV operands
    is($z, 12345, "string '$x' to UV conversion");
    ok(SvIOKp_notIOK_notUV($y), 'string to UV conversion caches IV');
    is($y >> 0, 12345, 'reusing cached IV');
}

{
    my $x = '40e+8';
    my $y = $x;
    my $z = $y | 0;             # "|" also requires UV operands
    is($z, 4000000000, "string '$x' to UV conversion");
    ok(SvNOK($y), "string to UV conversion caches NV");
    ok(SvUOK(4000000000) ? SvUOK($y) : SvIOK_notUV($y),
       'string to UV conversion caches IV or UV');
    is($y ^ 0, 4000000000, 'reusing cached IV or UV');
}

my $uv_max = ~0;

{
    my $x = $uv_max * 7;        # Some large value not representable in IV/UV
    my $y = "$x";               # Convert to string
    my $z = $y << 0;
    is($z, $uv_max, 'large value in string is coerced to UV_MAX when UV is requested');
    ok(SvUOKp($y), 'converted UV is cached');
    is($y >> 0, $uv_max, 'reusing cached UV_MAX');
    my $v = $x << 0;            # Now NV to UV conversion
    is($v, $uv_max, 'large NV is coerced to UV_MAX when UV is requested');
    ok(SvUOKp($v), 'converted UV is cached');
    is($x >> 0, $uv_max, 'reusing cached UV_MAX');
}

done_testing();

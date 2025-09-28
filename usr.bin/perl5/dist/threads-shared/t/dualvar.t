use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use ExtUtils::testlib;

BEGIN {
    $| = 1;
    print("1..226\n");    ### Number of tests that will be run ###
}

use threads;
use threads::shared;
use Scalar::Util qw(dualvar);

my $TEST = 1;

sub ok {
    my ($ok, $name) = @_;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $TEST - $name\n");
    } else {
        print("not ok $TEST - $name\n");
        printf("# Failed test at line %d\n", (caller(1))[2]);
    }

    $TEST++;
}

sub ok_iv
{
    my ($var, $iv) = @_;
    ok($var == $iv, 'IV number preserved');
    ok($var eq $iv, 'String preserved');
}

sub ok_nv
{
    my ($var, $nv) = @_;
    ok($var == $nv, 'NV number preserved');
    ok($var eq $nv, 'String preserved');
}

sub ok_uv
{
    my ($var, $uv) = @_;
    ok($var == $uv, 'UV number preserved');
    ok($var > 0, 'UV number preserved');
    ok($var eq $uv, 'String preserved');
}

### Start of Testing ###

my $iv = dualvar(42, 'Fourty-Two');
my $nv = dualvar(3.14, 'PI');
my $bits = ($Config{'use64bitint'}) ? 63 : 31;
my $uv = dualvar(1<<$bits, 'Large unsigned int');

print("# Shared scalar assignment using shared_clone()\n");

my $siv :shared = shared_clone($iv);
my $snv :shared = shared_clone($nv);
my $suv :shared = shared_clone($uv);

ok_iv($siv, $iv);
ok_nv($snv, $nv);
ok_uv($suv, $uv);

{
    print("# Shared array initialization\n");

    my @ary :shared = ($iv, $nv, $uv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);
}

{
    print("# Shared array list assignment\n");

    my @ary :shared;
    @ary = ($iv, $nv, $uv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);
}

{
    print("# Shared array element assignment\n");

    my @ary :shared;
    $ary[0] = $iv;
    $ary[1] = $nv;
    $ary[2] = $uv;

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);
}

{
    print("# Shared array initialization - shared scalars\n");

    my @ary :shared = ($siv, $snv, $suv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);
}

{
    print("# Shared array list assignment - shared scalars\n");

    my @ary :shared;
    @ary = ($siv, $snv, $suv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);
}

{
    print("# Shared array element assignment - shared scalars\n");

    my @ary :shared;
    $ary[0] = $siv;
    $ary[1] = $snv;
    $ary[2] = $suv;

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);
}

{
    print("# Shared hash initialization\n");

    my %hsh :shared = (
        'iv' => $iv,
        'nv' => $nv,
        'uv' => $uv,
    );

    ok_iv($hsh{'iv'}, $iv);
    ok_nv($hsh{'nv'}, $nv);
    ok_uv($hsh{'uv'}, $uv);
}

{
    print("# Shared hash assignment\n");

    my %hsh :shared;
    %hsh = (
        'iv' => $iv,
        'nv' => $nv,
        'uv' => $uv,
    );

    ok_iv($hsh{'iv'}, $iv);
    ok_nv($hsh{'nv'}, $nv);
    ok_uv($hsh{'uv'}, $uv);
}

{
    print("# Shared hash element assignment\n");

    my %hsh :shared;
    $hsh{'iv'} = $iv;
    $hsh{'nv'} = $nv;
    $hsh{'uv'} = $uv;

    ok_iv($hsh{'iv'}, $iv);
    ok_nv($hsh{'nv'}, $nv);
    ok_uv($hsh{'uv'}, $uv);
}

{
    print("# Shared hash initialization - shared scalars\n");

    my %hsh :shared = (
        'iv' => $siv,
        'nv' => $snv,
        'uv' => $suv,
    );

    ok_iv($hsh{'iv'}, $iv);
    ok_nv($hsh{'nv'}, $nv);
    ok_uv($hsh{'uv'}, $uv);
}

{
    print("# Shared hash assignment - shared scalars\n");

    my %hsh :shared;
    %hsh = (
        'iv' => $siv,
        'nv' => $snv,
        'uv' => $suv,
    );

    ok_iv($hsh{'iv'}, $iv);
    ok_nv($hsh{'nv'}, $nv);
    ok_uv($hsh{'uv'}, $uv);
}

{
    print("# Shared hash element assignment - shared scalars\n");

    my %hsh :shared;
    $hsh{'iv'} = $siv;
    $hsh{'nv'} = $snv;
    $hsh{'uv'} = $suv;

    ok_iv($hsh{'iv'}, $iv);
    ok_nv($hsh{'nv'}, $nv);
    ok_uv($hsh{'uv'}, $uv);
}

{
    print("# Shared array push\n");

    my @ary :shared;
    push(@ary, $iv, $nv, $uv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);

    print("# Shared array pop\n");

    my $xuv = pop(@ary);
    my $xnv = pop(@ary);
    my $xiv = pop(@ary);

    ok_iv($xiv, $iv);
    ok_nv($xnv, $nv);
    ok_uv($xuv, $uv);

    print("# Shared array unshift\n");

    unshift(@ary, $iv, $nv, $uv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);

    print("# Shared array shift\n");

    $xiv = shift(@ary);
    $xnv = shift(@ary);
    $xuv = shift(@ary);

    ok_iv($xiv, $iv);
    ok_nv($xnv, $nv);
    ok_uv($xuv, $uv);
}

{
    print("# Shared array push - shared scalars\n");

    my @ary :shared;
    push(@ary, $siv, $snv, $suv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);

    print("# Shared array pop - shared scalars\n");

    my $xuv = pop(@ary);
    my $xnv = pop(@ary);
    my $xiv = pop(@ary);

    ok_iv($xiv, $iv);
    ok_nv($xnv, $nv);
    ok_uv($xuv, $uv);

    print("# Shared array unshift - shared scalars\n");

    unshift(@ary, $siv, $snv, $suv);

    ok_iv($ary[0], $iv);
    ok_nv($ary[1], $nv);
    ok_uv($ary[2], $uv);

    print("# Shared array shift - shared scalars\n");

    $xiv = shift(@ary);
    $xnv = shift(@ary);
    $xuv = shift(@ary);

    ok_iv($xiv, $iv);
    ok_nv($xnv, $nv);
    ok_uv($xuv, $uv);
}

{
    print("# Shared hash delete\n");

    my %hsh :shared = (
        'iv' => $iv,
        'nv' => $nv,
        'uv' => $uv,
    );

    ok_iv(delete($hsh{'iv'}), $iv);
    ok_nv(delete($hsh{'nv'}), $nv);
    ok_uv(delete($hsh{'uv'}), $uv);
}

{
    print("# Shared hash delete - shared scalars\n");

    my %hsh :shared = (
        'iv' => $siv,
        'nv' => $snv,
        'uv' => $suv,
    );

    ok_iv(delete($hsh{'iv'}), $iv);
    ok_nv(delete($hsh{'nv'}), $nv);
    ok_uv(delete($hsh{'uv'}), $uv);
}

{
    print("# Shared array copy to non-shared array\n");

    my @ary :shared = ($iv, $nv, $uv);
    my @nsa = @ary;

    ok_iv($nsa[0], $iv);
    ok_nv($nsa[1], $nv);
    ok_uv($nsa[2], $uv);

    print("# Shared array copy using shared_clone()\n");

    my $copy :shared = shared_clone(\@nsa);

    ok_iv($$copy[0], $iv);
    ok_nv($$copy[1], $nv);
    ok_uv($$copy[2], $uv);
}

{
    print("# Shared array copy to non-shared array - shared scalars\n");

    my @ary :shared = ($siv, $snv, $suv);
    my @nsa = @ary;

    ok_iv($nsa[0], $iv);
    ok_nv($nsa[1], $nv);
    ok_uv($nsa[2], $uv);

    print("# Shared array copy using shared_clone()\n");

    my $copy :shared = shared_clone(\@nsa);

    ok_iv($$copy[0], $iv);
    ok_nv($$copy[1], $nv);
    ok_uv($$copy[2], $uv);
}

{
    print("# Shared hash copy to non-shared hash\n");

    my %hsh :shared = (
        'iv' => $iv,
        'nv' => $nv,
        'uv' => $uv,
    );
    my %nsh = %hsh;

    ok_iv($nsh{'iv'}, $iv);
    ok_nv($nsh{'nv'}, $nv);
    ok_uv($nsh{'uv'}, $uv);

    print("# Shared hash copy using shared_clone()\n");

    my $copy :shared = shared_clone(\%nsh);

    ok_iv($$copy{'iv'}, $iv);
    ok_nv($$copy{'nv'}, $nv);
    ok_uv($$copy{'uv'}, $uv);
}

{
    print("# Shared hash copy to non-shared hash - shared scalars\n");

    my %hsh :shared = (
        'iv' => $siv,
        'nv' => $snv,
        'uv' => $suv,
    );
    my %nsh = %hsh;

    ok_iv($nsh{'iv'}, $iv);
    ok_nv($nsh{'nv'}, $nv);
    ok_uv($nsh{'uv'}, $uv);

    print("# Shared hash copy using shared_clone()\n");

    my $copy :shared = shared_clone(\%nsh);

    ok_iv($$copy{'iv'}, $iv);
    ok_nv($$copy{'nv'}, $nv);
    ok_uv($$copy{'uv'}, $uv);
}

print("# Mix it up with a thread\n");
my @ary :shared;
my %hsh :shared;

threads->create(sub {
                    @ary = ($siv);
                    push(@ary, $snv);

                    %hsh = ( 'nv' => $ary[1] );
                    $hsh{'iv'} = $ary[0];
                    $hsh{'uv'} = $suv;

                    $ary[2] = $hsh{'uv'};
                })->join();

ok_iv($hsh{'iv'}, $ary[0]);
ok_nv($hsh{'nv'}, $ary[1]);
ok_uv($hsh{'uv'}, $ary[2]);

# $! behaves like a dualvar, but is really implemented as a tied SV.
# As a result sharing $! directly only propagates the string value.
# However, we can create a dualvar from it.
print("# Errno\n");
$! = 1;
my $ss :shared = dualvar($!,$!);
ok_iv($ss, $!);

exit(0);

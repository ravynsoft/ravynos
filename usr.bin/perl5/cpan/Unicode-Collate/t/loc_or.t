
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..20\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate::Locale;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

my $objOr = Unicode::Collate::Locale->
    new(locale => 'OR', normalization => undef);

ok($objOr->getlocale, 'or');

$objOr->change(level => 2);

ok($objOr->lt("\x{B2F}", "\x{B5F}"));

$objOr->change(level => 1);

ok($objOr->eq("\x{B2F}", "\x{B5F}"));

# 4

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objOr->lt("\x{B13}$t", "\x{B14}"));
    ok($objOr->lt("\x{B14}$t", "\x{B01}"));
    ok($objOr->lt("\x{B01}$t", "\x{B02}"));
    ok($objOr->lt("\x{B02}$t", "\x{B03}"));
    ok($objOr->lt("\x{B03}$t", "\x{B15}"));

    ok($objOr->lt("\x{B38}$t", "\x{B39}"));
    ok($objOr->lt("\x{B39}$t", "\x{B15}\x{B4D}\x{B37}"));
    ok($objOr->lt("\x{B15}\x{B4D}\x{B37}$t", "\x{B3D}"));
}

# 20


BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..25\n"; }
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

my $objSa = Unicode::Collate::Locale->
    new(locale => 'SA', normalization => undef);

ok($objSa->getlocale, 'sa');

$objSa->change(level => 2);

ok($objSa->lt("\x{902}", "\x{901}"));
ok($objSa->lt("\x{933}", "\x{934}"));

$objSa->change(level => 3);

ok($objSa->eq("\x{933}\x{93C}", "\x{934}"));

$objSa->change(level => 1);

ok($objSa->eq("\x{902}", "\x{901}"));
ok($objSa->eq("\x{933}", "\x{934}"));

# 7

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objSa->lt("\x{950}$t", "\x{902}"));
    ok($objSa->lt("\x{902}$t", "\x{903}"));
    ok($objSa->lt("\x{903}$t", "\x{A8FD}"));
    ok($objSa->lt("\x{903}$t", "\x{972}"));

    ok($objSa->lt("\x{938}$t", "\x{939}"));
    ok($objSa->lt("\x{939}$t", "\x{933}"));
    ok($objSa->lt("\x{933}$t", "\x{915}\x{94D}\x{937}"));
    ok($objSa->lt("\x{915}\x{94D}\x{937}$t", "\x{91C}\x{94D}\x{91E}"));
    ok($objSa->lt("\x{91C}\x{94D}\x{91E}$t", "\x{93D}"));
}

# 25

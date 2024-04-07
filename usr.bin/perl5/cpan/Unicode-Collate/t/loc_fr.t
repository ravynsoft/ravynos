
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..63\n"; }
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

my $ae = _pack_U(0xE6);
my $AE = _pack_U(0xC6);

my $objFr = Unicode::Collate::Locale->
    new(locale => 'FR', normalization => undef);

ok($objFr->getlocale, 'default'); # no tailoring since 1.18
ok($objFr->locale_version, undef);

$objFr->change(level => 1);

ok($objFr->eq($ae, "ae"));
ok($objFr->eq($AE, "AE"));
ok($objFr->eq("\x{1FD}", $ae));
ok($objFr->eq("\x{1FC}", $AE));
ok($objFr->eq("\x{1E3}", $ae));
ok($objFr->eq("\x{1E2}", $AE));

# 9

$objFr->change(level => 2);

ok($objFr->gt($ae, "ae"));
ok($objFr->gt($AE, "AE"));
ok($objFr->gt("\x{1FD}", $ae));
ok($objFr->gt("\x{1FC}", $AE));
ok($objFr->gt("\x{1E3}", $ae));
ok($objFr->gt("\x{1E2}", $AE));

ok($objFr->eq($ae, $AE));
ok($objFr->eq($AE, "\x{1D2D}"));
ok($objFr->eq("$ae\x{304}", "$AE\x{304}"));
ok($objFr->eq("$ae\x{301}", "$AE\x{301}"));

# 19

$objFr->change(level => 3);

ok($objFr->lt($ae, $AE));
ok($objFr->lt($AE, "\x{1D2D}"));
ok($objFr->lt("$ae\x{304}", "$AE\x{304}"));
ok($objFr->lt("$ae\x{301}", "$AE\x{301}"));

ok($objFr->eq("\x{1FD}", "$ae\x{301}"));
ok($objFr->eq("\x{1FC}", "$AE\x{301}"));
ok($objFr->eq("\x{1E3}", "$ae\x{304}"));
ok($objFr->eq("\x{1E2}", "$AE\x{304}"));

# 27

ok($objFr->eq("a\x{300}", _pack_U(0xE0)));
ok($objFr->eq("A\x{300}", _pack_U(0xC0)));
ok($objFr->eq("e\x{300}", _pack_U(0xE8)));
ok($objFr->eq("E\x{300}", _pack_U(0xC8)));
ok($objFr->eq("i\x{300}", _pack_U(0xEC)));
ok($objFr->eq("I\x{300}", _pack_U(0xCC)));
ok($objFr->eq("o\x{300}", _pack_U(0xF2)));
ok($objFr->eq("O\x{300}", _pack_U(0xD2)));
ok($objFr->eq("u\x{300}", _pack_U(0xF9)));
ok($objFr->eq("U\x{300}", _pack_U(0xD9)));
ok($objFr->eq("y\x{300}", "\x{1EF3}"));
ok($objFr->eq("Y\x{300}", "\x{1EF2}"));

ok($objFr->eq("a\x{301}", _pack_U(0xE1)));
ok($objFr->eq("A\x{301}", _pack_U(0xC1)));
ok($objFr->eq("e\x{301}", _pack_U(0xE9)));
ok($objFr->eq("E\x{301}", _pack_U(0xC9)));
ok($objFr->eq("i\x{301}", _pack_U(0xED)));
ok($objFr->eq("I\x{301}", _pack_U(0xCD)));
ok($objFr->eq("o\x{301}", _pack_U(0xF3)));
ok($objFr->eq("O\x{301}", _pack_U(0xD3)));
ok($objFr->eq("u\x{301}", _pack_U(0xFA)));
ok($objFr->eq("U\x{301}", _pack_U(0xDA)));
ok($objFr->eq("y\x{301}", _pack_U(0xFD)));
ok($objFr->eq("Y\x{301}", _pack_U(0xDD)));

ok($objFr->eq("a\x{308}", _pack_U(0xE4)));
ok($objFr->eq("A\x{308}", _pack_U(0xC4)));
ok($objFr->eq("e\x{308}", _pack_U(0xEB)));
ok($objFr->eq("E\x{308}", _pack_U(0xCB)));
ok($objFr->eq("i\x{308}", _pack_U(0xEF)));
ok($objFr->eq("I\x{308}", _pack_U(0xCF)));
ok($objFr->eq("o\x{308}", _pack_U(0xF6)));
ok($objFr->eq("O\x{308}", _pack_U(0xD6)));
ok($objFr->eq("u\x{308}", _pack_U(0xFC)));
ok($objFr->eq("U\x{308}", _pack_U(0xDC)));
ok($objFr->eq("y\x{308}", _pack_U(0xFF)));
ok($objFr->eq("Y\x{308}", "\x{178}"));

# 63

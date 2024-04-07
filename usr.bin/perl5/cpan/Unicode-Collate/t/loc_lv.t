
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..50\n"; }
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

my $objLv = Unicode::Collate::Locale->
    new(locale => 'LV', normalization => undef);

ok($objLv->getlocale, 'lv');

$objLv->change(level => 1);

ok($objLv->lt("c", "c\x{30C}"));
ok($objLv->gt("d", "c\x{30C}"));
ok($objLv->lt("g", "g\x{327}"));
ok($objLv->gt("h", "g\x{327}"));
ok($objLv->lt("k", "k\x{327}"));
ok($objLv->gt("l", "k\x{327}"));
ok($objLv->lt("l", "l\x{327}"));
ok($objLv->gt("m", "l\x{327}"));
ok($objLv->lt("n", "n\x{327}"));
ok($objLv->gt("o", "n\x{327}"));
ok($objLv->lt("r", "r\x{327}"));
ok($objLv->gt("s", "r\x{327}"));
ok($objLv->lt("s", "s\x{30C}"));
ok($objLv->gt("t", "s\x{30C}"));
ok($objLv->lt("z", "z\x{30C}"));
ok($objLv->lt("z\x{30C}", "\x{292}"));

# 18

$objLv->change(level => 2);

ok($objLv->eq("c\x{30C}", "C\x{30C}"));
ok($objLv->eq("g\x{327}", "G\x{327}"));
ok($objLv->eq("k\x{327}", "K\x{327}"));
ok($objLv->eq("l\x{327}", "L\x{327}"));
ok($objLv->eq("n\x{327}", "N\x{327}"));
ok($objLv->eq("r\x{327}", "R\x{327}"));
ok($objLv->eq("s\x{30C}", "S\x{30C}"));
ok($objLv->eq("z\x{30C}", "Z\x{30C}"));

# 26

$objLv->change(level => 3);

ok($objLv->lt("c\x{30C}", "C\x{30C}"));
ok($objLv->lt("g\x{327}", "G\x{327}"));
ok($objLv->lt("k\x{327}", "K\x{327}"));
ok($objLv->lt("l\x{327}", "L\x{327}"));
ok($objLv->lt("n\x{327}", "N\x{327}"));
ok($objLv->lt("r\x{327}", "R\x{327}"));
ok($objLv->lt("s\x{30C}", "S\x{30C}"));
ok($objLv->lt("z\x{30C}", "Z\x{30C}"));

# 34

ok($objLv->eq("c\x{30C}", "\x{10D}"));
ok($objLv->eq("C\x{30C}", "\x{10C}"));
ok($objLv->eq("g\x{327}", "\x{123}"));
ok($objLv->eq("G\x{327}", "\x{122}"));
ok($objLv->eq("k\x{327}", "\x{137}"));
ok($objLv->eq("K\x{327}", "\x{136}"));
ok($objLv->eq("l\x{327}", "\x{13C}"));
ok($objLv->eq("L\x{327}", "\x{13B}"));
ok($objLv->eq("n\x{327}", "\x{146}"));
ok($objLv->eq("N\x{327}", "\x{145}"));
ok($objLv->eq("r\x{327}", "\x{157}"));
ok($objLv->eq("R\x{327}", "\x{156}"));
ok($objLv->eq("s\x{30C}", "\x{161}"));
ok($objLv->eq("S\x{30C}", "\x{160}"));
ok($objLv->eq("z\x{30C}", "\x{17E}"));
ok($objLv->eq("Z\x{30C}", "\x{17D}"));

# 50

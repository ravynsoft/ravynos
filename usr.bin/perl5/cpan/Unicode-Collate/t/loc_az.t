
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..86\n"; }
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

my $objAz = Unicode::Collate::Locale->
    new(locale => 'AZ', normalization => undef);

ok($objAz->getlocale, 'az');

$objAz->change(level => 1);

ok($objAz->lt("c", "c\x{327}"));
ok($objAz->lt("cz","c\x{327}"));
ok($objAz->gt("d", "c\x{327}"));
ok($objAz->lt("g", "g\x{306}"));
ok($objAz->lt("gz","g\x{306}"));
ok($objAz->gt("h", "g\x{306}"));
ok($objAz->lt("h", "I"));
ok($objAz->lt("hz","I"));
ok($objAz->lt("I", "i"));
ok($objAz->lt("Iz","i"));
ok($objAz->gt("j", "i"));
ok($objAz->lt("o", "o\x{308}"));
ok($objAz->lt("oz","o\x{308}"));
ok($objAz->gt("p", "o\x{308}"));
ok($objAz->lt("s", "s\x{327}"));
ok($objAz->lt("sz","s\x{327}"));
ok($objAz->gt("t", "s\x{327}"));
ok($objAz->lt("u", "u\x{308}"));
ok($objAz->lt("uz","u\x{308}"));
ok($objAz->gt("v", "u\x{308}"));

# 22

ok($objAz->lt("k", "q"));
ok($objAz->lt("kz","q"));
ok($objAz->gt("l", "q"));
ok($objAz->lt("e", "\x{259}"));
ok($objAz->lt("ez","\x{259}"));
ok($objAz->gt("f", "\x{259}"));
ok($objAz->lt("h", "x"));
ok($objAz->lt("hz","x"));
ok($objAz->lt("x", "I"));
ok($objAz->lt("xz","I"));
ok($objAz->lt("x", "i"));
ok($objAz->lt("xz","i"));
ok($objAz->lt("z", "w"));
ok($objAz->lt("zz","w"));

# 36

$objAz->change(level => 2);

ok($objAz->eq("c\x{327}", "C\x{327}"));
ok($objAz->eq("g\x{306}", "G\x{306}"));
ok($objAz->eq("\x{131}", "I"));
ok($objAz->eq("\x{130}", "i"));
ok($objAz->eq("o\x{308}", "O\x{308}"));
ok($objAz->eq("s\x{327}", "S\x{327}"));
ok($objAz->eq("u\x{308}", "U\x{308}"));
ok($objAz->eq("q", "Q"));
ok($objAz->eq("\x{259}", "\x{18F}"));
ok($objAz->eq("x", "X"));
ok($objAz->eq("z", "Z"));

# 47

$objAz->change(level => 3);

ok($objAz->lt("c\x{327}", "C\x{327}"));
ok($objAz->lt("g\x{306}", "G\x{306}"));
ok($objAz->lt("\x{131}", "I"));
ok($objAz->gt("\x{130}", "i"));
ok($objAz->lt("o\x{308}", "O\x{308}"));
ok($objAz->lt("s\x{327}", "S\x{327}"));
ok($objAz->lt("u\x{308}", "U\x{308}"));
ok($objAz->lt("q", "Q"));
ok($objAz->lt("\x{259}", "\x{18F}"));
ok($objAz->lt("x", "X"));
ok($objAz->lt("z", "Z"));

# 58

ok($objAz->eq("c\x{327}", _pack_U(0xE7)));
ok($objAz->eq("C\x{327}", _pack_U(0xC7)));
ok($objAz->eq("g\x{306}", "\x{11F}"));
ok($objAz->eq("G\x{306}", "\x{11E}"));
ok($objAz->eq("I\x{300}", _pack_U(0xCC)));
ok($objAz->eq("I\x{301}", _pack_U(0xCD)));
ok($objAz->eq("I\x{302}", _pack_U(0xCE)));
ok($objAz->eq("I\x{308}", _pack_U(0xCF)));
ok($objAz->eq("I\x{304}", "\x{12A}"));
ok($objAz->eq("I\x{306}", "\x{12C}"));
ok($objAz->eq("I\x{328}", "\x{12E}"));
ok($objAz->eq("I\x{307}", "\x{130}"));

# 70

ok($objAz->eq("o\x{308}", _pack_U(0xF6)));
ok($objAz->eq("O\x{308}", _pack_U(0xD6)));
ok($objAz->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objAz->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objAz->eq("s\x{327}", "\x{15F}"));
ok($objAz->eq("S\x{327}", "\x{15E}"));
ok($objAz->eq("u\x{308}", _pack_U(0xFC)));
ok($objAz->eq("U\x{308}", _pack_U(0xDC)));
ok($objAz->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objAz->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objAz->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objAz->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objAz->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objAz->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objAz->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objAz->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 86

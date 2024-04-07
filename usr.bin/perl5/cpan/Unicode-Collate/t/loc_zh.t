
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..280\n"; }
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

my $objZh = Unicode::Collate::Locale->
    new(locale => 'ZH', normalization => undef);

ok($objZh->getlocale, 'zh');

$objZh->change(level => 1);

ok($objZh->eq("a\x{304}", "a\x{301}"));
ok($objZh->eq("a\x{301}", "a\x{30C}"));
ok($objZh->eq("a\x{30C}", "a\x{300}"));
ok($objZh->eq("a\x{300}", "a"));
ok($objZh->eq("e\x{304}", "e\x{301}"));
ok($objZh->eq("e\x{301}", "e\x{30C}"));
ok($objZh->eq("e\x{30C}", "e\x{300}"));
ok($objZh->eq("e\x{300}", "e"));
ok($objZh->eq("e\x{302}\x{304}", "e\x{302}\x{301}"));
ok($objZh->eq("e\x{302}\x{301}", "e\x{302}\x{30C}"));
ok($objZh->eq("e\x{302}\x{30C}", "e\x{302}\x{300}"));
ok($objZh->eq("e\x{302}\x{300}", "e\x{302}"));
ok($objZh->eq("i\x{304}", "i\x{301}"));
ok($objZh->eq("i\x{301}", "i\x{30C}"));
ok($objZh->eq("i\x{30C}", "i\x{300}"));
ok($objZh->eq("i\x{300}", "i"));
ok($objZh->eq("m\x{304}", "m\x{301}"));
ok($objZh->eq("m\x{301}", "m\x{30C}"));
ok($objZh->eq("m\x{30C}", "m\x{300}"));
ok($objZh->eq("m\x{300}", "m"));
ok($objZh->eq("n\x{304}", "n\x{301}"));
ok($objZh->eq("n\x{301}", "n\x{30C}"));
ok($objZh->eq("n\x{30C}", "n\x{300}"));
ok($objZh->eq("n\x{300}", "n"));
ok($objZh->eq("o\x{304}", "o\x{301}"));
ok($objZh->eq("o\x{301}", "o\x{30C}"));
ok($objZh->eq("o\x{30C}", "o\x{300}"));
ok($objZh->eq("o\x{300}", "o"));
ok($objZh->eq("u\x{304}", "u\x{301}"));
ok($objZh->eq("u\x{301}", "u\x{30C}"));
ok($objZh->eq("u\x{30C}", "u\x{300}"));
ok($objZh->eq("u\x{300}", "u"));
ok($objZh->eq("u\x{308}\x{304}", "u\x{308}\x{301}"));
ok($objZh->eq("u\x{308}\x{301}", "u\x{308}\x{30C}"));
ok($objZh->eq("u\x{308}\x{30C}", "u\x{308}\x{300}"));
ok($objZh->eq("u\x{308}\x{300}", "u\x{308}"));

# 38

$objZh->change(level => 2);

ok($objZh->lt("a\x{304}", "a\x{301}"));
ok($objZh->lt("a\x{301}", "a\x{30C}"));
ok($objZh->lt("a\x{30C}", "a\x{300}"));
ok($objZh->lt("a\x{300}", "a"));
ok($objZh->lt("e\x{304}", "e\x{301}"));
ok($objZh->lt("e\x{301}", "e\x{30C}"));
ok($objZh->lt("e\x{30C}", "e\x{300}"));
ok($objZh->lt("e\x{300}", "e"));
ok($objZh->lt("e\x{302}\x{304}", "e\x{302}\x{301}"));
ok($objZh->lt("e\x{302}\x{301}", "e\x{302}\x{30C}"));
ok($objZh->lt("e\x{302}\x{30C}", "e\x{302}\x{300}"));
ok($objZh->lt("e\x{302}\x{300}", "e\x{302}"));
ok($objZh->lt("i\x{304}", "i\x{301}"));
ok($objZh->lt("i\x{301}", "i\x{30C}"));
ok($objZh->lt("i\x{30C}", "i\x{300}"));
ok($objZh->lt("i\x{300}", "i"));
ok($objZh->lt("m\x{304}", "m\x{301}"));
ok($objZh->lt("m\x{301}", "m\x{30C}"));
ok($objZh->lt("m\x{30C}", "m\x{300}"));
ok($objZh->lt("m\x{300}", "m"));
ok($objZh->lt("n\x{304}", "n\x{301}"));
ok($objZh->lt("n\x{301}", "n\x{30C}"));
ok($objZh->lt("n\x{30C}", "n\x{300}"));
ok($objZh->lt("n\x{300}", "n"));
ok($objZh->lt("o\x{304}", "o\x{301}"));
ok($objZh->lt("o\x{301}", "o\x{30C}"));
ok($objZh->lt("o\x{30C}", "o\x{300}"));
ok($objZh->lt("o\x{300}", "o"));
ok($objZh->lt("u\x{304}", "u\x{301}"));
ok($objZh->lt("u\x{301}", "u\x{30C}"));
ok($objZh->lt("u\x{30C}", "u\x{300}"));
ok($objZh->lt("u\x{300}", "u"));
ok($objZh->lt("u\x{308}\x{304}", "u\x{308}\x{301}"));
ok($objZh->lt("u\x{308}\x{301}", "u\x{308}\x{30C}"));
ok($objZh->lt("u\x{308}\x{30C}", "u\x{308}\x{300}"));
ok($objZh->lt("u\x{308}\x{300}", "u\x{308}"));

# 74

ok($objZh->eq("a\x{304}", "A\x{304}"));
ok($objZh->eq("a\x{301}", "A\x{301}"));
ok($objZh->eq("a\x{30C}", "A\x{30C}"));
ok($objZh->eq("a\x{300}", "A\x{300}"));
ok($objZh->eq("e\x{304}", "E\x{304}"));
ok($objZh->eq("e\x{301}", "E\x{301}"));
ok($objZh->eq("e\x{30C}", "E\x{30C}"));
ok($objZh->eq("e\x{300}", "E\x{300}"));
ok($objZh->eq("e\x{302}\x{304}", "E\x{302}\x{304}"));
ok($objZh->eq("e\x{302}\x{301}", "E\x{302}\x{301}"));
ok($objZh->eq("e\x{302}\x{30C}", "E\x{302}\x{30C}"));
ok($objZh->eq("e\x{302}\x{300}", "E\x{302}\x{300}"));
ok($objZh->eq("e\x{302}", "E\x{302}"));
ok($objZh->eq("i\x{304}", "I\x{304}"));
ok($objZh->eq("i\x{301}", "I\x{301}"));
ok($objZh->eq("i\x{30C}", "I\x{30C}"));
ok($objZh->eq("i\x{300}", "I\x{300}"));
ok($objZh->eq("m\x{304}", "M\x{304}"));
ok($objZh->eq("m\x{301}", "M\x{301}"));
ok($objZh->eq("m\x{30C}", "M\x{30C}"));
ok($objZh->eq("m\x{300}", "M\x{300}"));
ok($objZh->eq("n\x{304}", "N\x{304}"));
ok($objZh->eq("n\x{301}", "N\x{301}"));
ok($objZh->eq("n\x{30C}", "N\x{30C}"));
ok($objZh->eq("n\x{300}", "N\x{300}"));
ok($objZh->eq("o\x{304}", "O\x{304}"));
ok($objZh->eq("o\x{301}", "O\x{301}"));
ok($objZh->eq("o\x{30C}", "O\x{30C}"));
ok($objZh->eq("o\x{300}", "O\x{300}"));
ok($objZh->eq("u\x{304}", "U\x{304}"));
ok($objZh->eq("u\x{301}", "U\x{301}"));
ok($objZh->eq("u\x{30C}", "U\x{30C}"));
ok($objZh->eq("u\x{300}", "U\x{300}"));
ok($objZh->eq("u\x{308}\x{304}", "U\x{308}\x{304}"));
ok($objZh->eq("u\x{308}\x{301}", "U\x{308}\x{301}"));
ok($objZh->eq("u\x{308}\x{30C}", "U\x{308}\x{30C}"));
ok($objZh->eq("u\x{308}\x{300}", "U\x{308}\x{300}"));
ok($objZh->eq("u\x{308}", "U\x{308}"));

# 112

$objZh->change(level => 3);

ok($objZh->lt("a\x{304}", "A\x{304}"));
ok($objZh->lt("a\x{301}", "A\x{301}"));
ok($objZh->lt("a\x{30C}", "A\x{30C}"));
ok($objZh->lt("a\x{300}", "A\x{300}"));
ok($objZh->lt("e\x{304}", "E\x{304}"));
ok($objZh->lt("e\x{301}", "E\x{301}"));
ok($objZh->lt("e\x{30C}", "E\x{30C}"));
ok($objZh->lt("e\x{300}", "E\x{300}"));
ok($objZh->lt("e\x{302}\x{304}", "E\x{302}\x{304}"));
ok($objZh->lt("e\x{302}\x{301}", "E\x{302}\x{301}"));
ok($objZh->lt("e\x{302}\x{30C}", "E\x{302}\x{30C}"));
ok($objZh->lt("e\x{302}\x{300}", "E\x{302}\x{300}"));
ok($objZh->lt("e\x{302}", "E\x{302}"));
ok($objZh->lt("i\x{304}", "I\x{304}"));
ok($objZh->lt("i\x{301}", "I\x{301}"));
ok($objZh->lt("i\x{30C}", "I\x{30C}"));
ok($objZh->lt("i\x{300}", "I\x{300}"));
ok($objZh->lt("m\x{304}", "M\x{304}"));
ok($objZh->lt("m\x{301}", "M\x{301}"));
ok($objZh->lt("m\x{30C}", "M\x{30C}"));
ok($objZh->lt("m\x{300}", "M\x{300}"));
ok($objZh->lt("n\x{304}", "N\x{304}"));
ok($objZh->lt("n\x{301}", "N\x{301}"));
ok($objZh->lt("n\x{30C}", "N\x{30C}"));
ok($objZh->lt("n\x{300}", "N\x{300}"));
ok($objZh->lt("o\x{304}", "O\x{304}"));
ok($objZh->lt("o\x{301}", "O\x{301}"));
ok($objZh->lt("o\x{30C}", "O\x{30C}"));
ok($objZh->lt("o\x{300}", "O\x{300}"));
ok($objZh->lt("u\x{304}", "U\x{304}"));
ok($objZh->lt("u\x{301}", "U\x{301}"));
ok($objZh->lt("u\x{30C}", "U\x{30C}"));
ok($objZh->lt("u\x{300}", "U\x{300}"));
ok($objZh->lt("u\x{308}\x{304}", "U\x{308}\x{304}"));
ok($objZh->lt("u\x{308}\x{301}", "U\x{308}\x{301}"));
ok($objZh->lt("u\x{308}\x{30C}", "U\x{308}\x{30C}"));
ok($objZh->lt("u\x{308}\x{300}", "U\x{308}\x{300}"));
ok($objZh->lt("u\x{308}", "U\x{308}"));

# 150

ok($objZh->eq("a\x{304}", "\x{101}"));
ok($objZh->eq("A\x{304}", "\x{100}"));
ok($objZh->eq("a\x{301}", _pack_U(0xE1)));
ok($objZh->eq("a\x{301}", "a\x{341}"));
ok($objZh->eq("A\x{301}", _pack_U(0xC1)));
ok($objZh->eq("A\x{301}", "A\x{341}"));
ok($objZh->eq("a\x{30C}", "\x{1CE}"));
ok($objZh->eq("A\x{30C}", "\x{1CD}"));
ok($objZh->eq("a\x{300}", _pack_U(0xE0)));
ok($objZh->eq("a\x{300}", "a\x{340}"));
ok($objZh->eq("A\x{300}", _pack_U(0xC0)));
ok($objZh->eq("A\x{300}", "A\x{340}"));
ok($objZh->eq("e\x{304}", "\x{113}"));
ok($objZh->eq("E\x{304}", "\x{112}"));
ok($objZh->eq("e\x{301}", _pack_U(0xE9)));
ok($objZh->eq("e\x{301}", "e\x{341}"));
ok($objZh->eq("E\x{301}", _pack_U(0xC9)));
ok($objZh->eq("E\x{301}", "E\x{341}"));
ok($objZh->eq("e\x{30C}", "\x{11B}"));
ok($objZh->eq("E\x{30C}", "\x{11A}"));
ok($objZh->eq("e\x{300}", _pack_U(0xE8)));
ok($objZh->eq("e\x{300}", "e\x{340}"));
ok($objZh->eq("E\x{300}", _pack_U(0xC8)));
ok($objZh->eq("E\x{300}", "E\x{340}"));
ok($objZh->eq("e\x{302}\x{304}", _pack_U(0xEA, 0x304)));
ok($objZh->eq("E\x{302}\x{304}", _pack_U(0xCA, 0x304)));
ok($objZh->eq("e\x{302}\x{301}", "\x{1EBF}"));
ok($objZh->eq("e\x{302}\x{301}", "e\x{302}\x{341}"));
ok($objZh->eq("E\x{302}\x{301}", "\x{1EBE}"));
ok($objZh->eq("E\x{302}\x{301}", "E\x{302}\x{341}"));
ok($objZh->eq("e\x{302}\x{301}", _pack_U(0xEA, 0x301)));
ok($objZh->eq("e\x{302}\x{301}", _pack_U(0xEA, 0x341)));
ok($objZh->eq("E\x{302}\x{301}", _pack_U(0xCA, 0x301)));
ok($objZh->eq("E\x{302}\x{301}", _pack_U(0xCA, 0x341)));
ok($objZh->eq("e\x{302}\x{30C}", _pack_U(0xEA, 0x30C)));
ok($objZh->eq("E\x{302}\x{30C}", _pack_U(0xCA, 0x30C)));
ok($objZh->eq("e\x{302}\x{300}", "\x{1EC1}"));
ok($objZh->eq("e\x{302}\x{300}", "e\x{302}\x{340}"));
ok($objZh->eq("E\x{302}\x{300}", "\x{1EC0}"));
ok($objZh->eq("E\x{302}\x{300}", "E\x{302}\x{340}"));
ok($objZh->eq("e\x{302}\x{300}", _pack_U(0xEA, 0x300)));
ok($objZh->eq("e\x{302}\x{300}", _pack_U(0xEA, 0x340)));
ok($objZh->eq("E\x{302}\x{300}", _pack_U(0xCA, 0x300)));
ok($objZh->eq("E\x{302}\x{300}", _pack_U(0xCA, 0x340)));
ok($objZh->eq("e\x{302}", _pack_U(0xEA)));
ok($objZh->eq("E\x{302}", _pack_U(0xCA)));
ok($objZh->eq("i\x{304}", "\x{12B}"));
ok($objZh->eq("I\x{304}", "\x{12A}"));
ok($objZh->eq("i\x{301}", _pack_U(0xED)));
ok($objZh->eq("i\x{301}", "i\x{341}"));
ok($objZh->eq("I\x{301}", _pack_U(0xCD)));
ok($objZh->eq("I\x{301}", "I\x{341}"));
ok($objZh->eq("i\x{30C}", "\x{1D0}"));
ok($objZh->eq("I\x{30C}", "\x{1CF}"));
ok($objZh->eq("i\x{300}", _pack_U(0xEC)));
ok($objZh->eq("i\x{300}", "i\x{340}"));
ok($objZh->eq("I\x{300}", _pack_U(0xCC)));
ok($objZh->eq("I\x{300}", "I\x{340}"));
ok($objZh->eq("m\x{301}", "\x{1E3F}"));
ok($objZh->eq("m\x{301}", "m\x{341}"));
ok($objZh->eq("M\x{301}", "\x{1E3E}"));
ok($objZh->eq("M\x{301}", "M\x{341}"));
ok($objZh->eq("m\x{300}", "m\x{340}"));
ok($objZh->eq("M\x{300}", "M\x{340}"));
ok($objZh->eq("n\x{301}", "\x{144}"));
ok($objZh->eq("n\x{301}", "n\x{341}"));
ok($objZh->eq("N\x{301}", "\x{143}"));
ok($objZh->eq("N\x{301}", "N\x{341}"));
ok($objZh->eq("n\x{30C}", "\x{148}"));
ok($objZh->eq("N\x{30C}", "\x{147}"));
ok($objZh->eq("n\x{300}", "\x{1F9}"));
ok($objZh->eq("n\x{300}", "n\x{340}"));
ok($objZh->eq("N\x{300}", "\x{1F8}"));
ok($objZh->eq("N\x{300}", "N\x{340}"));
ok($objZh->eq("o\x{304}", "\x{14D}"));
ok($objZh->eq("O\x{304}", "\x{14C}"));
ok($objZh->eq("o\x{301}", _pack_U(0xF3)));
ok($objZh->eq("o\x{301}", "o\x{341}"));
ok($objZh->eq("O\x{301}", _pack_U(0xD3)));
ok($objZh->eq("O\x{301}", "O\x{341}"));
ok($objZh->eq("o\x{30C}", "\x{1D2}"));
ok($objZh->eq("O\x{30C}", "\x{1D1}"));
ok($objZh->eq("o\x{300}", _pack_U(0xF2)));
ok($objZh->eq("o\x{300}", "o\x{340}"));
ok($objZh->eq("O\x{300}", _pack_U(0xD2)));
ok($objZh->eq("O\x{300}", "O\x{340}"));
ok($objZh->eq("u\x{304}", "\x{16B}"));
ok($objZh->eq("U\x{304}", "\x{16A}"));
ok($objZh->eq("u\x{301}", _pack_U(0xFA)));
ok($objZh->eq("u\x{301}", "u\x{341}"));
ok($objZh->eq("U\x{301}", _pack_U(0xDA)));
ok($objZh->eq("U\x{301}", "U\x{341}"));
ok($objZh->eq("u\x{30C}", "\x{1D4}"));
ok($objZh->eq("U\x{30C}", "\x{1D3}"));
ok($objZh->eq("u\x{300}", _pack_U(0xF9)));
ok($objZh->eq("u\x{300}", "u\x{340}"));
ok($objZh->eq("U\x{300}", _pack_U(0xD9)));
ok($objZh->eq("U\x{300}", "U\x{340}"));
ok($objZh->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objZh->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objZh->eq("u\x{308}\x{304}", _pack_U(0xFC, 0x304)));
ok($objZh->eq("U\x{308}\x{304}", _pack_U(0xDC, 0x304)));
ok($objZh->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objZh->eq("u\x{308}\x{301}", "u\x{308}\x{341}"));
ok($objZh->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objZh->eq("U\x{308}\x{301}", "U\x{308}\x{341}"));
ok($objZh->eq("u\x{308}\x{301}", _pack_U(0xFC, 0x301)));
ok($objZh->eq("u\x{308}\x{301}", _pack_U(0xFC, 0x341)));
ok($objZh->eq("U\x{308}\x{301}", _pack_U(0xDC, 0x301)));
ok($objZh->eq("U\x{308}\x{301}", _pack_U(0xDC, 0x341)));
ok($objZh->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objZh->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objZh->eq("u\x{308}\x{30C}", _pack_U(0xFC, 0x30C)));
ok($objZh->eq("U\x{308}\x{30C}", _pack_U(0xDC, 0x30C)));
ok($objZh->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objZh->eq("u\x{308}\x{300}", "u\x{308}\x{340}"));
ok($objZh->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objZh->eq("U\x{308}\x{300}", "U\x{308}\x{340}"));
ok($objZh->eq("u\x{308}\x{300}", _pack_U(0xFC, 0x300)));
ok($objZh->eq("u\x{308}\x{300}", _pack_U(0xFC, 0x340)));
ok($objZh->eq("U\x{308}\x{300}", _pack_U(0xDC, 0x300)));
ok($objZh->eq("U\x{308}\x{300}", _pack_U(0xDC, 0x340)));
ok($objZh->eq("u\x{308}", _pack_U(0xFC)));
ok($objZh->eq("U\x{308}", _pack_U(0xDC)));

# 274

ok($objZh->eq("e\x{302}\x{303}", "\x{1EC5}"));
ok($objZh->eq("E\x{302}\x{303}", "\x{1EC4}"));
ok($objZh->eq("e\x{302}\x{309}", "\x{1EC3}"));
ok($objZh->eq("E\x{302}\x{309}", "\x{1EC2}"));
ok($objZh->eq("e\x{302}\x{323}", "\x{1EC7}"));
ok($objZh->eq("E\x{302}\x{323}", "\x{1EC6}"));

# 280

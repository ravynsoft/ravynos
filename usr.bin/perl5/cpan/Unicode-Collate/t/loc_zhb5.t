
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..302\n"; }
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

my $objZhB = Unicode::Collate::Locale->
    new(locale => 'ZH__big5han', normalization => undef);

ok($objZhB->getlocale, 'zh__big5han');

$objZhB->change(level => 1);

ok($objZhB->eq("a\x{304}", "a\x{301}"));
ok($objZhB->eq("a\x{301}", "a\x{30C}"));
ok($objZhB->eq("a\x{30C}", "a\x{300}"));
ok($objZhB->eq("a\x{300}", "a"));
ok($objZhB->eq("e\x{304}", "e\x{301}"));
ok($objZhB->eq("e\x{301}", "e\x{30C}"));
ok($objZhB->eq("e\x{30C}", "e\x{300}"));
ok($objZhB->eq("e\x{300}", "e"));
ok($objZhB->eq("e\x{302}\x{304}", "e\x{302}\x{301}"));
ok($objZhB->eq("e\x{302}\x{301}", "e\x{302}\x{30C}"));
ok($objZhB->eq("e\x{302}\x{30C}", "e\x{302}\x{300}"));
ok($objZhB->eq("e\x{302}\x{300}", "e\x{302}"));
ok($objZhB->eq("i\x{304}", "i\x{301}"));
ok($objZhB->eq("i\x{301}", "i\x{30C}"));
ok($objZhB->eq("i\x{30C}", "i\x{300}"));
ok($objZhB->eq("i\x{300}", "i"));
ok($objZhB->eq("m\x{304}", "m\x{301}"));
ok($objZhB->eq("m\x{301}", "m\x{30C}"));
ok($objZhB->eq("m\x{30C}", "m\x{300}"));
ok($objZhB->eq("m\x{300}", "m"));
ok($objZhB->eq("n\x{304}", "n\x{301}"));
ok($objZhB->eq("n\x{301}", "n\x{30C}"));
ok($objZhB->eq("n\x{30C}", "n\x{300}"));
ok($objZhB->eq("n\x{300}", "n"));
ok($objZhB->eq("o\x{304}", "o\x{301}"));
ok($objZhB->eq("o\x{301}", "o\x{30C}"));
ok($objZhB->eq("o\x{30C}", "o\x{300}"));
ok($objZhB->eq("o\x{300}", "o"));
ok($objZhB->eq("u\x{304}", "u\x{301}"));
ok($objZhB->eq("u\x{301}", "u\x{30C}"));
ok($objZhB->eq("u\x{30C}", "u\x{300}"));
ok($objZhB->eq("u\x{300}", "u"));
ok($objZhB->eq("u\x{308}\x{304}", "u\x{308}\x{301}"));
ok($objZhB->eq("u\x{308}\x{301}", "u\x{308}\x{30C}"));
ok($objZhB->eq("u\x{308}\x{30C}", "u\x{308}\x{300}"));
ok($objZhB->eq("u\x{308}\x{300}", "u\x{308}"));

# 38

$objZhB->change(level => 2);

ok($objZhB->lt("a\x{304}", "a\x{301}"));
ok($objZhB->lt("a\x{301}", "a\x{30C}"));
ok($objZhB->lt("a\x{30C}", "a\x{300}"));
ok($objZhB->lt("a\x{300}", "a"));
ok($objZhB->lt("e\x{304}", "e\x{301}"));
ok($objZhB->lt("e\x{301}", "e\x{30C}"));
ok($objZhB->lt("e\x{30C}", "e\x{300}"));
ok($objZhB->lt("e\x{300}", "e"));
ok($objZhB->lt("e\x{302}\x{304}", "e\x{302}\x{301}"));
ok($objZhB->lt("e\x{302}\x{301}", "e\x{302}\x{30C}"));
ok($objZhB->lt("e\x{302}\x{30C}", "e\x{302}\x{300}"));
ok($objZhB->lt("e\x{302}\x{300}", "e\x{302}"));
ok($objZhB->lt("i\x{304}", "i\x{301}"));
ok($objZhB->lt("i\x{301}", "i\x{30C}"));
ok($objZhB->lt("i\x{30C}", "i\x{300}"));
ok($objZhB->lt("i\x{300}", "i"));
ok($objZhB->lt("m\x{304}", "m\x{301}"));
ok($objZhB->lt("m\x{301}", "m\x{30C}"));
ok($objZhB->lt("m\x{30C}", "m\x{300}"));
ok($objZhB->lt("m\x{300}", "m"));
ok($objZhB->lt("n\x{304}", "n\x{301}"));
ok($objZhB->lt("n\x{301}", "n\x{30C}"));
ok($objZhB->lt("n\x{30C}", "n\x{300}"));
ok($objZhB->lt("n\x{300}", "n"));
ok($objZhB->lt("o\x{304}", "o\x{301}"));
ok($objZhB->lt("o\x{301}", "o\x{30C}"));
ok($objZhB->lt("o\x{30C}", "o\x{300}"));
ok($objZhB->lt("o\x{300}", "o"));
ok($objZhB->lt("u\x{304}", "u\x{301}"));
ok($objZhB->lt("u\x{301}", "u\x{30C}"));
ok($objZhB->lt("u\x{30C}", "u\x{300}"));
ok($objZhB->lt("u\x{300}", "u"));
ok($objZhB->lt("u\x{308}\x{304}", "u\x{308}\x{301}"));
ok($objZhB->lt("u\x{308}\x{301}", "u\x{308}\x{30C}"));
ok($objZhB->lt("u\x{308}\x{30C}", "u\x{308}\x{300}"));
ok($objZhB->lt("u\x{308}\x{300}", "u\x{308}"));

# 74

ok($objZhB->eq("a\x{304}", "A\x{304}"));
ok($objZhB->eq("a\x{301}", "A\x{301}"));
ok($objZhB->eq("a\x{30C}", "A\x{30C}"));
ok($objZhB->eq("a\x{300}", "A\x{300}"));
ok($objZhB->eq("e\x{304}", "E\x{304}"));
ok($objZhB->eq("e\x{301}", "E\x{301}"));
ok($objZhB->eq("e\x{30C}", "E\x{30C}"));
ok($objZhB->eq("e\x{300}", "E\x{300}"));
ok($objZhB->eq("e\x{302}\x{304}", "E\x{302}\x{304}"));
ok($objZhB->eq("e\x{302}\x{301}", "E\x{302}\x{301}"));
ok($objZhB->eq("e\x{302}\x{30C}", "E\x{302}\x{30C}"));
ok($objZhB->eq("e\x{302}\x{300}", "E\x{302}\x{300}"));
ok($objZhB->eq("e\x{302}", "E\x{302}"));
ok($objZhB->eq("i\x{304}", "I\x{304}"));
ok($objZhB->eq("i\x{301}", "I\x{301}"));
ok($objZhB->eq("i\x{30C}", "I\x{30C}"));
ok($objZhB->eq("i\x{300}", "I\x{300}"));
ok($objZhB->eq("m\x{304}", "M\x{304}"));
ok($objZhB->eq("m\x{301}", "M\x{301}"));
ok($objZhB->eq("m\x{30C}", "M\x{30C}"));
ok($objZhB->eq("m\x{300}", "M\x{300}"));
ok($objZhB->eq("n\x{304}", "N\x{304}"));
ok($objZhB->eq("n\x{301}", "N\x{301}"));
ok($objZhB->eq("n\x{30C}", "N\x{30C}"));
ok($objZhB->eq("n\x{300}", "N\x{300}"));
ok($objZhB->eq("o\x{304}", "O\x{304}"));
ok($objZhB->eq("o\x{301}", "O\x{301}"));
ok($objZhB->eq("o\x{30C}", "O\x{30C}"));
ok($objZhB->eq("o\x{300}", "O\x{300}"));
ok($objZhB->eq("u\x{304}", "U\x{304}"));
ok($objZhB->eq("u\x{301}", "U\x{301}"));
ok($objZhB->eq("u\x{30C}", "U\x{30C}"));
ok($objZhB->eq("u\x{300}", "U\x{300}"));
ok($objZhB->eq("u\x{308}\x{304}", "U\x{308}\x{304}"));
ok($objZhB->eq("u\x{308}\x{301}", "U\x{308}\x{301}"));
ok($objZhB->eq("u\x{308}\x{30C}", "U\x{308}\x{30C}"));
ok($objZhB->eq("u\x{308}\x{300}", "U\x{308}\x{300}"));
ok($objZhB->eq("u\x{308}", "U\x{308}"));

# 112

$objZhB->change(level => 3);

ok($objZhB->lt("a\x{304}", "A\x{304}"));
ok($objZhB->lt("a\x{301}", "A\x{301}"));
ok($objZhB->lt("a\x{30C}", "A\x{30C}"));
ok($objZhB->lt("a\x{300}", "A\x{300}"));
ok($objZhB->lt("e\x{304}", "E\x{304}"));
ok($objZhB->lt("e\x{301}", "E\x{301}"));
ok($objZhB->lt("e\x{30C}", "E\x{30C}"));
ok($objZhB->lt("e\x{300}", "E\x{300}"));
ok($objZhB->lt("e\x{302}\x{304}", "E\x{302}\x{304}"));
ok($objZhB->lt("e\x{302}\x{301}", "E\x{302}\x{301}"));
ok($objZhB->lt("e\x{302}\x{30C}", "E\x{302}\x{30C}"));
ok($objZhB->lt("e\x{302}\x{300}", "E\x{302}\x{300}"));
ok($objZhB->lt("e\x{302}", "E\x{302}"));
ok($objZhB->lt("i\x{304}", "I\x{304}"));
ok($objZhB->lt("i\x{301}", "I\x{301}"));
ok($objZhB->lt("i\x{30C}", "I\x{30C}"));
ok($objZhB->lt("i\x{300}", "I\x{300}"));
ok($objZhB->lt("m\x{304}", "M\x{304}"));
ok($objZhB->lt("m\x{301}", "M\x{301}"));
ok($objZhB->lt("m\x{30C}", "M\x{30C}"));
ok($objZhB->lt("m\x{300}", "M\x{300}"));
ok($objZhB->lt("n\x{304}", "N\x{304}"));
ok($objZhB->lt("n\x{301}", "N\x{301}"));
ok($objZhB->lt("n\x{30C}", "N\x{30C}"));
ok($objZhB->lt("n\x{300}", "N\x{300}"));
ok($objZhB->lt("o\x{304}", "O\x{304}"));
ok($objZhB->lt("o\x{301}", "O\x{301}"));
ok($objZhB->lt("o\x{30C}", "O\x{30C}"));
ok($objZhB->lt("o\x{300}", "O\x{300}"));
ok($objZhB->lt("u\x{304}", "U\x{304}"));
ok($objZhB->lt("u\x{301}", "U\x{301}"));
ok($objZhB->lt("u\x{30C}", "U\x{30C}"));
ok($objZhB->lt("u\x{300}", "U\x{300}"));
ok($objZhB->lt("u\x{308}\x{304}", "U\x{308}\x{304}"));
ok($objZhB->lt("u\x{308}\x{301}", "U\x{308}\x{301}"));
ok($objZhB->lt("u\x{308}\x{30C}", "U\x{308}\x{30C}"));
ok($objZhB->lt("u\x{308}\x{300}", "U\x{308}\x{300}"));
ok($objZhB->lt("u\x{308}", "U\x{308}"));

# 150

ok($objZhB->eq("a\x{304}", "\x{101}"));
ok($objZhB->eq("A\x{304}", "\x{100}"));
ok($objZhB->eq("a\x{301}", _pack_U(0xE1)));
ok($objZhB->eq("a\x{301}", "a\x{341}"));
ok($objZhB->eq("A\x{301}", _pack_U(0xC1)));
ok($objZhB->eq("A\x{301}", "A\x{341}"));
ok($objZhB->eq("a\x{30C}", "\x{1CE}"));
ok($objZhB->eq("A\x{30C}", "\x{1CD}"));
ok($objZhB->eq("a\x{300}", _pack_U(0xE0)));
ok($objZhB->eq("a\x{300}", "a\x{340}"));
ok($objZhB->eq("A\x{300}", _pack_U(0xC0)));
ok($objZhB->eq("A\x{300}", "A\x{340}"));
ok($objZhB->eq("e\x{304}", "\x{113}"));
ok($objZhB->eq("E\x{304}", "\x{112}"));
ok($objZhB->eq("e\x{301}", _pack_U(0xE9)));
ok($objZhB->eq("e\x{301}", "e\x{341}"));
ok($objZhB->eq("E\x{301}", _pack_U(0xC9)));
ok($objZhB->eq("E\x{301}", "E\x{341}"));
ok($objZhB->eq("e\x{30C}", "\x{11B}"));
ok($objZhB->eq("E\x{30C}", "\x{11A}"));
ok($objZhB->eq("e\x{300}", _pack_U(0xE8)));
ok($objZhB->eq("e\x{300}", "e\x{340}"));
ok($objZhB->eq("E\x{300}", _pack_U(0xC8)));
ok($objZhB->eq("E\x{300}", "E\x{340}"));
ok($objZhB->eq("e\x{302}\x{304}", _pack_U(0xEA, 0x304)));
ok($objZhB->eq("E\x{302}\x{304}", _pack_U(0xCA, 0x304)));
ok($objZhB->eq("e\x{302}\x{301}", "\x{1EBF}"));
ok($objZhB->eq("e\x{302}\x{301}", "e\x{302}\x{341}"));
ok($objZhB->eq("E\x{302}\x{301}", "\x{1EBE}"));
ok($objZhB->eq("E\x{302}\x{301}", "E\x{302}\x{341}"));
ok($objZhB->eq("e\x{302}\x{301}", _pack_U(0xEA, 0x301)));
ok($objZhB->eq("e\x{302}\x{301}", _pack_U(0xEA, 0x341)));
ok($objZhB->eq("E\x{302}\x{301}", _pack_U(0xCA, 0x301)));
ok($objZhB->eq("E\x{302}\x{301}", _pack_U(0xCA, 0x341)));
ok($objZhB->eq("e\x{302}\x{30C}", _pack_U(0xEA, 0x30C)));
ok($objZhB->eq("E\x{302}\x{30C}", _pack_U(0xCA, 0x30C)));
ok($objZhB->eq("e\x{302}\x{300}", "\x{1EC1}"));
ok($objZhB->eq("e\x{302}\x{300}", "e\x{302}\x{340}"));
ok($objZhB->eq("E\x{302}\x{300}", "\x{1EC0}"));
ok($objZhB->eq("E\x{302}\x{300}", "E\x{302}\x{340}"));
ok($objZhB->eq("e\x{302}\x{300}", _pack_U(0xEA, 0x300)));
ok($objZhB->eq("e\x{302}\x{300}", _pack_U(0xEA, 0x340)));
ok($objZhB->eq("E\x{302}\x{300}", _pack_U(0xCA, 0x300)));
ok($objZhB->eq("E\x{302}\x{300}", _pack_U(0xCA, 0x340)));
ok($objZhB->eq("e\x{302}", _pack_U(0xEA)));
ok($objZhB->eq("E\x{302}", _pack_U(0xCA)));
ok($objZhB->eq("i\x{304}", "\x{12B}"));
ok($objZhB->eq("I\x{304}", "\x{12A}"));
ok($objZhB->eq("i\x{301}", _pack_U(0xED)));
ok($objZhB->eq("i\x{301}", "i\x{341}"));
ok($objZhB->eq("I\x{301}", _pack_U(0xCD)));
ok($objZhB->eq("I\x{301}", "I\x{341}"));
ok($objZhB->eq("i\x{30C}", "\x{1D0}"));
ok($objZhB->eq("I\x{30C}", "\x{1CF}"));
ok($objZhB->eq("i\x{300}", _pack_U(0xEC)));
ok($objZhB->eq("i\x{300}", "i\x{340}"));
ok($objZhB->eq("I\x{300}", _pack_U(0xCC)));
ok($objZhB->eq("I\x{300}", "I\x{340}"));
ok($objZhB->eq("m\x{301}", "\x{1E3F}"));
ok($objZhB->eq("m\x{301}", "m\x{341}"));
ok($objZhB->eq("M\x{301}", "\x{1E3E}"));
ok($objZhB->eq("M\x{301}", "M\x{341}"));
ok($objZhB->eq("m\x{300}", "m\x{340}"));
ok($objZhB->eq("M\x{300}", "M\x{340}"));
ok($objZhB->eq("n\x{301}", "\x{144}"));
ok($objZhB->eq("n\x{301}", "n\x{341}"));
ok($objZhB->eq("N\x{301}", "\x{143}"));
ok($objZhB->eq("N\x{301}", "N\x{341}"));
ok($objZhB->eq("n\x{30C}", "\x{148}"));
ok($objZhB->eq("N\x{30C}", "\x{147}"));
ok($objZhB->eq("n\x{300}", "\x{1F9}"));
ok($objZhB->eq("n\x{300}", "n\x{340}"));
ok($objZhB->eq("N\x{300}", "\x{1F8}"));
ok($objZhB->eq("N\x{300}", "N\x{340}"));
ok($objZhB->eq("o\x{304}", "\x{14D}"));
ok($objZhB->eq("O\x{304}", "\x{14C}"));
ok($objZhB->eq("o\x{301}", _pack_U(0xF3)));
ok($objZhB->eq("o\x{301}", "o\x{341}"));
ok($objZhB->eq("O\x{301}", _pack_U(0xD3)));
ok($objZhB->eq("O\x{301}", "O\x{341}"));
ok($objZhB->eq("o\x{30C}", "\x{1D2}"));
ok($objZhB->eq("O\x{30C}", "\x{1D1}"));
ok($objZhB->eq("o\x{300}", _pack_U(0xF2)));
ok($objZhB->eq("o\x{300}", "o\x{340}"));
ok($objZhB->eq("O\x{300}", _pack_U(0xD2)));
ok($objZhB->eq("O\x{300}", "O\x{340}"));
ok($objZhB->eq("u\x{304}", "\x{16B}"));
ok($objZhB->eq("U\x{304}", "\x{16A}"));
ok($objZhB->eq("u\x{301}", _pack_U(0xFA)));
ok($objZhB->eq("u\x{301}", "u\x{341}"));
ok($objZhB->eq("U\x{301}", _pack_U(0xDA)));
ok($objZhB->eq("U\x{301}", "U\x{341}"));
ok($objZhB->eq("u\x{30C}", "\x{1D4}"));
ok($objZhB->eq("U\x{30C}", "\x{1D3}"));
ok($objZhB->eq("u\x{300}", _pack_U(0xF9)));
ok($objZhB->eq("u\x{300}", "u\x{340}"));
ok($objZhB->eq("U\x{300}", _pack_U(0xD9)));
ok($objZhB->eq("U\x{300}", "U\x{340}"));
ok($objZhB->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objZhB->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objZhB->eq("u\x{308}\x{304}", _pack_U(0xFC, 0x304)));
ok($objZhB->eq("U\x{308}\x{304}", _pack_U(0xDC, 0x304)));
ok($objZhB->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objZhB->eq("u\x{308}\x{301}", "u\x{308}\x{341}"));
ok($objZhB->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objZhB->eq("U\x{308}\x{301}", "U\x{308}\x{341}"));
ok($objZhB->eq("u\x{308}\x{301}", _pack_U(0xFC, 0x301)));
ok($objZhB->eq("u\x{308}\x{301}", _pack_U(0xFC, 0x341)));
ok($objZhB->eq("U\x{308}\x{301}", _pack_U(0xDC, 0x301)));
ok($objZhB->eq("U\x{308}\x{301}", _pack_U(0xDC, 0x341)));
ok($objZhB->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objZhB->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objZhB->eq("u\x{308}\x{30C}", _pack_U(0xFC, 0x30C)));
ok($objZhB->eq("U\x{308}\x{30C}", _pack_U(0xDC, 0x30C)));
ok($objZhB->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objZhB->eq("u\x{308}\x{300}", "u\x{308}\x{340}"));
ok($objZhB->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objZhB->eq("U\x{308}\x{300}", "U\x{308}\x{340}"));
ok($objZhB->eq("u\x{308}\x{300}", _pack_U(0xFC, 0x300)));
ok($objZhB->eq("u\x{308}\x{300}", _pack_U(0xFC, 0x340)));
ok($objZhB->eq("U\x{308}\x{300}", _pack_U(0xDC, 0x300)));
ok($objZhB->eq("U\x{308}\x{300}", _pack_U(0xDC, 0x340)));
ok($objZhB->eq("u\x{308}", _pack_U(0xFC)));
ok($objZhB->eq("U\x{308}", _pack_U(0xDC)));

# 274

ok($objZhB->eq("e\x{302}\x{303}", "\x{1EC5}"));
ok($objZhB->eq("E\x{302}\x{303}", "\x{1EC4}"));
ok($objZhB->eq("e\x{302}\x{309}", "\x{1EC3}"));
ok($objZhB->eq("E\x{302}\x{309}", "\x{1EC2}"));
ok($objZhB->eq("e\x{302}\x{323}", "\x{1EC7}"));
ok($objZhB->eq("E\x{302}\x{323}", "\x{1EC6}"));

# 280

$objZhB->change(level => 1);

ok($objZhB->lt("\x{A000}", "\x{5159}"));
ok($objZhB->lt("\x{5159}", "\x{515B}"));
ok($objZhB->lt("\x{515B}", "\x{515E}"));
ok($objZhB->lt("\x{515E}", "\x{515D}"));
ok($objZhB->lt("\x{515D}", "\x{5161}"));
ok($objZhB->lt("\x{5161}", "\x{5163}"));
ok($objZhB->lt("\x{5163}", "\x{55E7}"));
ok($objZhB->lt("\x{55E7}", "\x{74E9}"));
ok($objZhB->lt("\x{74E9}", "\x{7CCE}"));
ok($objZhB->lt("\x{7CCE}", "\x{4E00}"));
ok($objZhB->lt("\x{4E00}", "\x{4E59}"));
ok($objZhB->lt("\x{4E59}", "\x{4E01}"));
ok($objZhB->lt("\x{4E01}", "\x{4E03}"));
ok($objZhB->lt("\x{4E03}", "\x{4E43}"));
ok($objZhB->lt("\x{4E43}", "\x{4E5D}"));
ok($objZhB->lt("\x{4E5D}", "\x{4E86}"));
ok($objZhB->lt("\x{4E86}", "\x{7069}"));
ok($objZhB->lt("\x{7069}", "\x{706A}"));
ok($objZhB->lt("\x{706A}", "\x{9EA4}"));
ok($objZhB->lt("\x{9EA4}", "\x{9F7E}"));
ok($objZhB->lt("\x{9F7E}", "\x{9F49}"));
ok($objZhB->lt("\x{9F49}", "\x{9F98}"));

# 302

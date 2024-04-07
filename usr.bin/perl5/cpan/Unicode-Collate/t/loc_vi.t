
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..424\n"; }
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

my $objVi = Unicode::Collate::Locale->
    new(locale => 'VI', normalization => undef);

ok($objVi->getlocale, 'vi');

$objVi->change(level => 1);

ok($objVi->lt("a", "a\x{306}"));
ok($objVi->lt("a\x{306}", "a\x{302}"));
ok($objVi->gt("b", "a\x{302}"));
ok($objVi->lt("d", "d\x{335}"));
ok($objVi->gt("e", "d\x{335}"));
ok($objVi->lt("e", "e\x{302}"));
ok($objVi->gt("f", "e\x{302}"));
ok($objVi->lt("o", "o\x{302}"));
ok($objVi->lt("o\x{302}", "o\x{31B}"));
ok($objVi->gt("p", "o\x{31B}"));
ok($objVi->lt("u", "u\x{31B}"));
ok($objVi->gt("v", "u\x{31B}"));

# 14

ok($objVi->eq("\x{300}", "\x{309}"));
ok($objVi->eq("\x{309}", "\x{303}"));
ok($objVi->eq("\x{303}", "\x{301}"));
ok($objVi->eq("\x{301}", "\x{323}"));
ok($objVi->eq("\x{323}", "\x{306}"));
ok($objVi->eq("\x{306}", "\x{302}"));

ok($objVi->eq("X\x{300}", "X\x{309}"));
ok($objVi->eq("X\x{309}", "X\x{303}"));
ok($objVi->eq("X\x{303}", "X\x{301}"));
ok($objVi->eq("X\x{301}", "X\x{323}"));
ok($objVi->eq("X\x{323}", "X\x{306}"));
ok($objVi->eq("X\x{306}", "X\x{302}"));

# 26

$objVi->change(level => 2);

ok($objVi->lt("\x{300}", "\x{309}"));
ok($objVi->lt("\x{309}", "\x{303}"));
ok($objVi->lt("\x{303}", "\x{301}"));
ok($objVi->lt("\x{301}", "\x{323}"));
ok($objVi->lt("\x{323}", "\x{306}"));
ok($objVi->lt("\x{306}", "\x{302}"));

ok($objVi->lt("X\x{300}", "X\x{309}"));
ok($objVi->lt("X\x{309}", "X\x{303}"));
ok($objVi->lt("X\x{303}", "X\x{301}"));
ok($objVi->lt("X\x{301}", "X\x{323}"));
ok($objVi->lt("X\x{323}", "X\x{306}"));
ok($objVi->lt("X\x{306}", "X\x{302}"));

# 38

ok($objVi->eq("a\x{306}", "A\x{306}"));
ok($objVi->eq("a\x{302}", "A\x{302}"));
ok($objVi->eq("d\x{335}", "D\x{335}"));
ok($objVi->eq("e\x{302}", "E\x{302}"));
ok($objVi->eq("o\x{302}", "O\x{302}"));
ok($objVi->eq("o\x{31B}", "O\x{31B}"));
ok($objVi->eq("u\x{31B}", "U\x{31B}"));

# 45

$objVi->change(level => 3);

ok($objVi->lt("a\x{306}", "A\x{306}"));
ok($objVi->lt("a\x{302}", "A\x{302}"));
ok($objVi->lt("d\x{335}", "D\x{335}"));
ok($objVi->lt("e\x{302}", "E\x{302}"));
ok($objVi->lt("o\x{302}", "O\x{302}"));
ok($objVi->lt("o\x{31B}", "O\x{31B}"));
ok($objVi->lt("u\x{31B}", "U\x{31B}"));

# 52

ok($objVi->eq("a\x{306}", "\x{103}"));
ok($objVi->eq("A\x{306}", "\x{102}"));
ok($objVi->eq("a\x{302}", _pack_U(0xE2)));
ok($objVi->eq("A\x{302}", _pack_U(0xC2)));
ok($objVi->eq("d\x{335}", "\x{111}"));
ok($objVi->eq("D\x{335}", "\x{110}"));
ok($objVi->eq("e\x{302}", _pack_U(0xEA)));
ok($objVi->eq("E\x{302}", _pack_U(0xCA)));
ok($objVi->eq("o\x{302}", _pack_U(0xF4)));
ok($objVi->eq("O\x{302}", _pack_U(0xD4)));
ok($objVi->eq("o\x{31B}", "\x{1A1}"));
ok($objVi->eq("O\x{31B}", "\x{1A0}"));
ok($objVi->eq("u\x{31B}", "\x{1B0}"));
ok($objVi->eq("U\x{31B}", "\x{1AF}"));

# 66

ok($objVi->eq("e\x{306}", "\x{115}"));
ok($objVi->eq("E\x{306}", "\x{114}"));
ok($objVi->eq("i\x{306}", "\x{12D}"));
ok($objVi->eq("I\x{306}", "\x{12C}"));
ok($objVi->eq("o\x{306}", "\x{14F}"));
ok($objVi->eq("O\x{306}", "\x{14E}"));
ok($objVi->eq("u\x{306}", "\x{16D}"));
ok($objVi->eq("U\x{306}", "\x{16C}"));

# 74

ok($objVi->eq("a\x{300}", _pack_U(0xE0)));
ok($objVi->eq("a\x{340}", _pack_U(0xE0)));
ok($objVi->eq("a\x{309}", "\x{1EA3}"));
ok($objVi->eq("a\x{303}", _pack_U(0xE3)));
ok($objVi->eq("a\x{301}", _pack_U(0xE1)));
ok($objVi->eq("a\x{341}", _pack_U(0xE1)));
ok($objVi->eq("a\x{323}", "\x{1EA1}"));
ok($objVi->eq("A\x{300}", _pack_U(0xC0)));
ok($objVi->eq("A\x{340}", _pack_U(0xC0)));
ok($objVi->eq("A\x{309}", "\x{1EA2}"));
ok($objVi->eq("A\x{303}", _pack_U(0xC3)));
ok($objVi->eq("A\x{301}", _pack_U(0xC1)));
ok($objVi->eq("A\x{341}", _pack_U(0xC1)));
ok($objVi->eq("A\x{323}", "\x{1EA0}"));
ok($objVi->eq("a\x{306}\x{300}", "\x{1EB1}"));
ok($objVi->eq("a\x{306}\x{340}", "\x{1EB1}"));
ok($objVi->eq("a\x{306}\x{309}", "\x{1EB3}"));
ok($objVi->eq("a\x{306}\x{303}", "\x{1EB5}"));
ok($objVi->eq("a\x{306}\x{301}", "\x{1EAF}"));
ok($objVi->eq("a\x{306}\x{341}", "\x{1EAF}"));
ok($objVi->eq("a\x{306}\x{323}", "\x{1EB7}"));
ok($objVi->eq("A\x{306}\x{300}", "\x{1EB0}"));
ok($objVi->eq("A\x{306}\x{340}", "\x{1EB0}"));
ok($objVi->eq("A\x{306}\x{309}", "\x{1EB2}"));
ok($objVi->eq("A\x{306}\x{303}", "\x{1EB4}"));
ok($objVi->eq("A\x{306}\x{301}", "\x{1EAE}"));
ok($objVi->eq("A\x{306}\x{341}", "\x{1EAE}"));
ok($objVi->eq("A\x{306}\x{323}", "\x{1EB6}"));
ok($objVi->eq("a\x{302}\x{300}", "\x{1EA7}"));
ok($objVi->eq("a\x{302}\x{340}", "\x{1EA7}"));
ok($objVi->eq("a\x{302}\x{309}", "\x{1EA9}"));
ok($objVi->eq("a\x{302}\x{303}", "\x{1EAB}"));
ok($objVi->eq("a\x{302}\x{301}", "\x{1EA5}"));
ok($objVi->eq("a\x{302}\x{341}", "\x{1EA5}"));
ok($objVi->eq("a\x{302}\x{323}", "\x{1EAD}"));
ok($objVi->eq("A\x{302}\x{300}", "\x{1EA6}"));
ok($objVi->eq("A\x{302}\x{340}", "\x{1EA6}"));
ok($objVi->eq("A\x{302}\x{309}", "\x{1EA8}"));
ok($objVi->eq("A\x{302}\x{303}", "\x{1EAA}"));
ok($objVi->eq("A\x{302}\x{301}", "\x{1EA4}"));
ok($objVi->eq("A\x{302}\x{341}", "\x{1EA4}"));
ok($objVi->eq("A\x{302}\x{323}", "\x{1EAC}"));
ok($objVi->eq("e\x{300}", _pack_U(0xE8)));
ok($objVi->eq("e\x{340}", _pack_U(0xE8)));
ok($objVi->eq("e\x{309}", "\x{1EBB}"));
ok($objVi->eq("e\x{303}", "\x{1EBD}"));
ok($objVi->eq("e\x{301}", _pack_U(0xE9)));
ok($objVi->eq("e\x{341}", _pack_U(0xE9)));
ok($objVi->eq("e\x{323}", "\x{1EB9}"));
ok($objVi->eq("E\x{300}", _pack_U(0xC8)));
ok($objVi->eq("E\x{340}", _pack_U(0xC8)));
ok($objVi->eq("E\x{309}", "\x{1EBA}"));
ok($objVi->eq("E\x{303}", "\x{1EBC}"));
ok($objVi->eq("E\x{301}", _pack_U(0xC9)));
ok($objVi->eq("E\x{341}", _pack_U(0xC9)));
ok($objVi->eq("E\x{323}", "\x{1EB8}"));
ok($objVi->eq("e\x{302}\x{300}", "\x{1EC1}"));
ok($objVi->eq("e\x{302}\x{340}", "\x{1EC1}"));
ok($objVi->eq("e\x{302}\x{309}", "\x{1EC3}"));
ok($objVi->eq("e\x{302}\x{303}", "\x{1EC5}"));
ok($objVi->eq("e\x{302}\x{301}", "\x{1EBF}"));
ok($objVi->eq("e\x{302}\x{341}", "\x{1EBF}"));
ok($objVi->eq("e\x{302}\x{323}", "\x{1EC7}"));
ok($objVi->eq("E\x{302}\x{300}", "\x{1EC0}"));
ok($objVi->eq("E\x{302}\x{340}", "\x{1EC0}"));
ok($objVi->eq("E\x{302}\x{309}", "\x{1EC2}"));
ok($objVi->eq("E\x{302}\x{303}", "\x{1EC4}"));
ok($objVi->eq("E\x{302}\x{301}", "\x{1EBE}"));
ok($objVi->eq("E\x{302}\x{341}", "\x{1EBE}"));
ok($objVi->eq("E\x{302}\x{323}", "\x{1EC6}"));
ok($objVi->eq("i\x{300}", _pack_U(0xEC)));
ok($objVi->eq("i\x{340}", _pack_U(0xEC)));
ok($objVi->eq("i\x{309}", "\x{1EC9}"));
ok($objVi->eq("i\x{303}", "\x{129}"));
ok($objVi->eq("i\x{301}", _pack_U(0xED)));
ok($objVi->eq("i\x{341}", _pack_U(0xED)));
ok($objVi->eq("i\x{323}", "\x{1ECB}"));
ok($objVi->eq("I\x{300}", _pack_U(0xCC)));
ok($objVi->eq("I\x{340}", _pack_U(0xCC)));
ok($objVi->eq("I\x{309}", "\x{1EC8}"));
ok($objVi->eq("I\x{303}", "\x{128}"));
ok($objVi->eq("I\x{301}", _pack_U(0xCD)));
ok($objVi->eq("I\x{341}", _pack_U(0xCD)));
ok($objVi->eq("I\x{323}", "\x{1ECA}"));
ok($objVi->eq("o\x{300}", _pack_U(0xF2)));
ok($objVi->eq("o\x{340}", _pack_U(0xF2)));
ok($objVi->eq("o\x{309}", "\x{1ECF}"));
ok($objVi->eq("o\x{303}", _pack_U(0xF5)));
ok($objVi->eq("o\x{301}", _pack_U(0xF3)));
ok($objVi->eq("o\x{341}", _pack_U(0xF3)));
ok($objVi->eq("o\x{323}", "\x{1ECD}"));
ok($objVi->eq("O\x{300}", _pack_U(0xD2)));
ok($objVi->eq("O\x{340}", _pack_U(0xD2)));
ok($objVi->eq("O\x{309}", "\x{1ECE}"));
ok($objVi->eq("O\x{303}", _pack_U(0xD5)));
ok($objVi->eq("O\x{301}", _pack_U(0xD3)));
ok($objVi->eq("O\x{341}", _pack_U(0xD3)));
ok($objVi->eq("O\x{323}", "\x{1ECC}"));
ok($objVi->eq("o\x{302}\x{300}", "\x{1ED3}"));
ok($objVi->eq("o\x{302}\x{340}", "\x{1ED3}"));
ok($objVi->eq("o\x{302}\x{309}", "\x{1ED5}"));
ok($objVi->eq("o\x{302}\x{303}", "\x{1ED7}"));
ok($objVi->eq("o\x{302}\x{301}", "\x{1ED1}"));
ok($objVi->eq("o\x{302}\x{341}", "\x{1ED1}"));
ok($objVi->eq("o\x{302}\x{323}", "\x{1ED9}"));
ok($objVi->eq("O\x{302}\x{300}", "\x{1ED2}"));
ok($objVi->eq("O\x{302}\x{340}", "\x{1ED2}"));
ok($objVi->eq("O\x{302}\x{309}", "\x{1ED4}"));
ok($objVi->eq("O\x{302}\x{303}", "\x{1ED6}"));
ok($objVi->eq("O\x{302}\x{301}", "\x{1ED0}"));
ok($objVi->eq("O\x{302}\x{341}", "\x{1ED0}"));
ok($objVi->eq("O\x{302}\x{323}", "\x{1ED8}"));
ok($objVi->eq("o\x{31B}\x{300}", "\x{1EDD}"));
ok($objVi->eq("o\x{31B}\x{340}", "\x{1EDD}"));
ok($objVi->eq("o\x{31B}\x{309}", "\x{1EDF}"));
ok($objVi->eq("o\x{31B}\x{303}", "\x{1EE1}"));
ok($objVi->eq("o\x{31B}\x{301}", "\x{1EDB}"));
ok($objVi->eq("o\x{31B}\x{341}", "\x{1EDB}"));
ok($objVi->eq("o\x{31B}\x{323}", "\x{1EE3}"));
ok($objVi->eq("O\x{31B}\x{300}", "\x{1EDC}"));
ok($objVi->eq("O\x{31B}\x{340}", "\x{1EDC}"));
ok($objVi->eq("O\x{31B}\x{309}", "\x{1EDE}"));
ok($objVi->eq("O\x{31B}\x{303}", "\x{1EE0}"));
ok($objVi->eq("O\x{31B}\x{301}", "\x{1EDA}"));
ok($objVi->eq("O\x{31B}\x{341}", "\x{1EDA}"));
ok($objVi->eq("O\x{31B}\x{323}", "\x{1EE2}"));
ok($objVi->eq("u\x{300}", _pack_U(0xF9)));
ok($objVi->eq("u\x{340}", _pack_U(0xF9)));
ok($objVi->eq("u\x{309}", "\x{1EE7}"));
ok($objVi->eq("u\x{303}", "\x{169}"));
ok($objVi->eq("u\x{301}", _pack_U(0xFA)));
ok($objVi->eq("u\x{341}", _pack_U(0xFA)));
ok($objVi->eq("u\x{323}", "\x{1EE5}"));
ok($objVi->eq("U\x{300}", _pack_U(0xD9)));
ok($objVi->eq("U\x{340}", _pack_U(0xD9)));
ok($objVi->eq("U\x{309}", "\x{1EE6}"));
ok($objVi->eq("U\x{303}", "\x{168}"));
ok($objVi->eq("U\x{301}", _pack_U(0xDA)));
ok($objVi->eq("U\x{341}", _pack_U(0xDA)));
ok($objVi->eq("U\x{323}", "\x{1EE4}"));
ok($objVi->eq("u\x{31B}\x{300}", "\x{1EEB}"));
ok($objVi->eq("u\x{31B}\x{340}", "\x{1EEB}"));
ok($objVi->eq("u\x{31B}\x{309}", "\x{1EED}"));
ok($objVi->eq("u\x{31B}\x{303}", "\x{1EEF}"));
ok($objVi->eq("u\x{31B}\x{301}", "\x{1EE9}"));
ok($objVi->eq("u\x{31B}\x{341}", "\x{1EE9}"));
ok($objVi->eq("u\x{31B}\x{323}", "\x{1EF1}"));
ok($objVi->eq("U\x{31B}\x{300}", "\x{1EEA}"));
ok($objVi->eq("U\x{31B}\x{340}", "\x{1EEA}"));
ok($objVi->eq("U\x{31B}\x{309}", "\x{1EEC}"));
ok($objVi->eq("U\x{31B}\x{303}", "\x{1EEE}"));
ok($objVi->eq("U\x{31B}\x{301}", "\x{1EE8}"));
ok($objVi->eq("U\x{31B}\x{341}", "\x{1EE8}"));
ok($objVi->eq("U\x{31B}\x{323}", "\x{1EF0}"));
ok($objVi->eq("y\x{300}", "\x{1EF3}"));
ok($objVi->eq("y\x{340}", "\x{1EF3}"));
ok($objVi->eq("y\x{309}", "\x{1EF7}"));
ok($objVi->eq("y\x{303}", "\x{1EF9}"));
ok($objVi->eq("y\x{301}", _pack_U(0xFD)));
ok($objVi->eq("y\x{341}", _pack_U(0xFD)));
ok($objVi->eq("y\x{323}", "\x{1EF5}"));
ok($objVi->eq("Y\x{300}", "\x{1EF2}"));
ok($objVi->eq("Y\x{340}", "\x{1EF2}"));
ok($objVi->eq("Y\x{309}", "\x{1EF6}"));
ok($objVi->eq("Y\x{303}", "\x{1EF8}"));
ok($objVi->eq("Y\x{301}", _pack_U(0xDD)));
ok($objVi->eq("Y\x{341}", _pack_U(0xDD)));
ok($objVi->eq("Y\x{323}", "\x{1EF4}"));

# 242

ok($objVi->eq("a\x{306}\x{323}", "\x{1EA1}\x{306}"));
ok($objVi->eq("A\x{306}\x{323}", "\x{1EA0}\x{306}"));
ok($objVi->eq("a\x{302}\x{323}", "\x{1EA1}\x{302}"));
ok($objVi->eq("A\x{302}\x{323}", "\x{1EA0}\x{302}"));
ok($objVi->eq("e\x{302}\x{323}", "\x{1EB9}\x{302}"));
ok($objVi->eq("E\x{302}\x{323}", "\x{1EB8}\x{302}"));
ok($objVi->eq("o\x{302}\x{323}", "\x{1ECD}\x{302}"));
ok($objVi->eq("O\x{302}\x{323}", "\x{1ECC}\x{302}"));
ok($objVi->eq("o\x{31B}\x{300}", _pack_U(0xF2, 0x31B)));
ok($objVi->eq("o\x{31B}\x{340}", _pack_U(0xF2, 0x31B)));
ok($objVi->eq("o\x{31B}\x{309}", "\x{1ECF}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{303}", _pack_U(0xF5, 0x31B)));
ok($objVi->eq("o\x{31B}\x{301}", _pack_U(0xF3, 0x31B)));
ok($objVi->eq("o\x{31B}\x{341}", _pack_U(0xF3, 0x31B)));
ok($objVi->eq("o\x{31B}\x{323}", "\x{1ECD}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{300}", _pack_U(0xD2, 0x31B)));
ok($objVi->eq("O\x{31B}\x{340}", _pack_U(0xD2, 0x31B)));
ok($objVi->eq("O\x{31B}\x{309}", "\x{1ECE}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{303}", _pack_U(0xD5, 0x31B)));
ok($objVi->eq("O\x{31B}\x{301}", _pack_U(0xD3, 0x31B)));
ok($objVi->eq("O\x{31B}\x{341}", _pack_U(0xD3, 0x31B)));
ok($objVi->eq("O\x{31B}\x{323}", "\x{1ECC}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{300}", _pack_U(0xF9, 0x31B)));
ok($objVi->eq("u\x{31B}\x{340}", _pack_U(0xF9, 0x31B)));
ok($objVi->eq("u\x{31B}\x{309}", "\x{1EE7}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{303}", "\x{169}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{301}", _pack_U(0xFA, 0x31B)));
ok($objVi->eq("u\x{31B}\x{341}", _pack_U(0xFA, 0x31B)));
ok($objVi->eq("u\x{31B}\x{323}", "\x{1EE5}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{300}", _pack_U(0xD9, 0x31B)));
ok($objVi->eq("U\x{31B}\x{340}", _pack_U(0xD9, 0x31B)));
ok($objVi->eq("U\x{31B}\x{309}", "\x{1EE6}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{303}", "\x{168}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{301}", _pack_U(0xDA, 0x31B)));
ok($objVi->eq("U\x{31B}\x{341}", _pack_U(0xDA, 0x31B)));
ok($objVi->eq("U\x{31B}\x{323}", "\x{1EE4}\x{31B}"));

# 278

ok($objVi->eq("a\x{306}\x{323}", "a\x{323}\x{306}"));
ok($objVi->eq("A\x{306}\x{323}", "A\x{323}\x{306}"));
ok($objVi->eq("a\x{302}\x{323}", "a\x{323}\x{302}"));
ok($objVi->eq("A\x{302}\x{323}", "A\x{323}\x{302}"));
ok($objVi->eq("e\x{302}\x{323}", "e\x{323}\x{302}"));
ok($objVi->eq("E\x{302}\x{323}", "E\x{323}\x{302}"));
ok($objVi->eq("o\x{302}\x{323}", "o\x{323}\x{302}"));
ok($objVi->eq("O\x{302}\x{323}", "O\x{323}\x{302}"));
ok($objVi->eq("o\x{31B}\x{300}", "o\x{300}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{340}", "o\x{340}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{309}", "o\x{309}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{303}", "o\x{303}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{301}", "o\x{301}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{341}", "o\x{341}\x{31B}"));
ok($objVi->eq("o\x{31B}\x{323}", "o\x{323}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{300}", "O\x{300}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{340}", "O\x{340}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{309}", "O\x{309}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{303}", "O\x{303}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{301}", "O\x{301}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{341}", "O\x{341}\x{31B}"));
ok($objVi->eq("O\x{31B}\x{323}", "O\x{323}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{300}", "u\x{300}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{340}", "u\x{340}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{309}", "u\x{309}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{303}", "u\x{303}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{301}", "u\x{301}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{341}", "u\x{341}\x{31B}"));
ok($objVi->eq("u\x{31B}\x{323}", "u\x{323}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{300}", "U\x{300}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{340}", "U\x{340}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{309}", "U\x{309}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{303}", "U\x{303}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{301}", "U\x{301}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{341}", "U\x{341}\x{31B}"));
ok($objVi->eq("U\x{31B}\x{323}", "U\x{323}\x{31B}"));

# 314

ok($objVi->eq("a\x{306}\x{300}", "\x{103}\x{300}"));
ok($objVi->eq("a\x{306}\x{340}", "\x{103}\x{340}"));
ok($objVi->eq("a\x{306}\x{309}", "\x{103}\x{309}"));
ok($objVi->eq("a\x{306}\x{303}", "\x{103}\x{303}"));
ok($objVi->eq("a\x{306}\x{301}", "\x{103}\x{301}"));
ok($objVi->eq("a\x{306}\x{341}", "\x{103}\x{341}"));
ok($objVi->eq("a\x{306}\x{323}", "\x{103}\x{323}"));
ok($objVi->eq("A\x{306}\x{300}", "\x{102}\x{300}"));
ok($objVi->eq("A\x{306}\x{340}", "\x{102}\x{340}"));
ok($objVi->eq("A\x{306}\x{309}", "\x{102}\x{309}"));
ok($objVi->eq("A\x{306}\x{303}", "\x{102}\x{303}"));
ok($objVi->eq("A\x{306}\x{301}", "\x{102}\x{301}"));
ok($objVi->eq("A\x{306}\x{341}", "\x{102}\x{341}"));
ok($objVi->eq("A\x{306}\x{323}", "\x{102}\x{323}"));
ok($objVi->eq("a\x{302}\x{300}", _pack_U(0xE2, 0x300)));
ok($objVi->eq("a\x{302}\x{340}", _pack_U(0xE2, 0x340)));
ok($objVi->eq("a\x{302}\x{309}", _pack_U(0xE2, 0x309)));
ok($objVi->eq("a\x{302}\x{303}", _pack_U(0xE2, 0x303)));
ok($objVi->eq("a\x{302}\x{301}", _pack_U(0xE2, 0x301)));
ok($objVi->eq("a\x{302}\x{341}", _pack_U(0xE2, 0x341)));
ok($objVi->eq("a\x{302}\x{323}", _pack_U(0xE2, 0x323)));
ok($objVi->eq("A\x{302}\x{300}", _pack_U(0xC2, 0x300)));
ok($objVi->eq("A\x{302}\x{340}", _pack_U(0xC2, 0x340)));
ok($objVi->eq("A\x{302}\x{309}", _pack_U(0xC2, 0x309)));
ok($objVi->eq("A\x{302}\x{303}", _pack_U(0xC2, 0x303)));
ok($objVi->eq("A\x{302}\x{301}", _pack_U(0xC2, 0x301)));
ok($objVi->eq("A\x{302}\x{341}", _pack_U(0xC2, 0x341)));
ok($objVi->eq("A\x{302}\x{323}", _pack_U(0xC2, 0x323)));
ok($objVi->eq("e\x{302}\x{300}", _pack_U(0xEA, 0x300)));
ok($objVi->eq("e\x{302}\x{340}", _pack_U(0xEA, 0x340)));
ok($objVi->eq("e\x{302}\x{309}", _pack_U(0xEA, 0x309)));
ok($objVi->eq("e\x{302}\x{303}", _pack_U(0xEA, 0x303)));
ok($objVi->eq("e\x{302}\x{301}", _pack_U(0xEA, 0x301)));
ok($objVi->eq("e\x{302}\x{341}", _pack_U(0xEA, 0x341)));
ok($objVi->eq("e\x{302}\x{323}", _pack_U(0xEA, 0x323)));
ok($objVi->eq("E\x{302}\x{300}", _pack_U(0xCA, 0x300)));
ok($objVi->eq("E\x{302}\x{340}", _pack_U(0xCA, 0x340)));
ok($objVi->eq("E\x{302}\x{309}", _pack_U(0xCA, 0x309)));
ok($objVi->eq("E\x{302}\x{303}", _pack_U(0xCA, 0x303)));
ok($objVi->eq("E\x{302}\x{301}", _pack_U(0xCA, 0x301)));
ok($objVi->eq("E\x{302}\x{341}", _pack_U(0xCA, 0x341)));
ok($objVi->eq("E\x{302}\x{323}", _pack_U(0xCA, 0x323)));
ok($objVi->eq("o\x{302}\x{300}", _pack_U(0xF4, 0x300)));
ok($objVi->eq("o\x{302}\x{340}", _pack_U(0xF4, 0x340)));
ok($objVi->eq("o\x{302}\x{309}", _pack_U(0xF4, 0x309)));
ok($objVi->eq("o\x{302}\x{303}", _pack_U(0xF4, 0x303)));
ok($objVi->eq("o\x{302}\x{301}", _pack_U(0xF4, 0x301)));
ok($objVi->eq("o\x{302}\x{341}", _pack_U(0xF4, 0x341)));
ok($objVi->eq("o\x{302}\x{323}", _pack_U(0xF4, 0x323)));
ok($objVi->eq("O\x{302}\x{300}", _pack_U(0xD4, 0x300)));
ok($objVi->eq("O\x{302}\x{340}", _pack_U(0xD4, 0x340)));
ok($objVi->eq("O\x{302}\x{309}", _pack_U(0xD4, 0x309)));
ok($objVi->eq("O\x{302}\x{303}", _pack_U(0xD4, 0x303)));
ok($objVi->eq("O\x{302}\x{301}", _pack_U(0xD4, 0x301)));
ok($objVi->eq("O\x{302}\x{341}", _pack_U(0xD4, 0x341)));
ok($objVi->eq("O\x{302}\x{323}", _pack_U(0xD4, 0x323)));
ok($objVi->eq("o\x{31B}\x{300}", "\x{1A1}\x{300}"));
ok($objVi->eq("o\x{31B}\x{340}", "\x{1A1}\x{340}"));
ok($objVi->eq("o\x{31B}\x{309}", "\x{1A1}\x{309}"));
ok($objVi->eq("o\x{31B}\x{303}", "\x{1A1}\x{303}"));
ok($objVi->eq("o\x{31B}\x{301}", "\x{1A1}\x{301}"));
ok($objVi->eq("o\x{31B}\x{341}", "\x{1A1}\x{341}"));
ok($objVi->eq("o\x{31B}\x{323}", "\x{1A1}\x{323}"));
ok($objVi->eq("O\x{31B}\x{300}", "\x{1A0}\x{300}"));
ok($objVi->eq("O\x{31B}\x{340}", "\x{1A0}\x{340}"));
ok($objVi->eq("O\x{31B}\x{309}", "\x{1A0}\x{309}"));
ok($objVi->eq("O\x{31B}\x{303}", "\x{1A0}\x{303}"));
ok($objVi->eq("O\x{31B}\x{301}", "\x{1A0}\x{301}"));
ok($objVi->eq("O\x{31B}\x{341}", "\x{1A0}\x{341}"));
ok($objVi->eq("O\x{31B}\x{323}", "\x{1A0}\x{323}"));
ok($objVi->eq("u\x{31B}\x{300}", "\x{1B0}\x{300}"));
ok($objVi->eq("u\x{31B}\x{340}", "\x{1B0}\x{340}"));
ok($objVi->eq("u\x{31B}\x{309}", "\x{1B0}\x{309}"));
ok($objVi->eq("u\x{31B}\x{303}", "\x{1B0}\x{303}"));
ok($objVi->eq("u\x{31B}\x{301}", "\x{1B0}\x{301}"));
ok($objVi->eq("u\x{31B}\x{341}", "\x{1B0}\x{341}"));
ok($objVi->eq("u\x{31B}\x{323}", "\x{1B0}\x{323}"));
ok($objVi->eq("U\x{31B}\x{300}", "\x{1AF}\x{300}"));
ok($objVi->eq("U\x{31B}\x{340}", "\x{1AF}\x{340}"));
ok($objVi->eq("U\x{31B}\x{309}", "\x{1AF}\x{309}"));
ok($objVi->eq("U\x{31B}\x{303}", "\x{1AF}\x{303}"));
ok($objVi->eq("U\x{31B}\x{301}", "\x{1AF}\x{301}"));
ok($objVi->eq("U\x{31B}\x{341}", "\x{1AF}\x{341}"));
ok($objVi->eq("U\x{31B}\x{323}", "\x{1AF}\x{323}"));

# 398

$objVi->change(normalization => undef);

ok($objVi->index("a\x{306}\x{323}", "a\x{323}\x{306}"), 0);
ok($objVi->index("o\x{31B}\x{300}", "o\x{300}\x{31B}"), 0);
ok($objVi->index("o\x{31B}\x{300}", "o\x{340}\x{31B}"), 0);
ok($objVi->index("o\x{31B}\x{301}", "o\x{301}\x{31B}"), 0);
ok($objVi->index("o\x{31B}\x{301}", "o\x{341}\x{31B}"), 0);
ok($objVi->index("a\x{306}\x{323}", _pack_U(0x1EA1, 0x306)), 0);
ok($objVi->index("o\x{31B}\x{300}", _pack_U(0x00F2, 0x31B)), 0);

ok($objVi->index("A\x{306}\x{323}", "a\x{323}\x{306}"), -1);
ok($objVi->index("O\x{31B}\x{300}", "o\x{300}\x{31B}"), -1);
ok($objVi->index("A\x{306}\x{323}", _pack_U(0x1EA1, 0x306)), -1);
ok($objVi->index("O\x{31B}\x{300}", _pack_U(0x00F2, 0x31B)), -1);

# 409

$objVi->change(level => 2);

ok($objVi->index("A\x{306}\x{323}", "a\x{323}\x{306}"), 0);
ok($objVi->index("O\x{31B}\x{300}", "o\x{300}\x{31B}"), 0);
ok($objVi->index("O\x{31B}\x{300}", "o\x{340}\x{31B}"), 0);
ok($objVi->index("O\x{31B}\x{301}", "o\x{301}\x{31B}"), 0);
ok($objVi->index("O\x{31B}\x{301}", "o\x{341}\x{31B}"), 0);
ok($objVi->index("A\x{306}\x{323}", _pack_U(0x1EA1, 0x306)), 0);
ok($objVi->index("O\x{31B}\x{300}", _pack_U(0x00F2, 0x31B)), 0);

ok($objVi->index("A\x{306}", "a\x{323}\x{306}"), -1);
ok($objVi->index("O\x{31B}", "o\x{300}\x{31B}"), -1);
ok($objVi->index("A\x{306}", _pack_U(0x1EA1, 0x306)), -1);
ok($objVi->index("O\x{31B}", _pack_U(0x00F2, 0x31B)), -1);

# 420

$objVi->change(level => 1);

ok($objVi->index("A\x{306}", "a\x{323}\x{306}"), 0);
ok($objVi->index("O\x{31B}", "o\x{300}\x{31B}"), 0);
ok($objVi->index("A\x{306}", _pack_U(0x1EA1, 0x306)), 0);
ok($objVi->index("O\x{31B}", _pack_U(0x00F2, 0x31B)), 0);

# 424

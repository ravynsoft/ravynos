
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..122\n"; }
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

my $objTo = Unicode::Collate::Locale->
    new(locale => 'TO', normalization => undef);

ok($objTo->getlocale, 'to');

$objTo->change(level => 1);

ok($objTo->lt("n", "ng"));
ok($objTo->lt("nz","ng"));
ok($objTo->gt("o", "ng"));
ok($objTo->lt("z", "\x{2BB}"));
ok($objTo->lt("z", "\x{2BD}"));

# 7

ok($objTo->eq("a", "a\x{301}"));
ok($objTo->eq("A", "A\x{301}"));
ok($objTo->eq("e", "e\x{301}"));
ok($objTo->eq("E", "E\x{301}"));
ok($objTo->eq("i", "i\x{301}"));
ok($objTo->eq("I", "I\x{301}"));
ok($objTo->eq("o", "o\x{301}"));
ok($objTo->eq("O", "O\x{301}"));
ok($objTo->eq("u", "u\x{301}"));
ok($objTo->eq("U", "U\x{301}"));

ok($objTo->eq("a\x{301}", "a\x{304}"));
ok($objTo->eq("A\x{301}", "A\x{304}"));
ok($objTo->eq("e\x{301}", "e\x{304}"));
ok($objTo->eq("E\x{301}", "E\x{304}"));
ok($objTo->eq("i\x{301}", "i\x{304}"));
ok($objTo->eq("I\x{301}", "I\x{304}"));
ok($objTo->eq("o\x{301}", "o\x{304}"));
ok($objTo->eq("O\x{301}", "O\x{304}"));
ok($objTo->eq("u\x{301}", "u\x{304}"));
ok($objTo->eq("U\x{301}", "U\x{304}"));

# 27

$objTo->change(level => 2);

ok($objTo->lt("a", "a\x{301}"));
ok($objTo->lt("A", "A\x{301}"));
ok($objTo->lt("e", "e\x{301}"));
ok($objTo->lt("E", "E\x{301}"));
ok($objTo->lt("i", "i\x{301}"));
ok($objTo->lt("I", "I\x{301}"));
ok($objTo->lt("o", "o\x{301}"));
ok($objTo->lt("O", "O\x{301}"));
ok($objTo->lt("u", "u\x{301}"));
ok($objTo->lt("U", "U\x{301}"));

ok($objTo->lt("a\x{301}", "a\x{304}"));
ok($objTo->lt("A\x{301}", "A\x{304}"));
ok($objTo->lt("e\x{301}", "e\x{304}"));
ok($objTo->lt("E\x{301}", "E\x{304}"));
ok($objTo->lt("i\x{301}", "i\x{304}"));
ok($objTo->lt("I\x{301}", "I\x{304}"));
ok($objTo->lt("o\x{301}", "o\x{304}"));
ok($objTo->lt("O\x{301}", "O\x{304}"));
ok($objTo->lt("u\x{301}", "u\x{304}"));
ok($objTo->lt("U\x{301}", "U\x{304}"));

# 47

ok($objTo->eq("ng", "Ng"));
ok($objTo->eq("Ng", "NG"));
ok($objTo->eq("NG", "\x{14B}"));
ok($objTo->eq("\x{14B}", "\x{14A}"));
ok($objTo->eq("\x{2BB}", "\x{2BD}"));

ok($objTo->eq("a\x{301}", "A\x{301}"));
ok($objTo->eq("a\x{304}", "A\x{304}"));
ok($objTo->eq("e\x{301}", "E\x{301}"));
ok($objTo->eq("e\x{304}", "E\x{304}"));
ok($objTo->eq("i\x{301}", "I\x{301}"));
ok($objTo->eq("i\x{304}", "I\x{304}"));
ok($objTo->eq("o\x{301}", "O\x{301}"));
ok($objTo->eq("o\x{304}", "O\x{304}"));
ok($objTo->eq("u\x{301}", "U\x{301}"));
ok($objTo->eq("u\x{304}", "U\x{304}"));

# 62

$objTo->change(level => 3);

ok($objTo->lt("ng", "Ng"));
ok($objTo->lt("Ng", "NG"));
ok($objTo->lt("NG", "\x{14B}"));
ok($objTo->lt("\x{14B}", "\x{14A}"));
ok($objTo->lt("\x{2BB}", "\x{2BD}"));

ok($objTo->lt("a\x{301}", "A\x{301}"));
ok($objTo->lt("a\x{304}", "A\x{304}"));
ok($objTo->lt("e\x{301}", "E\x{301}"));
ok($objTo->lt("e\x{304}", "E\x{304}"));
ok($objTo->lt("i\x{301}", "I\x{301}"));
ok($objTo->lt("i\x{304}", "I\x{304}"));
ok($objTo->lt("o\x{301}", "O\x{301}"));
ok($objTo->lt("o\x{304}", "O\x{304}"));
ok($objTo->lt("u\x{301}", "U\x{301}"));
ok($objTo->lt("u\x{304}", "U\x{304}"));

# 77

ok($objTo->eq("a\x{301}", _pack_U(0xE1)));
ok($objTo->eq("a\x{341}", _pack_U(0xE1)));
ok($objTo->eq("A\x{301}", _pack_U(0xC1)));
ok($objTo->eq("A\x{341}", _pack_U(0xC1)));
ok($objTo->eq("e\x{301}", _pack_U(0xE9)));
ok($objTo->eq("e\x{341}", _pack_U(0xE9)));
ok($objTo->eq("E\x{301}", _pack_U(0xC9)));
ok($objTo->eq("E\x{341}", _pack_U(0xC9)));
ok($objTo->eq("i\x{301}", _pack_U(0xED)));
ok($objTo->eq("i\x{341}", _pack_U(0xED)));
ok($objTo->eq("I\x{301}", _pack_U(0xCD)));
ok($objTo->eq("I\x{341}", _pack_U(0xCD)));
ok($objTo->eq("o\x{301}", _pack_U(0xF3)));
ok($objTo->eq("o\x{341}", _pack_U(0xF3)));
ok($objTo->eq("O\x{301}", _pack_U(0xD3)));
ok($objTo->eq("O\x{341}", _pack_U(0xD3)));
ok($objTo->eq("u\x{301}", _pack_U(0xFA)));
ok($objTo->eq("u\x{341}", _pack_U(0xFA)));
ok($objTo->eq("U\x{301}", _pack_U(0xDA)));
ok($objTo->eq("U\x{341}", _pack_U(0xDA)));

ok($objTo->eq("a\x{304}", "\x{101}"));
ok($objTo->eq("A\x{304}", "\x{100}"));
ok($objTo->eq("e\x{304}", "\x{113}"));
ok($objTo->eq("E\x{304}", "\x{112}"));
ok($objTo->eq("i\x{304}", "\x{12B}"));
ok($objTo->eq("I\x{304}", "\x{12A}"));
ok($objTo->eq("o\x{304}", "\x{14D}"));
ok($objTo->eq("O\x{304}", "\x{14C}"));
ok($objTo->eq("u\x{304}", "\x{16B}"));
ok($objTo->eq("U\x{304}", "\x{16A}"));

# 107

$objTo->change(upper_before_lower => 1);

ok($objTo->gt("ng", "Ng"));
ok($objTo->gt("Ng", "NG"));
ok($objTo->lt("NG", "\x{14B}"));
ok($objTo->gt("\x{14B}", "\x{14A}"));
ok($objTo->lt("\x{2BB}", "\x{2BD}"));

ok($objTo->gt("a\x{301}", "A\x{301}"));
ok($objTo->gt("a\x{304}", "A\x{304}"));
ok($objTo->gt("e\x{301}", "E\x{301}"));
ok($objTo->gt("e\x{304}", "E\x{304}"));
ok($objTo->gt("i\x{301}", "I\x{301}"));
ok($objTo->gt("i\x{304}", "I\x{304}"));
ok($objTo->gt("o\x{301}", "O\x{301}"));
ok($objTo->gt("o\x{304}", "O\x{304}"));
ok($objTo->gt("u\x{301}", "U\x{301}"));
ok($objTo->gt("u\x{304}", "U\x{304}"));

# 122

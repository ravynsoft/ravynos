
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..71\n"; }
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

my $objRo = Unicode::Collate::Locale->
    new(locale => 'RO', normalization => undef);

ok($objRo->getlocale, 'ro');

$objRo->change(level => 1);

ok($objRo->lt("a", "a\x{306}"));
ok($objRo->lt("a\x{306}", "a\x{302}"));
ok($objRo->gt("b", "a\x{302}"));
ok($objRo->eq("d", "d\x{335}")); # not tailored in CLDR 2.0
ok($objRo->gt("e", "d\x{335}"));
ok($objRo->lt("i", "i\x{302}"));
ok($objRo->gt("j", "i\x{302}"));
ok($objRo->lt("s", "s\x{327}"));
ok($objRo->gt("t", "s\x{327}"));
ok($objRo->lt("t", "t\x{327}"));
ok($objRo->gt("u", "t\x{327}"));
ok($objRo->eq("z", "z\x{307}")); # not tailored in CLDR 2.0
ok($objRo->lt("z\x{307}", "\x{292}")); # U+0292 EZH

# 15

$objRo->change(level => 2);

ok($objRo->eq("a\x{306}", "A\x{306}"));
ok($objRo->eq("a\x{302}", "A\x{302}"));
ok($objRo->eq("d\x{335}", "D\x{335}"));
ok($objRo->eq("i\x{302}", "I\x{302}"));
ok($objRo->eq("s\x{327}", "S\x{327}"));
ok($objRo->eq("t\x{327}", "T\x{327}"));
ok($objRo->eq("z\x{307}", "Z\x{307}"));

# 22

$objRo->change(level => 3);

ok($objRo->lt("a\x{306}", "A\x{306}"));
ok($objRo->lt("a\x{302}", "A\x{302}"));
ok($objRo->lt("d\x{335}", "D\x{335}"));
ok($objRo->lt("i\x{302}", "I\x{302}"));
ok($objRo->lt("s\x{327}", "S\x{327}"));
ok($objRo->lt("t\x{327}", "T\x{327}"));
ok($objRo->lt("z\x{307}", "Z\x{307}"));

ok($objRo->eq("s\x{327}", "s\x{326}"));
ok($objRo->eq("S\x{327}", "S\x{326}"));
ok($objRo->eq("t\x{327}", "t\x{326}"));
ok($objRo->eq("T\x{327}", "T\x{326}"));

# 33

ok($objRo->eq("a\x{306}", "\x{103}"));
ok($objRo->eq("A\x{306}", "\x{102}"));
ok($objRo->eq("a\x{302}", _pack_U(0xE2)));
ok($objRo->eq("A\x{302}", _pack_U(0xC2)));
ok($objRo->eq("d\x{335}", "\x{111}"));
ok($objRo->eq("D\x{335}", "\x{110}"));
ok($objRo->eq("i\x{302}", _pack_U(0xEE)));
ok($objRo->eq("I\x{302}", _pack_U(0xCE)));
ok($objRo->eq("s\x{327}", "\x{15F}"));
ok($objRo->eq("s\x{326}", "\x{219}"));
ok($objRo->eq("S\x{327}", "\x{15E}"));
ok($objRo->eq("S\x{326}", "\x{218}"));
ok($objRo->eq("t\x{327}", "\x{163}"));
ok($objRo->eq("t\x{326}", "\x{21B}"));
ok($objRo->eq("T\x{327}", "\x{162}"));
ok($objRo->eq("T\x{326}", "\x{21A}"));
ok($objRo->eq("z\x{307}", "\x{17C}"));
ok($objRo->eq("Z\x{307}", "\x{17B}"));

# 51

ok($objRo->eq("a\x{306}\x{300}", "\x{1EB1}"));
ok($objRo->eq("A\x{306}\x{300}", "\x{1EB0}"));
ok($objRo->eq("a\x{306}\x{301}", "\x{1EAF}"));
ok($objRo->eq("A\x{306}\x{301}", "\x{1EAE}"));
ok($objRo->eq("a\x{306}\x{303}", "\x{1EB5}"));
ok($objRo->eq("A\x{306}\x{303}", "\x{1EB4}"));
ok($objRo->eq("a\x{306}\x{309}", "\x{1EB3}"));
ok($objRo->eq("A\x{306}\x{309}", "\x{1EB2}"));
ok($objRo->eq("a\x{306}\x{323}", "\x{1EB7}"));
ok($objRo->eq("A\x{306}\x{323}", "\x{1EB6}"));

ok($objRo->eq("a\x{302}\x{300}", "\x{1EA7}"));
ok($objRo->eq("A\x{302}\x{300}", "\x{1EA6}"));
ok($objRo->eq("a\x{302}\x{301}", "\x{1EA5}"));
ok($objRo->eq("A\x{302}\x{301}", "\x{1EA4}"));
ok($objRo->eq("a\x{302}\x{303}", "\x{1EAB}"));
ok($objRo->eq("A\x{302}\x{303}", "\x{1EAA}"));
ok($objRo->eq("a\x{302}\x{309}", "\x{1EA9}"));
ok($objRo->eq("A\x{302}\x{309}", "\x{1EA8}"));
ok($objRo->eq("a\x{302}\x{323}", "\x{1EAD}"));
ok($objRo->eq("A\x{302}\x{323}", "\x{1EAC}"));

# 71

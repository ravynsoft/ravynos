
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..110\n"; }
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

my $eth  = _pack_U(0xF0);
my $ETH  = _pack_U(0xD0);
my $thrn = _pack_U(0xFE);
my $THRN = _pack_U(0xDE);
my $ae   = _pack_U(0xE6);
my $AE   = _pack_U(0xC6);
my $auml = _pack_U(0xE4);
my $Auml = _pack_U(0xC4);
my $ouml = _pack_U(0xF6);
my $Ouml = _pack_U(0xD6);
my $ostk = _pack_U(0xF8);
my $Ostk = _pack_U(0xD8);
my $arng = _pack_U(0xE5);
my $Arng = _pack_U(0xC5);

my $objIs = Unicode::Collate::Locale->
    new(locale => 'IS', normalization => undef);

ok($objIs->getlocale, 'is');

$objIs->change(level => 1);

ok($objIs->lt('a', "a\x{301}"));
ok($objIs->gt('b', "a\x{301}"));
ok($objIs->lt('d', $eth));
ok($objIs->gt('e', $eth));
ok($objIs->lt('e', "e\x{301}"));
ok($objIs->gt('f', "e\x{301}"));
ok($objIs->lt('i', "i\x{301}"));
ok($objIs->gt('j', "i\x{301}"));
ok($objIs->lt('o', "o\x{301}"));
ok($objIs->gt('p', "o\x{301}"));
ok($objIs->lt('u', "u\x{301}"));
ok($objIs->gt('v', "u\x{301}"));
ok($objIs->lt('y', "y\x{301}"));
ok($objIs->gt('z', "y\x{301}"));

# 16

ok($objIs->lt('z', $thrn));
ok($objIs->lt($thrn, $ae));
ok($objIs->lt($ae, $ouml));
ok($objIs->lt($ouml, $arng));
ok($objIs->lt($arng, "\x{1C0}"));

# 21

ok($objIs->eq('d', "d\x{335}"));
ok($objIs->eq($ae, $auml));
ok($objIs->eq($ouml, $ostk));

$objIs->change(level => 2);

ok($objIs->lt('d', "d\x{335}"));
ok($objIs->lt($ae, $auml));
ok($objIs->lt($ouml, $ostk));

# 27

ok($objIs->eq("a\x{301}", "A\x{301}"));
ok($objIs->eq("d\x{335}", "D\x{335}"));
ok($objIs->eq($eth, $ETH));
ok($objIs->eq("e\x{301}", "E\x{301}"));
ok($objIs->eq("i\x{301}", "I\x{301}"));
ok($objIs->eq("o\x{301}", "O\x{301}"));
ok($objIs->eq("u\x{301}", "U\x{301}"));
ok($objIs->eq("y\x{301}", "Y\x{301}"));
ok($objIs->eq($thrn, $THRN));
ok($objIs->eq($ae,   $AE));
ok($objIs->eq($AE, "\x{1D2D}"));
ok($objIs->eq($auml, $Auml));
ok($objIs->eq($ouml, $Ouml));
ok($objIs->eq($ostk, $Ostk));
ok($objIs->eq($arng, $Arng));

# 42

$objIs->change(level => 3);

ok($objIs->lt("a\x{301}", "A\x{301}"));
ok($objIs->lt("d\x{335}", "D\x{335}"));
ok($objIs->lt($eth, $ETH));
ok($objIs->lt("e\x{301}", "E\x{301}"));
ok($objIs->lt("i\x{301}", "I\x{301}"));
ok($objIs->lt("o\x{301}", "O\x{301}"));
ok($objIs->lt("u\x{301}", "U\x{301}"));
ok($objIs->lt("y\x{301}", "Y\x{301}"));
ok($objIs->lt($thrn, $THRN));
ok($objIs->lt($ae,   $AE));
ok($objIs->lt($AE, "\x{1D2D}"));
ok($objIs->lt($auml, $Auml));
ok($objIs->lt($ouml, $Ouml));
ok($objIs->lt($ostk, $Ostk));
ok($objIs->lt($arng, $Arng));

# 57

ok($objIs->eq("a\x{301}", _pack_U(0xE1)));
ok($objIs->eq("a\x{341}", _pack_U(0xE1)));
ok($objIs->eq("A\x{301}", _pack_U(0xC1)));
ok($objIs->eq("A\x{341}", _pack_U(0xC1)));
ok($objIs->eq("d\x{335}", "\x{111}"));
ok($objIs->eq("D\x{335}", "\x{110}"));
ok($objIs->eq("e\x{301}", _pack_U(0xE9)));
ok($objIs->eq("e\x{341}", _pack_U(0xE9)));
ok($objIs->eq("E\x{301}", _pack_U(0xC9)));
ok($objIs->eq("E\x{341}", _pack_U(0xC9)));
ok($objIs->eq("i\x{301}", _pack_U(0xED)));
ok($objIs->eq("i\x{341}", _pack_U(0xED)));
ok($objIs->eq("I\x{301}", _pack_U(0xCD)));
ok($objIs->eq("I\x{341}", _pack_U(0xCD)));
ok($objIs->eq("o\x{301}", _pack_U(0xF3)));
ok($objIs->eq("o\x{341}", _pack_U(0xF3)));
ok($objIs->eq("O\x{301}", _pack_U(0xD3)));
ok($objIs->eq("O\x{341}", _pack_U(0xD3)));
ok($objIs->eq("u\x{301}", _pack_U(0xFA)));
ok($objIs->eq("u\x{341}", _pack_U(0xFA)));
ok($objIs->eq("U\x{301}", _pack_U(0xDA)));
ok($objIs->eq("U\x{341}", _pack_U(0xDA)));
ok($objIs->eq("y\x{301}", _pack_U(0xFD)));
ok($objIs->eq("y\x{341}", _pack_U(0xFD)));
ok($objIs->eq("Y\x{301}", _pack_U(0xDD)));
ok($objIs->eq("Y\x{341}", _pack_U(0xDD)));
ok($objIs->eq("a\x{308}", $auml));
ok($objIs->eq("A\x{308}", $Auml));
ok($objIs->eq("o\x{308}", $ouml));
ok($objIs->eq("O\x{308}", $Ouml));
ok($objIs->eq("o\x{338}", $ostk));
ok($objIs->eq("O\x{338}", $Ostk));
ok($objIs->eq("a\x{30A}", $arng));
ok($objIs->eq("A\x{30A}", $Arng));

# 91

ok($objIs->eq("\x{1FD}", "$ae\x{301}"));
ok($objIs->eq("\x{1FC}", "$AE\x{301}"));
ok($objIs->eq("\x{1E3}", "$ae\x{304}"));
ok($objIs->eq("\x{1E2}", "$AE\x{304}"));
ok($objIs->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objIs->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objIs->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objIs->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objIs->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objIs->eq("O\x{338}\x{301}", "\x{1FE}"));
ok($objIs->eq("A\x{30A}", "\x{212B}"));
ok($objIs->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objIs->eq("A\x{30A}\x{301}", "\x{1FA}"));

# 104

$objIs->change(upper_before_lower => 1);

ok($objIs->gt($ae,   $AE));
ok($objIs->lt($AE, "\x{1D2D}"));
ok($objIs->gt($auml, $Auml));
ok($objIs->gt($ouml, $Ouml));
ok($objIs->gt($ostk, $Ostk));
ok($objIs->gt($arng, $Arng));

# 110

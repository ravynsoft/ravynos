
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..117\n"; }
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
my $uuml = _pack_U(0xFC);
my $Uuml = _pack_U(0xDC);
my $ae   = _pack_U(0xE6);
my $AE   = _pack_U(0xC6);
my $auml = _pack_U(0xE4);
my $Auml = _pack_U(0xC4);
my $ostk = _pack_U(0xF8);
my $Ostk = _pack_U(0xD8);
my $ouml = _pack_U(0xF6);
my $Ouml = _pack_U(0xD6);
my $arng = _pack_U(0xE5);
my $Arng = _pack_U(0xC5);

my $objDa = Unicode::Collate::Locale->
    new(locale => 'DA', normalization => undef);

ok($objDa->getlocale, 'da');

$objDa->change(level => 1);

ok($objDa->lt('z', $ae));
ok($objDa->lt($ae, $ostk));
ok($objDa->lt($ostk, $arng));
ok($objDa->lt($arng, "\x{1C0}"));

# 6

ok($objDa->eq('d', "\x{111}"));
ok($objDa->eq("\x{111}", $eth));
ok($objDa->eq('y', $uuml));
ok($objDa->eq($uuml, "\x{171}"));
ok($objDa->eq($ae, $auml));
ok($objDa->eq($auml, "\x{119}"));
ok($objDa->eq($ostk, $ouml));
ok($objDa->eq($ouml, "\x{151}"));
ok($objDa->eq("\x{151}", "\x{153}"));

# 15

$objDa->change(level => 2);

ok($objDa->lt('d', "\x{111}"));
ok($objDa->lt("\x{111}", $eth));
ok($objDa->lt('y', $uuml));
ok($objDa->lt($uuml, "\x{171}"));
ok($objDa->lt($ae, $auml));
ok($objDa->lt($auml, "\x{119}"));
ok($objDa->lt($ostk, $ouml));
ok($objDa->lt($ouml, "\x{151}"));
ok($objDa->lt("\x{151}", "\x{153}"));

# 24

ok($objDa->eq("\x{111}", "\x{110}"));
ok($objDa->eq($eth,  $ETH));
ok($objDa->eq('th',  $thrn));
ok($objDa->eq($thrn, 'TH'));
ok($objDa->eq('TH',  $THRN));
ok($objDa->eq($uuml, $Uuml));
ok($objDa->eq("\x{171}", "\x{170}"));
ok($objDa->eq($ae,   $AE));
ok($objDa->eq($AE, "\x{1D2D}"));
ok($objDa->eq($auml, $Auml));
ok($objDa->eq("\x{119}", "\x{118}"));
ok($objDa->eq($ostk, $Ostk));
ok($objDa->eq($ouml, $Ouml));
ok($objDa->eq("\x{151}", "\x{150}"));
ok($objDa->eq("\x{153}", "\x{152}"));
ok($objDa->eq($arng, $Arng));
ok($objDa->eq($Arng, 'AA'));
ok($objDa->eq('AA', 'aA'));
ok($objDa->eq('aA', 'Aa'));
ok($objDa->eq('Aa', $arng));
ok($objDa->eq($arng, 'aa'));

# 45

$objDa->change(level => 3);

ok($objDa->gt("\x{111}", "\x{110}"));
ok($objDa->gt($eth,  $ETH));
ok($objDa->lt('th',  $thrn));
ok($objDa->gt($thrn, 'TH'));
ok($objDa->lt('TH',  $THRN));
ok($objDa->gt($uuml, $Uuml));
ok($objDa->gt("\x{171}", "\x{170}"));
ok($objDa->gt($ae,   $AE));
ok($objDa->lt($AE, "\x{1D2D}"));
ok($objDa->gt($auml, $Auml));
ok($objDa->gt("\x{119}", "\x{118}"));
ok($objDa->gt($ostk, $Ostk));
ok($objDa->gt($ouml, $Ouml));
ok($objDa->gt("\x{151}", "\x{150}"));
ok($objDa->gt("\x{153}", "\x{152}"));
ok($objDa->gt($arng, $Arng));
ok($objDa->lt($Arng, 'AA'));
ok($objDa->lt('AA', 'aA'));
ok($objDa->lt('aA', 'Aa'));
ok($objDa->lt('Aa', $arng));
ok($objDa->lt($arng, 'aa'));

# 66

ok($objDa->eq("d\x{335}", "\x{111}"));
ok($objDa->eq("D\x{335}", "\x{110}"));
ok($objDa->eq("u\x{308}", $uuml));
ok($objDa->eq("U\x{308}", $Uuml));
ok($objDa->eq("u\x{30B}", "\x{171}"));
ok($objDa->eq("U\x{30B}", "\x{170}"));
ok($objDa->eq("a\x{308}", $auml));
ok($objDa->eq("A\x{308}", $Auml));
ok($objDa->eq("e\x{328}", "\x{119}"));
ok($objDa->eq("E\x{328}", "\x{118}"));
ok($objDa->eq("o\x{338}", $ostk));
ok($objDa->eq("O\x{338}", $Ostk));
ok($objDa->eq("o\x{308}", $ouml));
ok($objDa->eq("O\x{308}", $Ouml));
ok($objDa->eq("o\x{30B}", "\x{151}"));
ok($objDa->eq("O\x{30B}", "\x{150}"));
ok($objDa->eq("a\x{30A}", $arng));
ok($objDa->eq("A\x{30A}", $Arng));

# 84

ok($objDa->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objDa->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objDa->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objDa->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objDa->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objDa->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objDa->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objDa->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objDa->eq("\x{1FD}", "$ae\x{301}"));
ok($objDa->eq("\x{1FC}", "$AE\x{301}"));
ok($objDa->eq("\x{1E3}", "$ae\x{304}"));
ok($objDa->eq("\x{1E2}", "$AE\x{304}"));
ok($objDa->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objDa->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objDa->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objDa->eq("O\x{338}\x{301}", "\x{1FE}"));
ok($objDa->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objDa->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objDa->eq("A\x{30A}", "\x{212B}"));
ok($objDa->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objDa->eq("A\x{30A}\x{301}", "\x{1FA}"));

# 105

ok($objDa->gt("a", "A"));
ok($objDa->gt("b", "B"));
ok($objDa->gt("c", "C"));
ok($objDa->gt("x", "X"));
ok($objDa->gt("y", "Y"));
ok($objDa->gt("z", "Z"));

# 111

$objDa->change(upper_before_lower => 0);

ok($objDa->lt("a", "A"));
ok($objDa->lt("b", "B"));
ok($objDa->lt("c", "C"));
ok($objDa->lt("x", "X"));
ok($objDa->lt("y", "Y"));
ok($objDa->lt("z", "Z"));

# 117

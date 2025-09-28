
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..105\n"; }
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

my $objFo = Unicode::Collate::Locale->
    new(locale => 'FO', normalization => undef);

ok($objFo->getlocale, 'fo');

$objFo->change(level => 1);

ok($objFo->lt('z', $ae));
ok($objFo->lt($ae, $ostk));
ok($objFo->lt($ostk, $arng));
ok($objFo->lt($arng, "\x{1C0}"));

# 6

ok($objFo->eq('d', "\x{111}"));
ok($objFo->eq("\x{111}", $eth));
ok($objFo->eq('y', $uuml));
ok($objFo->eq($uuml, "\x{171}"));
ok($objFo->eq($ae, $auml));
ok($objFo->eq($auml, "\x{119}"));
ok($objFo->eq($ostk, $ouml));
ok($objFo->eq($ouml, "\x{151}"));
ok($objFo->eq("\x{151}", "\x{153}"));

# 15

$objFo->change(level => 2);

ok($objFo->lt('d', "\x{111}"));
ok($objFo->lt("\x{111}", $eth));
ok($objFo->lt('y', $uuml));
ok($objFo->lt($uuml, "\x{171}"));
ok($objFo->lt($ae, $auml));
ok($objFo->lt($auml, "\x{119}"));
ok($objFo->lt($ostk, $ouml));
ok($objFo->lt($ouml, "\x{151}"));
ok($objFo->lt("\x{151}", "\x{153}"));

# 24

ok($objFo->eq("\x{111}", "\x{110}"));
ok($objFo->eq($eth,  $ETH));
ok($objFo->eq('th',  $thrn));
ok($objFo->eq($thrn, 'TH'));
ok($objFo->eq('TH',  $THRN));
ok($objFo->eq($uuml, $Uuml));
ok($objFo->eq("\x{171}", "\x{170}"));
ok($objFo->eq($ae,   $AE));
ok($objFo->eq($AE, "\x{1D2D}"));
ok($objFo->eq($auml, $Auml));
ok($objFo->eq("\x{119}", "\x{118}"));
ok($objFo->eq($ostk, $Ostk));
ok($objFo->eq($ouml, $Ouml));
ok($objFo->eq("\x{151}", "\x{150}"));
ok($objFo->eq("\x{153}", "\x{152}"));
ok($objFo->eq($arng, $Arng));
ok($objFo->eq($Arng, 'aa'));
ok($objFo->eq('aa', 'aA'));
ok($objFo->eq('aA', 'Aa'));
ok($objFo->eq('Aa', 'AA'));
ok($objFo->eq('aa', 'AA'));

# 45

$objFo->change(level => 3);

ok($objFo->lt("\x{111}", "\x{110}"));
ok($objFo->lt($eth,  $ETH));
ok($objFo->lt('th',  $thrn));
ok($objFo->lt($thrn, 'TH'));
ok($objFo->lt('TH',  $THRN));
ok($objFo->lt($uuml, $Uuml));
ok($objFo->lt("\x{171}", "\x{170}"));
ok($objFo->lt($ae,   $AE));
ok($objFo->lt($AE, "\x{1D2D}"));
ok($objFo->lt($auml, $Auml));
ok($objFo->lt("\x{119}", "\x{118}"));
ok($objFo->lt($ostk, $Ostk));
ok($objFo->lt($ouml, $Ouml));
ok($objFo->lt("\x{151}", "\x{150}"));
ok($objFo->lt("\x{153}", "\x{152}"));
ok($objFo->lt($arng, $Arng));
ok($objFo->lt($Arng, 'aa'));
ok($objFo->lt('aa', 'aA'));
ok($objFo->lt('aA', 'Aa'));
ok($objFo->lt('Aa', 'AA'));
ok($objFo->lt('aa', 'AA'));

# 66

ok($objFo->eq("d\x{335}", "\x{111}"));
ok($objFo->eq("D\x{335}", "\x{110}"));
ok($objFo->eq("u\x{308}", $uuml));
ok($objFo->eq("U\x{308}", $Uuml));
ok($objFo->eq("u\x{30B}", "\x{171}"));
ok($objFo->eq("U\x{30B}", "\x{170}"));
ok($objFo->eq("a\x{308}", $auml));
ok($objFo->eq("A\x{308}", $Auml));
ok($objFo->eq("e\x{328}", "\x{119}"));
ok($objFo->eq("E\x{328}", "\x{118}"));
ok($objFo->eq("o\x{338}", $ostk));
ok($objFo->eq("O\x{338}", $Ostk));
ok($objFo->eq("o\x{308}", $ouml));
ok($objFo->eq("O\x{308}", $Ouml));
ok($objFo->eq("o\x{30B}", "\x{151}"));
ok($objFo->eq("O\x{30B}", "\x{150}"));
ok($objFo->eq("a\x{30A}", $arng));
ok($objFo->eq("A\x{30A}", $Arng));

# 84

ok($objFo->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objFo->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objFo->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objFo->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objFo->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objFo->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objFo->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objFo->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objFo->eq("\x{1FD}", "$ae\x{301}"));
ok($objFo->eq("\x{1FC}", "$AE\x{301}"));
ok($objFo->eq("\x{1E3}", "$ae\x{304}"));
ok($objFo->eq("\x{1E2}", "$AE\x{304}"));
ok($objFo->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objFo->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objFo->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objFo->eq("O\x{338}\x{301}", "\x{1FE}"));
ok($objFo->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objFo->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objFo->eq("A\x{30A}", "\x{212B}"));
ok($objFo->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objFo->eq("A\x{30A}\x{301}", "\x{1FA}"));

# 105

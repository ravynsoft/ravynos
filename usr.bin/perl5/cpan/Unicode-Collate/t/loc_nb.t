
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..101\n"; }
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

my $objNb = Unicode::Collate::Locale->
    new(locale => 'NB', normalization => undef);

ok($objNb->getlocale, 'nb');

$objNb->change(level => 1);

ok($objNb->lt('z', $ae));
ok($objNb->lt($ae, $ostk));
ok($objNb->lt($ostk, $arng));
ok($objNb->lt($arng, "\x{1C0}"));

# 6

ok($objNb->eq('d', "\x{111}"));
ok($objNb->eq("\x{111}", $eth));
ok($objNb->eq('y', $uuml));
ok($objNb->eq($uuml, "\x{171}"));
ok($objNb->eq($ae, $auml));
ok($objNb->eq($auml, "\x{119}"));
ok($objNb->eq($ostk, $ouml));
ok($objNb->eq($ouml, "\x{151}"));
ok($objNb->eq("\x{151}", "\x{153}"));
ok($objNb->eq($arng, 'aa'));

# 16

$objNb->change(level => 2);

ok($objNb->lt('d', "\x{111}"));
ok($objNb->lt("\x{111}", $eth));
ok($objNb->lt('y', $uuml));
ok($objNb->lt($uuml, "\x{171}"));
ok($objNb->lt($ae, $auml));
ok($objNb->lt($auml, "\x{119}"));
ok($objNb->lt($ostk, $ouml));
ok($objNb->lt($ouml, "\x{151}"));
ok($objNb->lt("\x{151}", "\x{153}"));
ok($objNb->lt($arng, 'aa'));

# 26

ok($objNb->eq("\x{111}", "\x{110}"));
ok($objNb->eq($eth,  $ETH));
ok($objNb->eq('th',  $thrn));
ok($objNb->eq($thrn, 'TH'));
ok($objNb->eq('TH',  $THRN));
ok($objNb->eq($uuml, $Uuml));
ok($objNb->eq("\x{171}", "\x{170}"));
ok($objNb->eq($ae,   $AE));
ok($objNb->eq($AE, "\x{1D2D}"));
ok($objNb->eq($auml, $Auml));
ok($objNb->eq("\x{119}", "\x{118}"));
ok($objNb->eq($ostk, $Ostk));
ok($objNb->eq($ouml, $Ouml));
ok($objNb->eq("\x{151}", "\x{150}"));
ok($objNb->eq("\x{153}", "\x{152}"));
ok($objNb->eq($arng, $Arng));
ok($objNb->eq('aa', 'Aa'));
ok($objNb->eq('Aa', 'AA'));

# 44

$objNb->change(level => 3);

ok($objNb->lt("\x{111}", "\x{110}"));
ok($objNb->lt($eth,  $ETH));
ok($objNb->lt('th',  $thrn));
ok($objNb->lt($thrn, 'TH'));
ok($objNb->lt('TH',  $THRN));
ok($objNb->lt($uuml, $Uuml));
ok($objNb->lt("\x{171}", "\x{170}"));
ok($objNb->lt($ae,   $AE));
ok($objNb->lt($AE, "\x{1D2D}"));
ok($objNb->lt($auml, $Auml));
ok($objNb->lt("\x{119}", "\x{118}"));
ok($objNb->lt($ostk, $Ostk));
ok($objNb->lt($ouml, $Ouml));
ok($objNb->lt("\x{151}", "\x{150}"));
ok($objNb->lt("\x{153}", "\x{152}"));
ok($objNb->lt($arng, $Arng));
ok($objNb->lt('aa', 'Aa'));
ok($objNb->lt('Aa', 'AA'));

# 62

ok($objNb->eq("d\x{335}", "\x{111}"));
ok($objNb->eq("D\x{335}", "\x{110}"));
ok($objNb->eq("u\x{308}", $uuml));
ok($objNb->eq("U\x{308}", $Uuml));
ok($objNb->eq("u\x{30B}", "\x{171}"));
ok($objNb->eq("U\x{30B}", "\x{170}"));
ok($objNb->eq("a\x{308}", $auml));
ok($objNb->eq("A\x{308}", $Auml));
ok($objNb->eq("e\x{328}", "\x{119}"));
ok($objNb->eq("E\x{328}", "\x{118}"));
ok($objNb->eq("o\x{338}", $ostk));
ok($objNb->eq("O\x{338}", $Ostk));
ok($objNb->eq("o\x{308}", $ouml));
ok($objNb->eq("O\x{308}", $Ouml));
ok($objNb->eq("o\x{30B}", "\x{151}"));
ok($objNb->eq("O\x{30B}", "\x{150}"));
ok($objNb->eq("a\x{30A}", $arng));
ok($objNb->eq("A\x{30A}", $Arng));

# 80

ok($objNb->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objNb->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objNb->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objNb->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objNb->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objNb->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objNb->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objNb->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objNb->eq("\x{1FD}", "$ae\x{301}"));
ok($objNb->eq("\x{1FC}", "$AE\x{301}"));
ok($objNb->eq("\x{1E3}", "$ae\x{304}"));
ok($objNb->eq("\x{1E2}", "$AE\x{304}"));
ok($objNb->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objNb->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objNb->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objNb->eq("O\x{338}\x{301}", "\x{1FE}"));
ok($objNb->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objNb->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objNb->eq("A\x{30A}", "\x{212B}"));
ok($objNb->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objNb->eq("A\x{30A}\x{301}", "\x{1FA}"));

# 101


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

my $objNn = Unicode::Collate::Locale->
    new(locale => 'NN', normalization => undef);

ok($objNn->getlocale, 'nn');

$objNn->change(level => 1);

ok($objNn->lt('z', $ae));
ok($objNn->lt($ae, $ostk));
ok($objNn->lt($ostk, $arng));
ok($objNn->lt($arng, "\x{1C0}"));

# 6

ok($objNn->eq('d', "\x{111}"));
ok($objNn->eq("\x{111}", $eth));
ok($objNn->eq('y', $uuml));
ok($objNn->eq($uuml, "\x{171}"));
ok($objNn->eq($ae, $auml));
ok($objNn->eq($auml, "\x{119}"));
ok($objNn->eq($ostk, $ouml));
ok($objNn->eq($ouml, "\x{151}"));
ok($objNn->eq("\x{151}", "\x{153}"));
ok($objNn->eq($arng, 'aa'));

# 16

$objNn->change(level => 2);

ok($objNn->lt('d', "\x{111}"));
ok($objNn->lt("\x{111}", $eth));
ok($objNn->lt('y', $uuml));
ok($objNn->lt($uuml, "\x{171}"));
ok($objNn->lt($ae, $auml));
ok($objNn->lt($auml, "\x{119}"));
ok($objNn->lt($ostk, $ouml));
ok($objNn->lt($ouml, "\x{151}"));
ok($objNn->lt("\x{151}", "\x{153}"));
ok($objNn->lt($arng, 'aa'));

# 26

ok($objNn->eq("\x{111}", "\x{110}"));
ok($objNn->eq($eth,  $ETH));
ok($objNn->eq('th',  $thrn));
ok($objNn->eq($thrn, 'TH'));
ok($objNn->eq('TH',  $THRN));
ok($objNn->eq($uuml, $Uuml));
ok($objNn->eq("\x{171}", "\x{170}"));
ok($objNn->eq($ae,   $AE));
ok($objNn->eq($AE, "\x{1D2D}"));
ok($objNn->eq($auml, $Auml));
ok($objNn->eq("\x{119}", "\x{118}"));
ok($objNn->eq($ostk, $Ostk));
ok($objNn->eq($ouml, $Ouml));
ok($objNn->eq("\x{151}", "\x{150}"));
ok($objNn->eq("\x{153}", "\x{152}"));
ok($objNn->eq($arng, $Arng));
ok($objNn->eq('aa', 'Aa'));
ok($objNn->eq('Aa', 'AA'));

# 44

$objNn->change(level => 3);

ok($objNn->lt("\x{111}", "\x{110}"));
ok($objNn->lt($eth,  $ETH));
ok($objNn->lt('th',  $thrn));
ok($objNn->lt($thrn, 'TH'));
ok($objNn->lt('TH',  $THRN));
ok($objNn->lt($uuml, $Uuml));
ok($objNn->lt("\x{171}", "\x{170}"));
ok($objNn->lt($ae,   $AE));
ok($objNn->lt($AE, "\x{1D2D}"));
ok($objNn->lt($auml, $Auml));
ok($objNn->lt("\x{119}", "\x{118}"));
ok($objNn->lt($ostk, $Ostk));
ok($objNn->lt($ouml, $Ouml));
ok($objNn->lt("\x{151}", "\x{150}"));
ok($objNn->lt("\x{153}", "\x{152}"));
ok($objNn->lt($arng, $Arng));
ok($objNn->lt('aa', 'Aa'));
ok($objNn->lt('Aa', 'AA'));

# 62

ok($objNn->eq("d\x{335}", "\x{111}"));
ok($objNn->eq("D\x{335}", "\x{110}"));
ok($objNn->eq("u\x{308}", $uuml));
ok($objNn->eq("U\x{308}", $Uuml));
ok($objNn->eq("u\x{30B}", "\x{171}"));
ok($objNn->eq("U\x{30B}", "\x{170}"));
ok($objNn->eq("a\x{308}", $auml));
ok($objNn->eq("A\x{308}", $Auml));
ok($objNn->eq("e\x{328}", "\x{119}"));
ok($objNn->eq("E\x{328}", "\x{118}"));
ok($objNn->eq("o\x{338}", $ostk));
ok($objNn->eq("O\x{338}", $Ostk));
ok($objNn->eq("o\x{308}", $ouml));
ok($objNn->eq("O\x{308}", $Ouml));
ok($objNn->eq("o\x{30B}", "\x{151}"));
ok($objNn->eq("O\x{30B}", "\x{150}"));
ok($objNn->eq("a\x{30A}", $arng));
ok($objNn->eq("A\x{30A}", $Arng));

# 80

ok($objNn->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objNn->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objNn->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objNn->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objNn->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objNn->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objNn->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objNn->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objNn->eq("\x{1FD}", "$ae\x{301}"));
ok($objNn->eq("\x{1FC}", "$AE\x{301}"));
ok($objNn->eq("\x{1E3}", "$ae\x{304}"));
ok($objNn->eq("\x{1E2}", "$AE\x{304}"));
ok($objNn->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objNn->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objNn->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objNn->eq("O\x{338}\x{301}", "\x{1FE}"));
ok($objNn->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objNn->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objNn->eq("A\x{30A}", "\x{212B}"));
ok($objNn->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objNn->eq("A\x{30A}\x{301}", "\x{1FA}"));

# 101

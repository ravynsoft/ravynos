
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..115\n"; }
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
my $arng = _pack_U(0xE5);
my $Arng = _pack_U(0xC5);
my $auml = _pack_U(0xE4);
my $Auml = _pack_U(0xC4);
my $ae   = _pack_U(0xE6);
my $AE   = _pack_U(0xC6);
my $ouml = _pack_U(0xF6);
my $Ouml = _pack_U(0xD6);
my $ostk = _pack_U(0xF8);
my $Ostk = _pack_U(0xD8);
my $ocrc = _pack_U(0xF4);
my $Ocrc = _pack_U(0xD4);

my $objSv = Unicode::Collate::Locale->
    new(locale => 'SV', normalization => undef);

ok($objSv->getlocale, 'sv');

$objSv->change(level => 1);

ok($objSv->lt('z', $arng));
ok($objSv->lt($arng, $auml));
ok($objSv->lt($auml, $ouml));
ok($objSv->lt($ouml, "\x{1C0}"));

# 6

ok($objSv->eq('d', "\x{111}"));
ok($objSv->eq("\x{111}", $eth));
ok($objSv->eq('v', 'w'));
ok($objSv->eq('y', $uuml));
ok($objSv->eq($uuml, "\x{171}"));

ok($objSv->eq($auml, $ae));
ok($objSv->eq($ae, "\x{119}"));
ok($objSv->eq($ouml, $ostk));
ok($objSv->eq($ostk, "\x{151}"));
ok($objSv->eq("\x{151}", "\x{153}"));
ok($objSv->eq("\x{153}", $ocrc));

# 17

$objSv->change(level => 2);

ok($objSv->lt('d', "\x{111}"));
ok($objSv->lt("\x{111}", $eth));
ok($objSv->lt('v', 'w'));
ok($objSv->lt('y', $uuml));
ok($objSv->lt($uuml, "\x{171}"));

ok($objSv->lt($auml, $ae));
ok($objSv->lt($ae, "\x{119}"));
ok($objSv->lt($ouml, $ostk));
ok($objSv->lt($ostk, "\x{151}"));
ok($objSv->lt("\x{151}", "\x{153}"));
ok($objSv->lt("\x{153}", $ocrc));

# 28

ok($objSv->eq("\x{111}", "\x{110}"));
ok($objSv->eq($eth,  $ETH));
ok($objSv->eq('th',  $thrn));
ok($objSv->eq($thrn, 'TH'));
ok($objSv->eq('TH',  $THRN));
ok($objSv->eq('w',   'W'));
ok($objSv->eq($uuml, $Uuml));
ok($objSv->eq("\x{171}", "\x{170}"));
ok($objSv->eq($arng, $Arng));
ok($objSv->eq($auml, $Auml));
ok($objSv->eq($ae,   $AE));
ok($objSv->eq($AE, "\x{1D2D}"));
ok($objSv->eq("\x{119}", "\x{118}"));
ok($objSv->eq($ouml, $Ouml));
ok($objSv->eq($ostk, $Ostk));
ok($objSv->eq("\x{151}", "\x{150}"));
ok($objSv->eq("\x{153}", "\x{152}"));
ok($objSv->eq($ocrc, $Ocrc));

# 46

$objSv->change(level => 3);

ok($objSv->lt("\x{111}", "\x{110}"));
ok($objSv->lt($eth,  $ETH));
ok($objSv->lt('th',  $thrn));
ok($objSv->lt($thrn, 'TH'));
ok($objSv->lt('TH',  $THRN));
ok($objSv->lt('w',   'W'));
ok($objSv->lt($uuml, $Uuml));
ok($objSv->lt("\x{171}", "\x{170}"));
ok($objSv->lt($arng, $Arng));
ok($objSv->lt($auml, $Auml));
ok($objSv->lt($ae,   $AE));
ok($objSv->lt($AE, "\x{1D2D}"));
ok($objSv->lt("\x{119}", "\x{118}"));
ok($objSv->lt($ouml, $Ouml));
ok($objSv->lt($ostk, $Ostk));
ok($objSv->lt("\x{151}", "\x{150}"));
ok($objSv->lt("\x{153}", "\x{152}"));
ok($objSv->lt($ocrc, $Ocrc));

# 64

ok($objSv->eq("d\x{335}", "\x{111}"));
ok($objSv->eq("D\x{335}", "\x{110}"));
ok($objSv->eq("u\x{308}", $uuml));
ok($objSv->eq("U\x{308}", $Uuml));
ok($objSv->eq("u\x{30B}", "\x{171}"));
ok($objSv->eq("U\x{30B}", "\x{170}"));
ok($objSv->eq("a\x{30A}", $arng));
ok($objSv->eq("A\x{30A}", $Arng));
ok($objSv->eq("a\x{308}", $auml));
ok($objSv->eq("A\x{308}", $Auml));
ok($objSv->eq("e\x{328}", "\x{119}"));
ok($objSv->eq("E\x{328}", "\x{118}"));
ok($objSv->eq("o\x{308}", $ouml));
ok($objSv->eq("O\x{308}", $Ouml));
ok($objSv->eq("o\x{338}", $ostk));
ok($objSv->eq("O\x{338}", $Ostk));
ok($objSv->eq("o\x{30B}", "\x{151}"));
ok($objSv->eq("O\x{30B}", "\x{150}"));
ok($objSv->eq("o\x{302}", $ocrc));
ok($objSv->eq("O\x{302}", $Ocrc));

# 84

ok($objSv->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objSv->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objSv->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objSv->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objSv->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objSv->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objSv->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objSv->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objSv->eq("A\x{30A}", "\x{212B}"));
ok($objSv->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objSv->eq("A\x{30A}\x{301}", "\x{1FA}"));
ok($objSv->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objSv->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objSv->eq("\x{1FD}", "$ae\x{301}"));
ok($objSv->eq("\x{1FC}", "$AE\x{301}"));
ok($objSv->eq("\x{1E3}", "$ae\x{304}"));
ok($objSv->eq("\x{1E2}", "$AE\x{304}"));
ok($objSv->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objSv->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objSv->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objSv->eq("O\x{338}\x{301}", "\x{1FE}"));

# 105

ok($objSv->eq("o\x{302}\x{300}", "\x{1ED3}"));
ok($objSv->eq("O\x{302}\x{300}", "\x{1ED2}"));
ok($objSv->eq("o\x{302}\x{301}", "\x{1ED1}"));
ok($objSv->eq("O\x{302}\x{301}", "\x{1ED0}"));
ok($objSv->eq("o\x{302}\x{303}", "\x{1ED7}"));
ok($objSv->eq("O\x{302}\x{303}", "\x{1ED6}"));
ok($objSv->eq("o\x{302}\x{309}", "\x{1ED5}"));
ok($objSv->eq("O\x{302}\x{309}", "\x{1ED4}"));
ok($objSv->eq("o\x{302}\x{323}", "\x{1ED9}"));
ok($objSv->eq("O\x{302}\x{323}", "\x{1ED8}"));

# 115

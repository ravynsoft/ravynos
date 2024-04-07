
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

my $objSvReform = Unicode::Collate::Locale->
    new(locale => 'SV-reform', normalization => undef);

ok($objSvReform->getlocale, 'sv__reformed');

$objSvReform->change(level => 1);

ok($objSvReform->lt('z', $arng));
ok($objSvReform->lt($arng, $auml));
ok($objSvReform->lt($auml, $ouml));
ok($objSvReform->lt($ouml, "\x{1C0}"));

# 6

ok($objSvReform->lt('v', 'w'));
ok($objSvReform->gt('x', 'w'));

ok($objSvReform->eq('d', "\x{111}"));
ok($objSvReform->eq("\x{111}", $eth));
ok($objSvReform->eq('y', $uuml));
ok($objSvReform->eq($uuml, "\x{171}"));

ok($objSvReform->eq($auml, $ae));
ok($objSvReform->eq($ae, "\x{119}"));
ok($objSvReform->eq($ouml, $ostk));
ok($objSvReform->eq($ostk, "\x{151}"));
ok($objSvReform->eq("\x{151}", "\x{153}"));
ok($objSvReform->eq("\x{153}", $ocrc));

# 18

$objSvReform->change(level => 2);

ok($objSvReform->lt('d', "\x{111}"));
ok($objSvReform->lt("\x{111}", $eth));
ok($objSvReform->lt('y', $uuml));
ok($objSvReform->lt($uuml, "\x{171}"));

ok($objSvReform->lt($auml, $ae));
ok($objSvReform->lt($ae, "\x{119}"));
ok($objSvReform->lt($ouml, $ostk));
ok($objSvReform->lt($ostk, "\x{151}"));
ok($objSvReform->lt("\x{151}", "\x{153}"));
ok($objSvReform->lt("\x{153}", $ocrc));

# 28

ok($objSvReform->eq("\x{111}", "\x{110}"));
ok($objSvReform->eq($eth,  $ETH));
ok($objSvReform->eq('th',  $thrn));
ok($objSvReform->eq($thrn, 'TH'));
ok($objSvReform->eq('TH',  $THRN));
ok($objSvReform->eq('w',   'W'));
ok($objSvReform->eq($uuml, $Uuml));
ok($objSvReform->eq("\x{171}", "\x{170}"));
ok($objSvReform->eq($arng, $Arng));
ok($objSvReform->eq($auml, $Auml));
ok($objSvReform->eq($ae,   $AE));
ok($objSvReform->eq($AE, "\x{1D2D}"));
ok($objSvReform->eq("\x{119}", "\x{118}"));
ok($objSvReform->eq($ouml, $Ouml));
ok($objSvReform->eq($ostk, $Ostk));
ok($objSvReform->eq("\x{151}", "\x{150}"));
ok($objSvReform->eq("\x{153}", "\x{152}"));
ok($objSvReform->eq($ocrc, $Ocrc));

# 46

$objSvReform->change(level => 3);

ok($objSvReform->lt("\x{111}", "\x{110}"));
ok($objSvReform->lt($eth,  $ETH));
ok($objSvReform->lt('th',  $thrn));
ok($objSvReform->lt($thrn, 'TH'));
ok($objSvReform->lt('TH',  $THRN));
ok($objSvReform->lt('w',   'W'));
ok($objSvReform->lt($uuml, $Uuml));
ok($objSvReform->lt("\x{171}", "\x{170}"));
ok($objSvReform->lt($arng, $Arng));
ok($objSvReform->lt($auml, $Auml));
ok($objSvReform->lt($ae,   $AE));
ok($objSvReform->lt($AE, "\x{1D2D}"));
ok($objSvReform->lt("\x{119}", "\x{118}"));
ok($objSvReform->lt($ouml, $Ouml));
ok($objSvReform->lt($ostk, $Ostk));
ok($objSvReform->lt("\x{151}", "\x{150}"));
ok($objSvReform->lt("\x{153}", "\x{152}"));
ok($objSvReform->lt($ocrc, $Ocrc));

# 64

ok($objSvReform->eq("d\x{335}", "\x{111}"));
ok($objSvReform->eq("D\x{335}", "\x{110}"));
ok($objSvReform->eq("u\x{308}", $uuml));
ok($objSvReform->eq("U\x{308}", $Uuml));
ok($objSvReform->eq("u\x{30B}", "\x{171}"));
ok($objSvReform->eq("U\x{30B}", "\x{170}"));
ok($objSvReform->eq("a\x{30A}", $arng));
ok($objSvReform->eq("A\x{30A}", $Arng));
ok($objSvReform->eq("a\x{308}", $auml));
ok($objSvReform->eq("A\x{308}", $Auml));
ok($objSvReform->eq("e\x{328}", "\x{119}"));
ok($objSvReform->eq("E\x{328}", "\x{118}"));
ok($objSvReform->eq("o\x{308}", $ouml));
ok($objSvReform->eq("O\x{308}", $Ouml));
ok($objSvReform->eq("o\x{338}", $ostk));
ok($objSvReform->eq("O\x{338}", $Ostk));
ok($objSvReform->eq("o\x{30B}", "\x{151}"));
ok($objSvReform->eq("O\x{30B}", "\x{150}"));
ok($objSvReform->eq("o\x{302}", $ocrc));
ok($objSvReform->eq("O\x{302}", $Ocrc));

# 84

ok($objSvReform->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objSvReform->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objSvReform->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objSvReform->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objSvReform->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objSvReform->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objSvReform->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objSvReform->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objSvReform->eq("A\x{30A}", "\x{212B}"));
ok($objSvReform->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objSvReform->eq("A\x{30A}\x{301}", "\x{1FA}"));
ok($objSvReform->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objSvReform->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objSvReform->eq("\x{1FD}", "$ae\x{301}"));
ok($objSvReform->eq("\x{1FC}", "$AE\x{301}"));
ok($objSvReform->eq("\x{1E3}", "$ae\x{304}"));
ok($objSvReform->eq("\x{1E2}", "$AE\x{304}"));
ok($objSvReform->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objSvReform->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objSvReform->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objSvReform->eq("O\x{338}\x{301}", "\x{1FE}"));

# 105

ok($objSvReform->eq("o\x{302}\x{300}", "\x{1ED3}"));
ok($objSvReform->eq("O\x{302}\x{300}", "\x{1ED2}"));
ok($objSvReform->eq("o\x{302}\x{301}", "\x{1ED1}"));
ok($objSvReform->eq("O\x{302}\x{301}", "\x{1ED0}"));
ok($objSvReform->eq("o\x{302}\x{303}", "\x{1ED7}"));
ok($objSvReform->eq("O\x{302}\x{303}", "\x{1ED6}"));
ok($objSvReform->eq("o\x{302}\x{309}", "\x{1ED5}"));
ok($objSvReform->eq("O\x{302}\x{309}", "\x{1ED4}"));
ok($objSvReform->eq("o\x{302}\x{323}", "\x{1ED9}"));
ok($objSvReform->eq("O\x{302}\x{323}", "\x{1ED8}"));

# 115

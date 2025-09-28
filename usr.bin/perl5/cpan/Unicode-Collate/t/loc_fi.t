
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..83\n"; }
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

my $objFi = Unicode::Collate::Locale->
    new(locale => 'FI', normalization => undef);

ok($objFi->getlocale, 'fi');

$objFi->change(level => 1);

ok($objFi->lt('z', $arng));
ok($objFi->lt($arng, $auml));
ok($objFi->lt($auml, $ouml));
ok($objFi->lt($ouml, "\x{1C0}"));

# 6

ok($objFi->eq("d\x{335}", "\x{111}"));
ok($objFi->eq("g\x{335}", "\x{1E5}"));
ok($objFi->eq("n\x{335}", "\x{14B}"));
ok($objFi->eq("t\x{335}", "\x{167}"));
ok($objFi->eq("z\x{335}", "\x{292}"));
ok($objFi->eq('v', 'w'));
ok($objFi->eq('y', $uuml));
ok($objFi->eq($auml, $ae));
ok($objFi->eq($ouml, $ostk));

# 15

$objFi->change(level => 2);

ok($objFi->lt("d\x{335}", "\x{111}"));
ok($objFi->lt("g\x{335}", "\x{1E5}"));
ok($objFi->lt("n\x{335}", "\x{14B}"));
ok($objFi->lt("t\x{335}", "\x{167}"));
ok($objFi->lt("z\x{335}", "\x{292}"));
ok($objFi->lt('v', 'w'));
ok($objFi->lt('y', $uuml));
ok($objFi->lt($auml, $ae));
ok($objFi->lt($ouml, $ostk));

# 24

ok($objFi->eq("\x{111}", "\x{110}"));
ok($objFi->eq("\x{1E5}", "\x{1E4}"));
ok($objFi->eq("\x{14B}", "\x{14A}"));
ok($objFi->eq("\x{167}", "\x{166}"));
ok($objFi->eq("\x{292}", "\x{1B7}"));
ok($objFi->eq('w',   'W'));
ok($objFi->eq($uuml, $Uuml));
ok($objFi->eq($arng, $Arng));
ok($objFi->eq($auml, $Auml));
ok($objFi->eq($ae,   $AE));
ok($objFi->eq($AE, "\x{1D2D}"));
ok($objFi->eq($ouml, $Ouml));
ok($objFi->eq($ostk, $Ostk));

# 37

$objFi->change(level => 3);

ok($objFi->lt("\x{111}", "\x{110}"));
ok($objFi->lt("\x{1E5}", "\x{1E4}"));
ok($objFi->lt("\x{14B}", "\x{14A}"));
ok($objFi->lt("\x{167}", "\x{166}"));
ok($objFi->lt("\x{292}", "\x{1B7}"));
ok($objFi->lt('w',   'W'));
ok($objFi->lt($uuml, $Uuml));
ok($objFi->lt($arng, $Arng));
ok($objFi->lt($auml, $Auml));
ok($objFi->lt($ae,   $AE));
ok($objFi->lt($AE, "\x{1D2D}"));
ok($objFi->lt($ouml, $Ouml));
ok($objFi->lt($ostk, $Ostk));

# 50

ok($objFi->eq("u\x{308}", $uuml));
ok($objFi->eq("U\x{308}", $Uuml));
ok($objFi->eq("\x{1EF}", "\x{292}\x{30C}"));
ok($objFi->eq("\x{1EE}", "\x{1B7}\x{30C}"));
ok($objFi->eq("a\x{30A}", $arng));
ok($objFi->eq("A\x{30A}", $Arng));
ok($objFi->eq("a\x{308}", $auml));
ok($objFi->eq("A\x{308}", $Auml));
ok($objFi->eq("o\x{308}", $ouml));
ok($objFi->eq("O\x{308}", $Ouml));
ok($objFi->eq("o\x{338}", $ostk));
ok($objFi->eq("O\x{338}", $Ostk));

# 62

ok($objFi->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objFi->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objFi->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objFi->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objFi->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objFi->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objFi->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objFi->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objFi->eq("A\x{30A}", "\x{212B}"));
ok($objFi->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objFi->eq("A\x{30A}\x{301}", "\x{1FA}"));
ok($objFi->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objFi->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objFi->eq("\x{1FD}", "$ae\x{301}"));
ok($objFi->eq("\x{1FC}", "$AE\x{301}"));
ok($objFi->eq("\x{1E3}", "$ae\x{304}"));
ok($objFi->eq("\x{1E2}", "$AE\x{304}"));
ok($objFi->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objFi->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objFi->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objFi->eq("O\x{338}\x{301}", "\x{1FE}"));

# 83

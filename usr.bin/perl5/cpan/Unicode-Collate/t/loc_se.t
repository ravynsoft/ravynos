
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..217\n"; }
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

my $objSe = Unicode::Collate::Locale->
    new(locale => 'SE', normalization => undef);

my $eth  = _pack_U(0xF0);
my $ETH  = _pack_U(0xD0);
my $thrn = _pack_U(0xFE);
my $THRN = _pack_U(0xDE);
my $ae   = _pack_U(0xE6);
my $AE   = _pack_U(0xC6);

ok($objSe->getlocale, 'se');

$objSe->change(level => 1);

ok($objSe->lt("a", "a\x{301}"));
ok($objSe->gt("b", "a\x{301}"));
ok($objSe->lt("c", "c\x{30C}"));
ok($objSe->lt("c\x{30C}", "\x{292}"));
ok($objSe->lt( "\x{292}", "\x{1EF}"));
ok($objSe->gt("d", "\x{1EF}"));
ok($objSe->lt("d", "d\x{335}"));
ok($objSe->gt("e", "d\x{335}"));
ok($objSe->lt("g", "g\x{30C}"));
ok($objSe->lt("g\x{30C}", "\x{1E5}"));
ok($objSe->gt("h", "\x{1E5}"));
ok($objSe->lt("k", "k\x{30C}"));
ok($objSe->gt("l", "k\x{30C}"));
ok($objSe->lt("n", "\x{14B}"));
ok($objSe->gt("o", "\x{14B}"));
ok($objSe->lt("s", "s\x{30C}"));
ok($objSe->gt("t", "s\x{30C}"));
ok($objSe->lt("t",  "\x{167}"));
ok($objSe->gt("u",  "\x{167}"));
ok($objSe->lt("z", "z\x{30C}"));
ok($objSe->lt("z\x{30C}", "o\x{338}"));
ok($objSe->lt("o\x{338}", $ae));
ok($objSe->lt($ae, "a\x{30A}"));
ok($objSe->lt("a\x{30A}", "a\x{308}"));
ok($objSe->lt("a\x{308}", "o\x{308}"));
ok($objSe->lt("o\x{308}", "\x{1C0}"));

# 28

ok($objSe->eq("d\x{335}", $eth));
ok($objSe->eq( "\x{14B}", "n\x{301}"));
ok($objSe->eq("n\x{301}", "n\x{303}"));
ok($objSe->eq( "\x{167}", $thrn));
ok($objSe->eq("y", "u\x{308}"));
ok($objSe->eq("u\x{308}", "u\x{30B}"));
ok($objSe->eq("o\x{338}",  "\x{153}"));
ok($objSe->eq("a\x{30A}", "a\x{307}"));
ok($objSe->eq("a\x{308}", "a\x{303}"));
ok($objSe->eq("o\x{308}", "o\x{30B}"));
ok($objSe->eq("o\x{30B}", "o\x{303}"));
ok($objSe->eq("o\x{303}", "o\x{302}"));
ok($objSe->eq("o\x{302}", "o\x{328}"));

# 41

$objSe->change(level => 2);

ok($objSe->lt("d\x{335}", $eth));
ok($objSe->lt( "\x{14B}", "n\x{301}"));
ok($objSe->lt("n\x{301}", "n\x{303}"));
ok($objSe->lt( "\x{167}", $thrn));
ok($objSe->lt("y", "u\x{308}"));
ok($objSe->lt("u\x{308}", "u\x{30B}"));
ok($objSe->lt("o\x{338}",  "\x{153}"));
ok($objSe->lt("a\x{30A}", "a\x{307}"));
ok($objSe->lt("a\x{308}", "a\x{303}"));
ok($objSe->lt("o\x{308}", "o\x{30B}"));
ok($objSe->lt("o\x{30B}", "o\x{303}"));
ok($objSe->lt("o\x{303}", "o\x{302}"));
ok($objSe->lt("o\x{302}", "o\x{328}"));

# 54

ok($objSe->eq("a\x{301}", "A\x{301}"));
ok($objSe->eq("c\x{30C}", "C\x{30C}"));
ok($objSe->eq( "\x{292}",  "\x{1B7}"));
ok($objSe->eq( "\x{1EF}",  "\x{1EE}"));
ok($objSe->eq("d\x{335}", "D\x{335}"));
ok($objSe->eq($eth, $ETH));
ok($objSe->eq("g\x{30C}", "G\x{30C}"));
ok($objSe->eq( "\x{1E5}",  "\x{1E4}"));
ok($objSe->eq("k\x{30C}", "K\x{30C}"));
ok($objSe->eq( "\x{14B}",  "\x{14A}"));
ok($objSe->eq("n\x{301}", "N\x{301}"));
ok($objSe->eq("n\x{303}", "N\x{303}"));
ok($objSe->eq("s\x{30C}", "S\x{30C}"));
ok($objSe->eq( "\x{167}",  "\x{166}"));
ok($objSe->eq($thrn, $THRN));
ok($objSe->eq("u\x{308}", "U\x{308}"));
ok($objSe->eq("u\x{30B}", "U\x{30B}"));
ok($objSe->eq("z\x{30C}", "Z\x{30C}"));
ok($objSe->eq("o\x{338}", "O\x{338}"));
ok($objSe->eq( "\x{153}",  "\x{152}"));
ok($objSe->eq($ae, $AE));
ok($objSe->eq($AE, "\x{1D2D}"));
ok($objSe->eq("a\x{30A}", "A\x{30A}"));
ok($objSe->eq("a\x{307}", "A\x{307}"));
ok($objSe->eq("a\x{308}", "A\x{308}"));
ok($objSe->eq("a\x{303}", "A\x{303}"));
ok($objSe->eq("o\x{308}", "O\x{308}"));
ok($objSe->eq("o\x{30B}", "O\x{30B}"));
ok($objSe->eq("o\x{303}", "O\x{303}"));
ok($objSe->eq("o\x{302}", "O\x{302}"));
ok($objSe->eq("o\x{328}", "O\x{328}"));

# 85

$objSe->change(level => 3);

ok($objSe->lt("a\x{301}", "A\x{301}"));
ok($objSe->lt("c\x{30C}", "C\x{30C}"));
ok($objSe->lt( "\x{292}",  "\x{1B7}"));
ok($objSe->lt( "\x{1EF}",  "\x{1EE}"));
ok($objSe->lt("d\x{335}", "D\x{335}"));
ok($objSe->lt($eth, $ETH));
ok($objSe->lt("g\x{30C}", "G\x{30C}"));
ok($objSe->lt( "\x{1E5}",  "\x{1E4}"));
ok($objSe->lt("k\x{30C}", "K\x{30C}"));
ok($objSe->lt( "\x{14B}",  "\x{14A}"));
ok($objSe->lt("n\x{301}", "N\x{301}"));
ok($objSe->lt("n\x{303}", "N\x{303}"));
ok($objSe->lt("s\x{30C}", "S\x{30C}"));
ok($objSe->lt( "\x{167}",  "\x{166}"));
ok($objSe->lt($thrn, $THRN));
ok($objSe->lt("u\x{308}", "U\x{308}"));
ok($objSe->lt("u\x{30B}", "U\x{30B}"));
ok($objSe->lt("z\x{30C}", "Z\x{30C}"));
ok($objSe->lt("o\x{338}", "O\x{338}"));
ok($objSe->lt( "\x{153}",  "\x{152}"));
ok($objSe->lt($ae, $AE));
ok($objSe->lt($AE, "\x{1D2D}"));
ok($objSe->lt("a\x{30A}", "A\x{30A}"));
ok($objSe->lt("a\x{307}", "A\x{307}"));
ok($objSe->lt("a\x{308}", "A\x{308}"));
ok($objSe->lt("a\x{303}", "A\x{303}"));
ok($objSe->lt("o\x{308}", "O\x{308}"));
ok($objSe->lt("o\x{30B}", "O\x{30B}"));
ok($objSe->lt("o\x{303}", "O\x{303}"));
ok($objSe->lt("o\x{302}", "O\x{302}"));
ok($objSe->lt("o\x{328}", "O\x{328}"));

# 116

ok($objSe->eq("a\x{301}", _pack_U(0xE1)));
ok($objSe->eq("a\x{341}", _pack_U(0xE1)));
ok($objSe->eq("A\x{301}", _pack_U(0xC1)));
ok($objSe->eq("A\x{341}", _pack_U(0xC1)));
ok($objSe->eq("c\x{30C}", "\x{10D}"));
ok($objSe->eq("C\x{30C}", "\x{10C}"));
ok($objSe->eq("\x{1EF}", "\x{292}\x{30C}"));
ok($objSe->eq("\x{1EE}", "\x{1B7}\x{30C}"));
ok($objSe->eq("d\x{335}", "\x{111}"));
ok($objSe->eq("D\x{335}", "\x{110}"));
ok($objSe->eq("g\x{30C}", "\x{1E7}"));
ok($objSe->eq("G\x{30C}", "\x{1E6}"));
ok($objSe->eq("k\x{30C}", "\x{1E9}"));
ok($objSe->eq("K\x{30C}", "\x{1E8}"));
ok($objSe->eq("n\x{301}", "\x{144}"));
ok($objSe->eq("n\x{341}", "\x{144}"));
ok($objSe->eq("N\x{301}", "\x{143}"));
ok($objSe->eq("N\x{341}", "\x{143}"));
ok($objSe->eq("n\x{303}", _pack_U(0xF1)));
ok($objSe->eq("N\x{303}", _pack_U(0xD1)));
ok($objSe->eq("s\x{30C}", "\x{161}"));
ok($objSe->eq("S\x{30C}", "\x{160}"));
ok($objSe->eq("u\x{308}", _pack_U(0xFC)));
ok($objSe->eq("U\x{308}", _pack_U(0xDC)));
ok($objSe->eq("u\x{30B}", "\x{171}"));
ok($objSe->eq("U\x{30B}", "\x{170}"));

# 142

ok($objSe->eq("z\x{30C}", "\x{17E}"));
ok($objSe->eq("Z\x{30C}", "\x{17D}"));
ok($objSe->eq("o\x{338}", _pack_U(0xF8)));
ok($objSe->eq("O\x{338}", _pack_U(0xD8)));
ok($objSe->eq("a\x{30A}", _pack_U(0xE5)));
ok($objSe->eq("A\x{30A}", _pack_U(0xC5)));
ok($objSe->eq("a\x{307}", "\x{227}"));
ok($objSe->eq("A\x{307}", "\x{226}"));
ok($objSe->eq("a\x{308}", _pack_U(0xE4)));
ok($objSe->eq("A\x{308}", _pack_U(0xC4)));
ok($objSe->eq("a\x{303}", _pack_U(0xE3)));
ok($objSe->eq("A\x{303}", _pack_U(0xC3)));
ok($objSe->eq("o\x{308}", _pack_U(0xF6)));
ok($objSe->eq("O\x{308}", _pack_U(0xD6)));
ok($objSe->eq("o\x{30B}", "\x{151}"));
ok($objSe->eq("O\x{30B}", "\x{150}"));
ok($objSe->eq("o\x{303}", _pack_U(0xF5)));
ok($objSe->eq("O\x{303}", _pack_U(0xD5)));
ok($objSe->eq("o\x{302}", _pack_U(0xF4)));
ok($objSe->eq("O\x{302}", _pack_U(0xD4)));
ok($objSe->eq("o\x{328}", "\x{1EB}"));
ok($objSe->eq("O\x{328}", "\x{1EA}"));

# 164

ok($objSe->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objSe->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objSe->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objSe->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objSe->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objSe->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objSe->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objSe->eq("U\x{308}\x{30C}", "\x{1D9}"));
ok($objSe->eq("o\x{338}\x{301}", "\x{1FF}"));
ok($objSe->eq("O\x{338}\x{301}", "\x{1FE}"));

ok($objSe->eq("\x{1FD}", "$ae\x{301}"));
ok($objSe->eq("\x{1FC}", "$AE\x{301}"));
ok($objSe->eq("\x{1E3}", "$ae\x{304}"));
ok($objSe->eq("\x{1E2}", "$AE\x{304}"));
ok($objSe->eq("A\x{30A}", "\x{212B}"));
ok($objSe->eq("a\x{30A}\x{301}", "\x{1FB}"));
ok($objSe->eq("A\x{30A}\x{301}", "\x{1FA}"));
ok($objSe->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objSe->eq("A\x{308}\x{304}", "\x{1DE}"));

ok($objSe->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objSe->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objSe->eq("o\x{303}\x{301}", "\x{1E4D}"));
ok($objSe->eq("O\x{303}\x{301}", "\x{1E4C}"));
ok($objSe->eq("o\x{303}\x{304}", "\x{22D}"));
ok($objSe->eq("O\x{303}\x{304}", "\x{22C}"));
ok($objSe->eq("o\x{303}\x{308}", "\x{1E4F}"));
ok($objSe->eq("O\x{303}\x{308}", "\x{1E4E}"));
ok($objSe->eq("o\x{303}\x{31B}", "\x{1EE1}"));
ok($objSe->eq("O\x{303}\x{31B}", "\x{1EE0}"));

ok($objSe->eq("o\x{302}\x{300}", "\x{1ED3}"));
ok($objSe->eq("O\x{302}\x{300}", "\x{1ED2}"));
ok($objSe->eq("o\x{302}\x{301}", "\x{1ED1}"));
ok($objSe->eq("O\x{302}\x{301}", "\x{1ED0}"));
ok($objSe->eq("o\x{302}\x{303}", "\x{1ED7}"));
ok($objSe->eq("O\x{302}\x{303}", "\x{1ED6}"));
ok($objSe->eq("o\x{302}\x{309}", "\x{1ED5}"));
ok($objSe->eq("O\x{302}\x{309}", "\x{1ED4}"));
ok($objSe->eq("o\x{302}\x{323}", "\x{1ED9}"));
ok($objSe->eq("O\x{302}\x{323}", "\x{1ED8}"));

# 203

$objSe->change(upper_before_lower => 1);

ok($objSe->gt("z\x{30C}", "Z\x{30C}"));
ok($objSe->gt("o\x{338}", "O\x{338}"));
ok($objSe->gt( "\x{153}",  "\x{152}"));
ok($objSe->gt($ae, $AE));
ok($objSe->lt($AE, "\x{1D2D}"));
ok($objSe->gt("a\x{30A}", "A\x{30A}"));
ok($objSe->gt("a\x{307}", "A\x{307}"));
ok($objSe->gt("a\x{308}", "A\x{308}"));
ok($objSe->gt("a\x{303}", "A\x{303}"));
ok($objSe->gt("o\x{308}", "O\x{308}"));
ok($objSe->gt("o\x{30B}", "O\x{30B}"));
ok($objSe->gt("o\x{303}", "O\x{303}"));
ok($objSe->gt("o\x{302}", "O\x{302}"));
ok($objSe->gt("o\x{328}", "O\x{328}"));

# 217

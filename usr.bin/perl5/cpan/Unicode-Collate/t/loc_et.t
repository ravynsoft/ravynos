
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..60\n"; }
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

my $objEt = Unicode::Collate::Locale->
    new(locale => 'ET', normalization => undef);

ok($objEt->getlocale, 'et');

$objEt->change(level => 1);

ok($objEt->lt("s", "s\x{30C}"));
ok($objEt->gt("z", "s\x{30C}"));
ok($objEt->lt("z", "z\x{30C}"));
ok($objEt->gt("t", "z\x{30C}"));
ok($objEt->lt("v", "w")); # no tailoring
ok($objEt->lt("w", "o\x{303}"));
ok($objEt->lt("o\x{303}", "a\x{308}"));
ok($objEt->lt("a\x{308}", "o\x{308}"));
ok($objEt->lt("o\x{308}", "u\x{308}"));
ok($objEt->lt("u\x{308}", "x"));

# 12

$objEt->change(level => 2);

ok($objEt->eq("s\x{30C}", "S\x{30C}"));
ok($objEt->eq("z", "Z"));
ok($objEt->eq("z\x{30C}", "Z\x{30C}"));
ok($objEt->eq("w", "W"));
ok($objEt->eq("o\x{303}", "O\x{303}"));
ok($objEt->eq("a\x{308}", "A\x{308}"));
ok($objEt->eq("o\x{308}", "O\x{308}"));
ok($objEt->eq("u\x{308}", "U\x{308}"));

# 20

$objEt->change(level => 3);

ok($objEt->lt("s\x{30C}", "S\x{30C}"));
ok($objEt->lt("z", "Z"));
ok($objEt->lt("z\x{30C}", "Z\x{30C}"));
ok($objEt->lt("w", "W"));
ok($objEt->lt("o\x{303}", "O\x{303}"));
ok($objEt->lt("a\x{308}", "A\x{308}"));
ok($objEt->lt("o\x{308}", "O\x{308}"));
ok($objEt->lt("u\x{308}", "U\x{308}"));

# 28

ok($objEt->eq("s\x{30C}", "\x{161}"));
ok($objEt->eq("S\x{30C}", "\x{160}"));
ok($objEt->eq("z\x{30C}", "\x{17E}"));
ok($objEt->eq("Z\x{30C}", "\x{17D}"));
ok($objEt->eq("o\x{303}", _pack_U(0xF5)));
ok($objEt->eq("O\x{303}", _pack_U(0xD5)));
ok($objEt->eq("a\x{308}", _pack_U(0xE4)));
ok($objEt->eq("A\x{308}", _pack_U(0xC4)));
ok($objEt->eq("o\x{308}", _pack_U(0xF6)));
ok($objEt->eq("O\x{308}", _pack_U(0xD6)));
ok($objEt->eq("u\x{308}", _pack_U(0xFC)));
ok($objEt->eq("U\x{308}", _pack_U(0xDC)));

# 40

ok($objEt->eq("o\x{303}\x{301}", "\x{1E4D}"));
ok($objEt->eq("O\x{303}\x{301}", "\x{1E4C}"));
ok($objEt->eq("o\x{303}\x{304}", "\x{22D}"));
ok($objEt->eq("O\x{303}\x{304}", "\x{22C}"));
ok($objEt->eq("o\x{303}\x{308}", "\x{1E4F}"));
ok($objEt->eq("O\x{303}\x{308}", "\x{1E4E}"));
ok($objEt->eq("o\x{303}\x{31B}", "\x{1EE1}"));
ok($objEt->eq("O\x{303}\x{31B}", "\x{1EE0}"));
ok($objEt->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objEt->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objEt->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objEt->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objEt->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objEt->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objEt->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objEt->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objEt->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objEt->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objEt->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objEt->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 60

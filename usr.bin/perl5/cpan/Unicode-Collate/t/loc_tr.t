
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..57\n"; }
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

my $objTr = Unicode::Collate::Locale->
    new(locale => 'TR', normalization => undef);

ok($objTr->getlocale, 'tr');

$objTr->change(level => 1);

ok($objTr->lt("c", "c\x{327}"));
ok($objTr->gt("d", "c\x{327}"));
ok($objTr->lt("g", "g\x{306}"));
ok($objTr->gt("h", "g\x{306}"));
ok($objTr->lt("h", "I"));
ok($objTr->lt("I", "i"));
ok($objTr->gt("j", "i"));
ok($objTr->lt("o", "o\x{308}"));
ok($objTr->gt("p", "o\x{308}"));
ok($objTr->lt("s", "s\x{327}"));
ok($objTr->gt("t", "s\x{327}"));
ok($objTr->lt("u", "u\x{308}"));
ok($objTr->gt("v", "u\x{308}"));

# 15

$objTr->change(level => 2);

ok($objTr->eq("c\x{327}", "C\x{327}"));
ok($objTr->eq("g\x{306}", "G\x{306}"));
ok($objTr->eq("\x{131}", "I"));
ok($objTr->eq("\x{130}", "i"));
ok($objTr->eq("o\x{308}", "O\x{308}"));
ok($objTr->eq("s\x{327}", "S\x{327}"));
ok($objTr->eq("u\x{308}", "U\x{308}"));

# 22

$objTr->change(level => 3);

ok($objTr->lt("c\x{327}", "C\x{327}"));
ok($objTr->lt("g\x{306}", "G\x{306}"));
ok($objTr->lt("\x{131}", "I"));
ok($objTr->gt("\x{130}", "i"));
ok($objTr->lt("o\x{308}", "O\x{308}"));
ok($objTr->lt("s\x{327}", "S\x{327}"));
ok($objTr->lt("u\x{308}", "U\x{308}"));

# 29

ok($objTr->eq("c\x{327}", _pack_U(0xE7)));
ok($objTr->eq("C\x{327}", _pack_U(0xC7)));
ok($objTr->eq("g\x{306}", "\x{11F}"));
ok($objTr->eq("G\x{306}", "\x{11E}"));
ok($objTr->eq("I\x{300}", _pack_U(0xCC)));
ok($objTr->eq("I\x{301}", _pack_U(0xCD)));
ok($objTr->eq("I\x{302}", _pack_U(0xCE)));
ok($objTr->eq("I\x{308}", _pack_U(0xCF)));
ok($objTr->eq("I\x{304}", "\x{12A}"));
ok($objTr->eq("I\x{306}", "\x{12C}"));
ok($objTr->eq("I\x{328}", "\x{12E}"));
ok($objTr->eq("I\x{307}", "\x{130}"));

# 41

ok($objTr->eq("o\x{308}", _pack_U(0xF6)));
ok($objTr->eq("O\x{308}", _pack_U(0xD6)));
ok($objTr->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objTr->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objTr->eq("s\x{327}", "\x{15F}"));
ok($objTr->eq("S\x{327}", "\x{15E}"));
ok($objTr->eq("u\x{308}", _pack_U(0xFC)));
ok($objTr->eq("U\x{308}", _pack_U(0xDC)));
ok($objTr->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objTr->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objTr->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objTr->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objTr->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objTr->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objTr->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objTr->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 57


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

my $objHaw = Unicode::Collate::Locale->
    new(locale => 'HAW', normalization => undef);

ok($objHaw->getlocale, 'haw');

$objHaw->change(level => 1);

ok($objHaw->lt('a', 'e'));
ok($objHaw->lt('e', 'i'));
ok($objHaw->lt('i', 'o'));
ok($objHaw->lt('o', 'u'));
ok($objHaw->lt('u', 'h'));
ok($objHaw->lt('h', 'k'));
ok($objHaw->lt('k', 'l'));
ok($objHaw->lt('l', 'm'));
ok($objHaw->lt('m', 'n'));
ok($objHaw->lt('n', 'p'));
ok($objHaw->lt('p', 'w'));
ok($objHaw->lt('w', "\x{2BB}"));
ok($objHaw->lt('b', "\x{2BB}"));

# 15

ok($objHaw->lt('u', 'b'));
ok($objHaw->lt('b', 'h'));
ok($objHaw->gt('x', "\x{2BB}"));

ok($objHaw->lt('aw', 'e'));
ok($objHaw->lt('ew', 'i'));
ok($objHaw->lt('iw', 'o'));
ok($objHaw->lt('ow', 'u'));
ok($objHaw->lt('uw', 'h'));

# 23

$objHaw->change(level => 2);

ok($objHaw->eq('a', 'A'));
ok($objHaw->eq('e', 'E'));
ok($objHaw->eq('i', 'I'));
ok($objHaw->eq('o', 'O'));
ok($objHaw->eq('u', 'U'));
ok($objHaw->eq('h', 'H'));
ok($objHaw->eq('k', 'K'));
ok($objHaw->eq('l', 'L'));
ok($objHaw->eq('m', 'M'));
ok($objHaw->eq('n', 'N'));
ok($objHaw->eq('p', 'P'));
ok($objHaw->eq('w', 'W'));

# 35

$objHaw->change(level => 3);

ok($objHaw->lt('a', 'A'));
ok($objHaw->lt('e', 'E'));
ok($objHaw->lt('i', 'I'));
ok($objHaw->lt('o', 'O'));
ok($objHaw->lt('u', 'U'));
ok($objHaw->lt('h', 'H'));
ok($objHaw->lt('k', 'K'));
ok($objHaw->lt('l', 'L'));
ok($objHaw->lt('m', 'M'));
ok($objHaw->lt('n', 'N'));
ok($objHaw->lt('p', 'P'));
ok($objHaw->lt('w', 'W'));

# 47

ok($objHaw->eq("a\x{304}", "\x{101}"));
ok($objHaw->eq("A\x{304}", "\x{100}"));
ok($objHaw->eq("e\x{304}", "\x{113}"));
ok($objHaw->eq("E\x{304}", "\x{112}"));
ok($objHaw->eq("i\x{304}", "\x{12B}"));
ok($objHaw->eq("I\x{304}", "\x{12A}"));
ok($objHaw->eq("o\x{304}", "\x{14D}"));
ok($objHaw->eq("O\x{304}", "\x{14C}"));
ok($objHaw->eq("u\x{304}", "\x{16B}"));
ok($objHaw->eq("U\x{304}", "\x{16A}"));

# 57

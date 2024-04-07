
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..58\n"; }
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

my $objSk = Unicode::Collate::Locale->
    new(locale => 'SK', normalization => undef);

ok($objSk->getlocale, 'sk');

$objSk->change(level => 1);

ok($objSk->lt("a", "a\x{308}"));
ok($objSk->gt("b", "a\x{308}"));
ok($objSk->lt("c", "c\x{30C}"));
ok($objSk->gt("d", "c\x{30C}"));
ok($objSk->lt("h", "ch"));
ok($objSk->gt("i", "ch"));
ok($objSk->lt("o", "o\x{302}"));
ok($objSk->gt("p", "o\x{302}"));
ok($objSk->lt("r", "r\x{30C}"));
ok($objSk->gt("s", "r\x{30C}"));
ok($objSk->lt("s", "s\x{30C}"));
ok($objSk->gt("t", "s\x{30C}"));
ok($objSk->lt("z", "z\x{30C}"));
ok($objSk->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 16

$objSk->change(level => 2);

ok($objSk->eq("a\x{308}", "A\x{308}"));
ok($objSk->eq("c\x{30C}", "C\x{30C}"));
ok($objSk->eq("o\x{302}", "O\x{302}"));
ok($objSk->eq("r\x{30C}", "R\x{30C}"));
ok($objSk->eq("s\x{30C}", "S\x{30C}"));
ok($objSk->eq("z\x{30C}", "Z\x{30C}"));
ok($objSk->eq("ch", "cH"));
ok($objSk->eq("cH", "Ch"));
ok($objSk->eq("Ch", "CH"));

# 25

$objSk->change(level => 3);

ok($objSk->lt("a\x{308}", "A\x{308}"));
ok($objSk->lt("c\x{30C}", "C\x{30C}"));
ok($objSk->lt("o\x{302}", "O\x{302}"));
ok($objSk->lt("r\x{30C}", "R\x{30C}"));
ok($objSk->lt("s\x{30C}", "S\x{30C}"));
ok($objSk->lt("z\x{30C}", "Z\x{30C}"));
ok($objSk->lt("ch", "cH"));
ok($objSk->lt("cH", "Ch"));
ok($objSk->lt("Ch", "CH"));

# 34

ok($objSk->eq("a\x{308}", _pack_U(0xE4)));
ok($objSk->eq("A\x{308}", _pack_U(0xC4)));
ok($objSk->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objSk->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objSk->eq("c\x{30C}", "\x{10D}"));
ok($objSk->eq("C\x{30C}", "\x{10C}"));
ok($objSk->eq("o\x{302}", _pack_U(0xF4)));
ok($objSk->eq("O\x{302}", _pack_U(0xD4)));
ok($objSk->eq("r\x{30C}", "\x{159}"));
ok($objSk->eq("R\x{30C}", "\x{158}"));
ok($objSk->eq("s\x{30C}", "\x{161}"));
ok($objSk->eq("S\x{30C}", "\x{160}"));
ok($objSk->eq("z\x{30C}", "\x{17E}"));
ok($objSk->eq("Z\x{30C}", "\x{17D}"));

# 48

ok($objSk->eq("o\x{302}\x{300}", "\x{1ED3}"));
ok($objSk->eq("O\x{302}\x{300}", "\x{1ED2}"));
ok($objSk->eq("o\x{302}\x{301}", "\x{1ED1}"));
ok($objSk->eq("O\x{302}\x{301}", "\x{1ED0}"));
ok($objSk->eq("o\x{302}\x{303}", "\x{1ED7}"));
ok($objSk->eq("O\x{302}\x{303}", "\x{1ED6}"));
ok($objSk->eq("o\x{302}\x{309}", "\x{1ED5}"));
ok($objSk->eq("O\x{302}\x{309}", "\x{1ED4}"));
ok($objSk->eq("o\x{302}\x{323}", "\x{1ED9}"));
ok($objSk->eq("O\x{302}\x{323}", "\x{1ED8}"));

# 58

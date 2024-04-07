
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..124\n"; }
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

my $objEe = Unicode::Collate::Locale->
    new(locale => 'EE', normalization => undef);

ok($objEe->getlocale, 'ee');

# 2

$objEe->change(level => 1);

my @prim = (
    "d", "d\x{292}", "dz", "\x{256}", # 5
    "e",  "\x{25B}", "f",  "\x{192}", # 9
    "g", "gz", "gb", "\x{263}", "h",  # 14
    "hz", "x", "i", "kz", "kp", "l",  # 20
    "nz", "ny", "\x{14B}", "o",       # 24
    "\x{254}", "p", "tz", "ts", "u",  # 29
    "v", "\x{28B}", "w",
);

for (my $i = 1; $i < @prim; $i++) {
    ok($objEe->lt($prim[$i-1], $prim[$i]));
}

# 32

ok($objEe->eq("\x{302}",  "\x{30C}"));
ok($objEe->eq("a\x{302}", "a\x{30C}"));
ok($objEe->eq("A\x{302}", "A\x{30C}"));
ok($objEe->eq("e\x{302}", "e\x{30C}"));
ok($objEe->eq("E\x{302}", "E\x{30C}"));
ok($objEe->eq("i\x{302}", "i\x{30C}"));
ok($objEe->eq("I\x{302}", "I\x{30C}"));
ok($objEe->eq("o\x{302}", "o\x{30C}"));
ok($objEe->eq("O\x{302}", "O\x{30C}"));
ok($objEe->eq("u\x{302}", "u\x{30C}"));
ok($objEe->eq("U\x{302}", "U\x{30C}"));
ok($objEe->eq("y\x{302}", "y\x{30C}"));
ok($objEe->eq("Y\x{302}", "Y\x{30C}"));

# 45

$objEe->change(level => 2);

ok($objEe->gt("\x{302}",  "\x{30C}"));
ok($objEe->gt("a\x{302}", "a\x{30C}"));
ok($objEe->gt("A\x{302}", "A\x{30C}"));
ok($objEe->gt("e\x{302}", "e\x{30C}"));
ok($objEe->gt("E\x{302}", "E\x{30C}"));
ok($objEe->gt("i\x{302}", "i\x{30C}"));
ok($objEe->gt("I\x{302}", "I\x{30C}"));
ok($objEe->gt("o\x{302}", "o\x{30C}"));
ok($objEe->gt("O\x{302}", "O\x{30C}"));
ok($objEe->gt("u\x{302}", "u\x{30C}"));
ok($objEe->gt("U\x{302}", "U\x{30C}"));
ok($objEe->gt("y\x{302}", "y\x{30C}"));
ok($objEe->gt("Y\x{302}", "Y\x{30C}"));

# 58

ok($objEe->eq("dz", "Dz"));
ok($objEe->eq("Dz", "DZ"));
ok($objEe->eq("\x{256}", "\x{189}"));
ok($objEe->eq("\x{25B}", "\x{190}"));
ok($objEe->eq("\x{192}", "\x{191}"));
ok($objEe->eq("gb", "Gb"));
ok($objEe->eq("Gb", "GB"));
ok($objEe->eq("\x{263}", "\x{194}"));
ok($objEe->eq("x", "X"));
ok($objEe->eq("kp", "Kp"));
ok($objEe->eq("Kp", "KP"));
ok($objEe->eq("ny", "Ny"));
ok($objEe->eq("Ny", "NY"));
ok($objEe->eq("\x{14B}", "\x{14A}"));
ok($objEe->eq("\x{254}", "\x{186}"));
ok($objEe->eq("ts", "Ts"));
ok($objEe->eq("Ts", "TS"));
ok($objEe->eq("\x{28B}", "\x{1B2}"));

# 76

$objEe->change(level => 3);

ok($objEe->lt("dz", "Dz"));
ok($objEe->lt("Dz", "DZ"));
ok($objEe->lt("\x{256}", "\x{189}"));
ok($objEe->lt("\x{25B}", "\x{190}"));
ok($objEe->lt("\x{192}", "\x{191}"));
ok($objEe->lt("gb", "Gb"));
ok($objEe->lt("Gb", "GB"));
ok($objEe->lt("\x{263}", "\x{194}"));
ok($objEe->lt("x", "X"));
ok($objEe->lt("kp", "Kp"));
ok($objEe->lt("Kp", "KP"));
ok($objEe->lt("ny", "Ny"));
ok($objEe->lt("Ny", "NY"));
ok($objEe->lt("\x{14B}", "\x{14A}"));
ok($objEe->lt("\x{254}", "\x{186}"));
ok($objEe->lt("ts", "Ts"));
ok($objEe->lt("Ts", "TS"));
ok($objEe->lt("\x{28B}", "\x{1B2}"));

# 94

ok($objEe->eq("a\x{302}", _pack_U(0xE2)));
ok($objEe->eq("A\x{302}", _pack_U(0xC2)));
ok($objEe->eq("e\x{302}", _pack_U(0xEA)));
ok($objEe->eq("E\x{302}", _pack_U(0xCA)));
ok($objEe->eq("i\x{302}", _pack_U(0xEE)));
ok($objEe->eq("I\x{302}", _pack_U(0xCE)));
ok($objEe->eq("o\x{302}", _pack_U(0xF4)));
ok($objEe->eq("O\x{302}", _pack_U(0xD4)));
ok($objEe->eq("u\x{302}", _pack_U(0xFB)));
ok($objEe->eq("U\x{302}", _pack_U(0xDB)));
ok($objEe->eq("y\x{302}", "\x{177}"));
ok($objEe->eq("Y\x{302}", "\x{176}"));

# 106

$objEe->change(upper_before_lower => 1);

ok($objEe->gt("dz", "Dz"));
ok($objEe->gt("Dz", "DZ"));
ok($objEe->gt("\x{256}", "\x{189}"));
ok($objEe->gt("\x{25B}", "\x{190}"));
ok($objEe->gt("\x{192}", "\x{191}"));
ok($objEe->gt("gb", "Gb"));
ok($objEe->gt("Gb", "GB"));
ok($objEe->gt("\x{263}", "\x{194}"));
ok($objEe->gt("x", "X"));
ok($objEe->gt("kp", "Kp"));
ok($objEe->gt("Kp", "KP"));
ok($objEe->gt("ny", "Ny"));
ok($objEe->gt("Ny", "NY"));
ok($objEe->gt("\x{14B}", "\x{14A}"));
ok($objEe->gt("\x{254}", "\x{186}"));
ok($objEe->gt("ts", "Ts"));
ok($objEe->gt("Ts", "TS"));
ok($objEe->gt("\x{28B}", "\x{1B2}"));

# 124


BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..37\n"; }
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

my $objLkt = Unicode::Collate::Locale->
    new(locale => 'LKT', normalization => undef);

ok($objLkt->getlocale, 'lkt');

$objLkt->change(level => 1);

ok($objLkt->lt("c", "c\x{30C}"));
ok($objLkt->lt("cz","c\x{30C}"));
ok($objLkt->gt("d", "c\x{30C}"));
ok($objLkt->lt("g", "g\x{30C}"));
ok($objLkt->lt("gz","g\x{30C}"));
ok($objLkt->gt("h", "g\x{30C}"));
ok($objLkt->lt("h", "h\x{30C}"));
ok($objLkt->lt("hz","h\x{30C}"));
ok($objLkt->gt("i", "h\x{30C}"));
ok($objLkt->lt("s", "s\x{30C}"));
ok($objLkt->lt("sz","s\x{30C}"));
ok($objLkt->gt("t", "s\x{30C}"));
ok($objLkt->lt("z", "z\x{30C}"));
ok($objLkt->lt("zz", "z\x{30C}"));
ok($objLkt->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 17

$objLkt->change(level => 2);

ok($objLkt->eq("c\x{30C}", "C\x{30C}"));
ok($objLkt->eq("g\x{30C}", "G\x{30C}"));
ok($objLkt->eq("h\x{30C}", "H\x{30C}"));
ok($objLkt->eq("s\x{30C}", "S\x{30C}"));
ok($objLkt->eq("z\x{30C}", "Z\x{30C}"));

# 22

$objLkt->change(level => 3);

ok($objLkt->lt("c\x{30C}", "C\x{30C}"));
ok($objLkt->lt("g\x{30C}", "G\x{30C}"));
ok($objLkt->lt("h\x{30C}", "H\x{30C}"));
ok($objLkt->lt("s\x{30C}", "S\x{30C}"));
ok($objLkt->lt("z\x{30C}", "Z\x{30C}"));

# 27

ok($objLkt->eq("c\x{30C}", "\x{10D}"));
ok($objLkt->eq("C\x{30C}", "\x{10C}"));
ok($objLkt->eq("g\x{30C}", "\x{1E7}"));
ok($objLkt->eq("G\x{30C}", "\x{1E6}"));
ok($objLkt->eq("h\x{30C}", "\x{21F}"));
ok($objLkt->eq("H\x{30C}", "\x{21E}"));
ok($objLkt->eq("s\x{30C}", "\x{161}"));
ok($objLkt->eq("S\x{30C}", "\x{160}"));
ok($objLkt->eq("z\x{30C}", "\x{17E}"));
ok($objLkt->eq("Z\x{30C}", "\x{17D}"));

# 37

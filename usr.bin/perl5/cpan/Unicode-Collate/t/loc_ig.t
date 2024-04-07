
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..112\n"; }
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

my $objIg = Unicode::Collate::Locale->
    new(locale => 'IG', normalization => undef);

ok($objIg->getlocale, 'ig');

$objIg->change(level => 1);

ok($objIg->lt("b", "ch"));
ok($objIg->lt("bz","ch"));
ok($objIg->gt("c", "ch"));
ok($objIg->lt("g", "gb"));
ok($objIg->lt("gz","gb"));
ok($objIg->lt("gb","gh"));
ok($objIg->lt("gbz","gh"));
ok($objIg->lt("gh","gw"));
ok($objIg->lt("ghz","gw"));
ok($objIg->gt("h", "gw"));
ok($objIg->lt("i", "i\x{323}"));
ok($objIg->gt("j", "i\x{323}"));
ok($objIg->lt("k", "kp"));
ok($objIg->lt("kz","kp"));
ok($objIg->lt("kp","kw"));
ok($objIg->lt("kpz","kw"));
ok($objIg->gt("l", "kw"));
ok($objIg->lt("n", "n\x{307}"));
ok($objIg->lt("nz","n\x{307}"));
ok($objIg->gt("nw","n\x{307}"));
ok($objIg->lt("nw", "ny"));
ok($objIg->lt("nwz","ny"));
ok($objIg->gt("o",  "ny"));
ok($objIg->lt("o", "o\x{323}"));
ok($objIg->gt("p", "o\x{323}"));
ok($objIg->lt("s", "sh"));
ok($objIg->lt("sz","sh"));
ok($objIg->gt("t", "sh"));
ok($objIg->lt("u", "u\x{323}"));
ok($objIg->gt("v", "u\x{323}"));

# 32

$objIg->change(level => 2);

ok($objIg->eq("ch", "Ch"));
ok($objIg->eq("Ch", "CH"));
ok($objIg->eq("gb", "Gb"));
ok($objIg->eq("Gb", "GB"));
ok($objIg->eq("gh", "Gh"));
ok($objIg->eq("Gh", "GH"));
ok($objIg->eq("gw", "Gw"));
ok($objIg->eq("Gw", "GW"));
ok($objIg->eq("i\x{323}", "I\x{323}"));
ok($objIg->eq("kp", "Kp"));
ok($objIg->eq("Kp", "KP"));
ok($objIg->eq("kw", "Kw"));
ok($objIg->eq("Kw", "KW"));
ok($objIg->eq("n\x{307}", "N\x{307}"));
ok($objIg->eq("nw", "Nw"));
ok($objIg->eq("Nw", "NW"));
ok($objIg->eq("ny", "Ny"));
ok($objIg->eq("Ny", "NY"));
ok($objIg->eq("o\x{323}", "O\x{323}"));
ok($objIg->eq("sh", "Sh"));
ok($objIg->eq("Sh", "SH"));
ok($objIg->eq("u\x{323}", "U\x{323}"));

# 54

$objIg->change(level => 3);

ok($objIg->lt("ch", "Ch"));
ok($objIg->lt("Ch", "CH"));
ok($objIg->lt("gb", "Gb"));
ok($objIg->lt("Gb", "GB"));
ok($objIg->lt("gh", "Gh"));
ok($objIg->lt("Gh", "GH"));
ok($objIg->lt("gw", "Gw"));
ok($objIg->lt("Gw", "GW"));
ok($objIg->lt("i\x{323}", "I\x{323}"));
ok($objIg->lt("kp", "Kp"));
ok($objIg->lt("Kp", "KP"));
ok($objIg->lt("kw", "Kw"));
ok($objIg->lt("Kw", "KW"));
ok($objIg->lt("n\x{307}", "N\x{307}"));
ok($objIg->lt("nw", "Nw"));
ok($objIg->lt("Nw", "NW"));
ok($objIg->lt("ny", "Ny"));
ok($objIg->lt("Ny", "NY"));
ok($objIg->lt("o\x{323}", "O\x{323}"));
ok($objIg->lt("sh", "Sh"));
ok($objIg->lt("Sh", "SH"));
ok($objIg->lt("u\x{323}", "U\x{323}"));

# 76

ok($objIg->eq("i\x{323}", "\x{1ECB}"));
ok($objIg->eq("I\x{323}", "\x{1ECA}"));
ok($objIg->eq("n\x{307}", "\x{1E45}"));
ok($objIg->eq("N\x{307}", "\x{1E44}"));
ok($objIg->eq("o\x{323}", "\x{1ECD}"));
ok($objIg->eq("O\x{323}", "\x{1ECC}"));
ok($objIg->eq("u\x{323}", "\x{1EE5}"));
ok($objIg->eq("U\x{323}", "\x{1EE4}"));

ok($objIg->eq("o\x{323}\x{302}", "\x{1ED9}"));
ok($objIg->eq("O\x{323}\x{302}", "\x{1ED8}"));
ok($objIg->eq("o\x{323}\x{31B}", "\x{1EE3}"));
ok($objIg->eq("O\x{323}\x{31B}", "\x{1EE2}"));
ok($objIg->eq("u\x{323}\x{31B}", "\x{1EF1}"));
ok($objIg->eq("U\x{323}\x{31B}", "\x{1EF0}"));

# 90

$objIg->change(upper_before_lower => 1);

ok($objIg->gt("ch", "Ch"));
ok($objIg->gt("Ch", "CH"));
ok($objIg->gt("gb", "Gb"));
ok($objIg->gt("Gb", "GB"));
ok($objIg->gt("gh", "Gh"));
ok($objIg->gt("Gh", "GH"));
ok($objIg->gt("gw", "Gw"));
ok($objIg->gt("Gw", "GW"));
ok($objIg->gt("i\x{323}", "I\x{323}"));
ok($objIg->gt("kp", "Kp"));
ok($objIg->gt("Kp", "KP"));
ok($objIg->gt("kw", "Kw"));
ok($objIg->gt("Kw", "KW"));
ok($objIg->gt("n\x{307}", "N\x{307}"));
ok($objIg->gt("nw", "Nw"));
ok($objIg->gt("Nw", "NW"));
ok($objIg->gt("ny", "Ny"));
ok($objIg->gt("Ny", "NY"));
ok($objIg->gt("o\x{323}", "O\x{323}"));
ok($objIg->gt("sh", "Sh"));
ok($objIg->gt("Sh", "SH"));
ok($objIg->gt("u\x{323}", "U\x{323}"));

# 112

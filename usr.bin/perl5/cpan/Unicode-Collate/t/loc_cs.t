
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..42\n"; }
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

my $objCs = Unicode::Collate::Locale->
    new(locale => 'CS', normalization => undef);

ok($objCs->getlocale, 'cs');

$objCs->change(level => 1);

ok($objCs->lt("c", "c\x{30C}"));
ok($objCs->lt("cz","c\x{30C}"));
ok($objCs->gt("d", "c\x{30C}"));
ok($objCs->lt("h", "ch"));
ok($objCs->lt("hz","ch"));
ok($objCs->gt("i", "ch"));
ok($objCs->lt("r", "r\x{30C}"));
ok($objCs->lt("rz","r\x{30C}"));
ok($objCs->gt("s", "r\x{30C}"));
ok($objCs->lt("s", "s\x{30C}"));
ok($objCs->lt("sz","s\x{30C}"));
ok($objCs->gt("t", "s\x{30C}"));
ok($objCs->lt("z", "z\x{30C}"));
ok($objCs->lt("zz","z\x{30C}"));
ok($objCs->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 17

$objCs->change(level => 2);

ok($objCs->eq("c\x{30C}", "C\x{30C}"));
ok($objCs->eq("r\x{30C}", "R\x{30C}"));
ok($objCs->eq("s\x{30C}", "S\x{30C}"));
ok($objCs->eq("z\x{30C}", "Z\x{30C}"));

ok($objCs->eq("ch", "cH"));
ok($objCs->eq("cH", "Ch"));
ok($objCs->eq("Ch", "CH"));

# 24

$objCs->change(level => 3);

ok($objCs->lt("c\x{30C}", "C\x{30C}"));
ok($objCs->lt("r\x{30C}", "R\x{30C}"));
ok($objCs->lt("s\x{30C}", "S\x{30C}"));
ok($objCs->lt("z\x{30C}", "Z\x{30C}"));

ok($objCs->lt("ch", "cH"));
ok($objCs->lt("cH", "Ch"));
ok($objCs->lt("Ch", "CH"));

# 31

ok($objCs->eq("c\x{30C}", "\x{10D}"));
ok($objCs->eq("C\x{30C}", "\x{10C}"));
ok($objCs->eq("r\x{30C}", "\x{159}"));
ok($objCs->eq("R\x{30C}", "\x{158}"));
ok($objCs->eq("s\x{30C}", "\x{161}"));
ok($objCs->eq("S\x{30C}", "\x{160}"));
ok($objCs->eq("z\x{30C}", "\x{17E}"));
ok($objCs->eq("Z\x{30C}", "\x{17D}"));

# 39

$objCs->change(upper_before_lower => 1);

ok($objCs->gt("ch", "cH"));
ok($objCs->gt("cH", "Ch"));
ok($objCs->gt("Ch", "CH"));

# 42

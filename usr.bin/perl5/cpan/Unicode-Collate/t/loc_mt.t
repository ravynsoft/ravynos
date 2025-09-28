
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..50\n"; }
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

my $objMt = Unicode::Collate::Locale->
    new(locale => 'MT', normalization => undef);

ok($objMt->getlocale, 'mt');

$objMt->change(level => 1);

ok($objMt->lt("b", "c\x{307}"));
ok($objMt->gt("c", "c\x{307}"));
ok($objMt->lt("f", "g\x{307}"));
ok($objMt->gt("g", "g\x{307}"));
ok($objMt->lt("g", "gh\x{335}"));
ok($objMt->gt("h", "gh\x{335}"));
ok($objMt->lt("h", "h\x{335}"));
ok($objMt->gt("i", "h\x{335}"));
ok($objMt->lt("y", "z\x{307}"));
ok($objMt->gt("z", "z\x{307}"));

# 12

$objMt->change(level => 2);

ok($objMt->eq("c\x{307}", "C\x{307}"));
ok($objMt->eq("g\x{307}", "G\x{307}"));
ok($objMt->eq("gh\x{335}","gH\x{335}"));
ok($objMt->eq("gH\x{335}","Gh\x{335}"));
ok($objMt->eq("Gh\x{335}","GH\x{335}"));
ok($objMt->eq("h\x{335}", "H\x{335}"));
ok($objMt->eq("z\x{307}", "Z\x{307}"));

# 19

$objMt->change(level => 3);

ok($objMt->gt("c\x{307}", "C\x{307}"));
ok($objMt->gt("g\x{307}", "G\x{307}"));
ok($objMt->gt("gh\x{335}","gH\x{335}"));
ok($objMt->gt("gH\x{335}","Gh\x{335}"));
ok($objMt->gt("Gh\x{335}","GH\x{335}"));
ok($objMt->gt("h\x{335}", "H\x{335}"));
ok($objMt->gt("z\x{307}", "Z\x{307}"));

# 26

ok($objMt->eq("c\x{307}", "\x{10B}"));
ok($objMt->eq("C\x{307}", "\x{10A}"));
ok($objMt->eq("g\x{307}", "\x{121}"));
ok($objMt->eq("G\x{307}", "\x{120}"));
ok($objMt->eq("gh\x{335}","g\x{127}"));
ok($objMt->eq("gH\x{335}","g\x{126}"));
ok($objMt->eq("Gh\x{335}","G\x{127}"));
ok($objMt->eq("GH\x{335}","G\x{126}"));
ok($objMt->eq("h\x{335}", "\x{127}"));
ok($objMt->eq("H\x{335}", "\x{126}"));
ok($objMt->eq("z\x{307}", "\x{17C}"));
ok($objMt->eq("Z\x{307}", "\x{17B}"));

# 38

ok($objMt->gt("a", "A"));
ok($objMt->gt("b", "B"));
ok($objMt->gt("c", "C"));
ok($objMt->gt("x", "X"));
ok($objMt->gt("y", "Y"));
ok($objMt->gt("z", "Z"));

# 44

$objMt->change(upper_before_lower => 0);

ok($objMt->lt("a", "A"));
ok($objMt->lt("b", "B"));
ok($objMt->lt("c", "C"));
ok($objMt->lt("x", "X"));
ok($objMt->lt("y", "Y"));
ok($objMt->lt("z", "Z"));

# 50

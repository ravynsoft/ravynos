
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..69\n"; }
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

my $objLt = Unicode::Collate::Locale->
    new(locale => 'LT', normalization => undef);

ok($objLt->getlocale, 'lt');

$objLt->change(level => 1);

ok($objLt->lt("c", "c\x{30C}"));
ok($objLt->gt("d", "c\x{30C}"));
ok($objLt->lt("s", "s\x{30C}"));
ok($objLt->gt("t", "s\x{30C}"));
ok($objLt->lt("z", "z\x{30C}"));
ok($objLt->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 8

ok($objLt->eq("a", "a\x{328}"));
ok($objLt->eq("e", "e\x{328}"));
ok($objLt->eq("e\x{328}", "e\x{307}"));
ok($objLt->eq("i", "i\x{328}"));
ok($objLt->eq("y", "i\x{328}"));
ok($objLt->eq("u", "u\x{328}"));
ok($objLt->eq("u\x{328}", "u\x{304}"));

# 15

$objLt->change(level => 2);

ok($objLt->lt("a", "a\x{328}"));
ok($objLt->lt("e", "e\x{328}"));
ok($objLt->lt("e\x{328}", "e\x{307}"));
ok($objLt->lt("i", "i\x{328}"));
ok($objLt->gt("y", "i\x{328}"));
ok($objLt->lt("u", "u\x{328}"));
ok($objLt->lt("u\x{328}", "u\x{304}"));

# 22

ok($objLt->eq("c\x{30C}", "C\x{30C}"));
ok($objLt->eq("s\x{30C}", "S\x{30C}"));
ok($objLt->eq("z\x{30C}", "Z\x{30C}"));

ok($objLt->eq("i\x{307}", "I\x{307}"));
ok($objLt->eq("y", "Y"));

ok($objLt->eq("a\x{328}", "A\x{328}"));
ok($objLt->eq("e\x{328}", "E\x{328}"));
ok($objLt->eq("e\x{307}", "E\x{307}"));
ok($objLt->eq("i\x{328}", "I\x{328}"));
ok($objLt->eq("u\x{328}", "U\x{328}"));
ok($objLt->eq("u\x{304}", "U\x{304}"));

# 33

# according to CLDR tests
ok($objLt->gt("I\x{307}\x{300}", "I\x{300}"));
ok($objLt->gt("I\x{307}\x{301}", "I\x{301}"));
ok($objLt->gt("I\x{307}\x{303}", "I\x{303}"));

# 36

$objLt->change(level => 3);

ok($objLt->lt("c\x{30C}", "C\x{30C}"));
ok($objLt->lt("s\x{30C}", "S\x{30C}"));
ok($objLt->lt("z\x{30C}", "Z\x{30C}"));

ok($objLt->lt("i\x{307}", "I\x{307}"));
ok($objLt->lt("y", "Y"));

ok($objLt->lt("a\x{328}", "A\x{328}"));
ok($objLt->lt("e\x{328}", "E\x{328}"));
ok($objLt->lt("e\x{307}", "E\x{307}"));
ok($objLt->lt("i\x{328}", "I\x{328}"));
ok($objLt->lt("u\x{328}", "U\x{328}"));
ok($objLt->lt("u\x{304}", "U\x{304}"));

# 47

ok($objLt->eq("c\x{30C}", "\x{10D}"));
ok($objLt->eq("C\x{30C}", "\x{10C}"));
ok($objLt->eq("s\x{30C}", "\x{161}"));
ok($objLt->eq("S\x{30C}", "\x{160}"));
ok($objLt->eq("z\x{30C}", "\x{17E}"));
ok($objLt->eq("Z\x{30C}", "\x{17D}"));

ok($objLt->eq("I\x{307}", "\x{130}"));

ok($objLt->eq("a\x{328}", "\x{105}"));
ok($objLt->eq("A\x{328}", "\x{104}"));
ok($objLt->eq("e\x{328}", "\x{119}"));
ok($objLt->eq("E\x{328}", "\x{118}"));
ok($objLt->eq("e\x{307}", "\x{117}"));
ok($objLt->eq("E\x{307}", "\x{116}"));
ok($objLt->eq("i\x{328}", "\x{12F}"));
ok($objLt->eq("I\x{328}", "\x{12E}"));
ok($objLt->eq("u\x{328}", "\x{173}"));
ok($objLt->eq("U\x{328}", "\x{172}"));
ok($objLt->eq("u\x{304}", "\x{16B}"));
ok($objLt->eq("U\x{304}", "\x{16A}"));

# 66

ok($objLt->eq("i\x{307}\x{300}", "i\x{300}"));
ok($objLt->eq("i\x{307}\x{301}", "i\x{301}"));
ok($objLt->eq("i\x{307}\x{303}", "i\x{303}"));

# 69

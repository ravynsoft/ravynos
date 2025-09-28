
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..36\n"; }
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

my $objYo = Unicode::Collate::Locale->
    new(locale => 'YO', normalization => undef);

ok($objYo->getlocale, 'yo');

$objYo->change(level => 1);

ok($objYo->lt("e", "e\x{323}"));
ok($objYo->lt("ez","e\x{323}"));
ok($objYo->gt("f", "e\x{323}"));
ok($objYo->lt("g", "gb"));
ok($objYo->lt("gz","gb"));
ok($objYo->gt("h", "gb"));
ok($objYo->lt("o", "o\x{323}"));
ok($objYo->lt("oz","o\x{323}"));
ok($objYo->gt("p", "o\x{323}"));
ok($objYo->lt("s", "s\x{323}"));
ok($objYo->lt("sz","s\x{323}"));
ok($objYo->gt("t", "s\x{323}"));

# 14

$objYo->change(level => 2);

ok($objYo->eq("e\x{323}", "E\x{323}"));
ok($objYo->eq("gb", "Gb"));
ok($objYo->eq("Gb", "GB"));
ok($objYo->eq("o\x{323}", "O\x{323}"));
ok($objYo->eq("s\x{323}", "S\x{323}"));

# 19

$objYo->change(level => 3);

ok($objYo->lt("e\x{323}", "E\x{323}"));
ok($objYo->lt("gb", "Gb"));
ok($objYo->lt("Gb", "GB"));
ok($objYo->lt("o\x{323}", "O\x{323}"));
ok($objYo->lt("s\x{323}", "S\x{323}"));

# 24

ok($objYo->eq("e\x{323}", "\x{1EB9}"));
ok($objYo->eq("E\x{323}", "\x{1EB8}"));
ok($objYo->eq("o\x{323}", "\x{1ECD}"));
ok($objYo->eq("O\x{323}", "\x{1ECC}"));
ok($objYo->eq("s\x{323}", "\x{1E63}"));
ok($objYo->eq("S\x{323}", "\x{1E62}"));

# 30

ok($objYo->eq("e\x{323}\x{302}", "\x{1EC7}"));
ok($objYo->eq("E\x{323}\x{302}", "\x{1EC6}"));
ok($objYo->eq("o\x{323}\x{302}", "\x{1ED9}"));
ok($objYo->eq("O\x{323}\x{302}", "\x{1ED8}"));
ok($objYo->eq("o\x{323}\x{31B}", "\x{1EE3}"));
ok($objYo->eq("O\x{323}\x{31B}", "\x{1EE2}"));

# 36

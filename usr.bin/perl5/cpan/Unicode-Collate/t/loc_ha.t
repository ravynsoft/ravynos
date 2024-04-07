
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..53\n"; }
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

my $objHa = Unicode::Collate::Locale->
    new(locale => 'HA', normalization => undef);

ok($objHa->getlocale, 'ha');

$objHa->change(level => 1);

ok($objHa->lt("b", "\x{253}"));
ok($objHa->gt("c", "\x{253}"));
ok($objHa->lt("d", "\x{257}"));
ok($objHa->gt("e", "\x{257}"));
ok($objHa->lt("k", "\x{199}"));
ok($objHa->gt("l", "\x{199}"));
ok($objHa->lt("s", "sh"));
ok($objHa->lt("sz","sh"));
ok($objHa->gt("t", "sh"));
ok($objHa->lt("t", "ts"));
ok($objHa->lt("tz","ts"));
ok($objHa->gt("u", "ts"));
ok($objHa->lt("y", "\x{1B4}"));
ok($objHa->gt("z", "\x{1B4}"));

# 16

$objHa->change(level => 2);

ok($objHa->eq("\x{253}", "\x{181}"));
ok($objHa->eq("\x{257}", "\x{18A}"));
ok($objHa->eq("\x{199}", "\x{198}"));
ok($objHa->eq("sh", "Sh"));
ok($objHa->eq("Sh", "SH"));
ok($objHa->eq("ts", "Ts"));
ok($objHa->eq("Ts", "TS"));
ok($objHa->eq("'y", "'Y"));
ok($objHa->eq("\x{1B4}", "\x{1B3}"));

ok($objHa->eq("\x{1B4}", "\x{2BC}y"));
ok($objHa->eq("\x{2BC}y","'y"));
ok($objHa->eq("'y",      "\x{1B3}"));
ok($objHa->eq("\x{1B3}", "\x{2BC}Y"));
ok($objHa->eq("\x{2BC}Y","'Y"));
ok($objHa->eq("'Y",      "\x{1B4}"));

# 31

$objHa->change(level => 3);

ok($objHa->lt("\x{253}", "\x{181}"));
ok($objHa->lt("\x{257}", "\x{18A}"));
ok($objHa->lt("\x{199}", "\x{198}"));
ok($objHa->lt("sh", "Sh"));
ok($objHa->lt("Sh", "SH"));
ok($objHa->lt("ts", "Ts"));
ok($objHa->lt("Ts", "TS"));
ok($objHa->lt("'y", "'Y"));
ok($objHa->lt("\x{1B4}", "\x{1B3}"));

ok($objHa->lt("\x{1B4}", "\x{2BC}y"));
ok($objHa->lt("\x{2BC}y","'y"));
ok($objHa->lt("'y",      "\x{1B3}"));
ok($objHa->lt("\x{1B3}", "\x{2BC}Y"));
ok($objHa->lt("\x{2BC}Y","'Y"));
ok($objHa->gt("'Y",      "\x{1B4}"));

# 46

$objHa->change(upper_before_lower => 1);

ok($objHa->gt("\x{1B4}", "\x{1B3}"));
ok($objHa->lt("\x{1B4}", "\x{2BC}y"));
ok($objHa->lt("\x{2BC}y","'y"));
ok($objHa->gt("'y",      "\x{1B3}"));
ok($objHa->lt("\x{1B3}", "\x{2BC}Y"));
ok($objHa->lt("\x{2BC}Y","'Y"));
ok($objHa->lt("'Y",      "\x{1B4}"));

# 53


BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..128\n"; }
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

my $objSr = Unicode::Collate::Locale->
    new(locale => 'SR', normalization => undef);

ok($objSr->getlocale, 'sr');

$objSr->change(level => 1);

ok($objSr->lt("\x{430}z", "\x{431}"));
ok($objSr->lt("\x{431}z", "\x{432}"));
ok($objSr->lt("\x{432}z", "\x{433}"));
ok($objSr->lt("\x{433}z", "\x{434}"));
ok($objSr->lt("\x{434}z", "\x{452}"));
ok($objSr->lt("\x{452}z", "\x{435}"));
ok($objSr->lt("\x{435}z", "\x{436}"));
ok($objSr->lt("\x{436}z", "\x{437}"));
ok($objSr->lt("\x{437}z", "\x{438}"));
ok($objSr->lt("\x{438}z", "\x{458}"));
ok($objSr->lt("\x{458}z", "\x{43a}"));
ok($objSr->lt("\x{43a}z", "\x{43b}"));
ok($objSr->lt("\x{43b}z", "\x{459}"));
ok($objSr->lt("\x{459}z", "\x{43c}"));
ok($objSr->lt("\x{43c}z", "\x{43d}"));
ok($objSr->lt("\x{43d}z", "\x{45a}"));
ok($objSr->lt("\x{45a}z", "\x{43e}"));
ok($objSr->lt("\x{43e}z", "\x{43f}"));
ok($objSr->lt("\x{43f}z", "\x{440}"));
ok($objSr->lt("\x{440}z", "\x{441}"));
ok($objSr->lt("\x{441}z", "\x{442}"));
ok($objSr->lt("\x{442}z", "\x{45b}"));
ok($objSr->lt("\x{45b}z", "\x{443}"));
ok($objSr->lt("\x{443}z", "\x{444}"));
ok($objSr->lt("\x{444}z", "\x{445}"));
ok($objSr->lt("\x{445}z", "\x{446}"));
ok($objSr->lt("\x{446}z", "\x{447}"));
ok($objSr->lt("\x{447}z", "\x{45f}"));
ok($objSr->lt("\x{45f}z", "\x{448}"));

ok($objSr->lt("\x{410}z", "\x{411}"));
ok($objSr->lt("\x{411}z", "\x{412}"));
ok($objSr->lt("\x{412}z", "\x{413}"));
ok($objSr->lt("\x{413}z", "\x{414}"));
ok($objSr->lt("\x{414}z", "\x{402}"));
ok($objSr->lt("\x{402}z", "\x{415}"));
ok($objSr->lt("\x{415}z", "\x{416}"));
ok($objSr->lt("\x{416}z", "\x{417}"));
ok($objSr->lt("\x{417}z", "\x{418}"));
ok($objSr->lt("\x{418}z", "\x{408}"));
ok($objSr->lt("\x{408}z", "\x{41a}"));
ok($objSr->lt("\x{41a}z", "\x{41b}"));
ok($objSr->lt("\x{41b}z", "\x{409}"));
ok($objSr->lt("\x{409}z", "\x{41c}"));
ok($objSr->lt("\x{41c}z", "\x{41d}"));
ok($objSr->lt("\x{41d}z", "\x{40a}"));
ok($objSr->lt("\x{40a}z", "\x{41e}"));
ok($objSr->lt("\x{41e}z", "\x{41f}"));
ok($objSr->lt("\x{41f}z", "\x{420}"));
ok($objSr->lt("\x{420}z", "\x{421}"));
ok($objSr->lt("\x{421}z", "\x{422}"));
ok($objSr->lt("\x{422}z", "\x{40b}"));
ok($objSr->lt("\x{40b}z", "\x{423}"));
ok($objSr->lt("\x{423}z", "\x{424}"));
ok($objSr->lt("\x{424}z", "\x{425}"));
ok($objSr->lt("\x{425}z", "\x{426}"));
ok($objSr->lt("\x{426}z", "\x{427}"));
ok($objSr->lt("\x{427}z", "\x{40f}"));
ok($objSr->lt("\x{40f}z", "\x{428}"));

# 60

ok($objSr->eq("\x{438}", "\x{439}"));
ok($objSr->eq("\x{418}", "\x{419}"));

# 62

$objSr->change(level => 2);

ok($objSr->lt("\x{438}", "\x{439}"));
ok($objSr->lt("\x{418}", "\x{419}"));

# 64

ok($objSr->eq("\x{430}", "\x{410}"));
ok($objSr->eq("\x{431}", "\x{411}"));
ok($objSr->eq("\x{432}", "\x{412}"));
ok($objSr->eq("\x{433}", "\x{413}"));
ok($objSr->eq("\x{434}", "\x{414}"));
ok($objSr->eq("\x{452}", "\x{402}"));
ok($objSr->eq("\x{435}", "\x{415}"));
ok($objSr->eq("\x{436}", "\x{416}"));
ok($objSr->eq("\x{437}", "\x{417}"));
ok($objSr->eq("\x{438}", "\x{418}"));
ok($objSr->eq("\x{439}", "\x{419}"));
ok($objSr->eq("\x{458}", "\x{408}"));
ok($objSr->eq("\x{43a}", "\x{41a}"));
ok($objSr->eq("\x{43b}", "\x{41b}"));
ok($objSr->eq("\x{459}", "\x{409}"));
ok($objSr->eq("\x{43c}", "\x{41c}"));
ok($objSr->eq("\x{43d}", "\x{41d}"));
ok($objSr->eq("\x{45a}", "\x{40a}"));
ok($objSr->eq("\x{43e}", "\x{41e}"));
ok($objSr->eq("\x{43f}", "\x{41f}"));
ok($objSr->eq("\x{440}", "\x{420}"));
ok($objSr->eq("\x{441}", "\x{421}"));
ok($objSr->eq("\x{442}", "\x{422}"));
ok($objSr->eq("\x{45b}", "\x{40b}"));
ok($objSr->eq("\x{443}", "\x{423}"));
ok($objSr->eq("\x{444}", "\x{424}"));
ok($objSr->eq("\x{445}", "\x{425}"));
ok($objSr->eq("\x{446}", "\x{426}"));
ok($objSr->eq("\x{447}", "\x{427}"));
ok($objSr->eq("\x{45f}", "\x{40f}"));
ok($objSr->eq("\x{448}", "\x{428}"));

# 95

$objSr->change(level => 3);

ok($objSr->lt("\x{430}", "\x{410}"));
ok($objSr->lt("\x{431}", "\x{411}"));
ok($objSr->lt("\x{432}", "\x{412}"));
ok($objSr->lt("\x{433}", "\x{413}"));
ok($objSr->lt("\x{434}", "\x{414}"));
ok($objSr->lt("\x{452}", "\x{402}"));
ok($objSr->lt("\x{435}", "\x{415}"));
ok($objSr->lt("\x{436}", "\x{416}"));
ok($objSr->lt("\x{437}", "\x{417}"));
ok($objSr->lt("\x{438}", "\x{418}"));
ok($objSr->lt("\x{439}", "\x{419}"));
ok($objSr->lt("\x{458}", "\x{408}"));
ok($objSr->lt("\x{43a}", "\x{41a}"));
ok($objSr->lt("\x{43b}", "\x{41b}"));
ok($objSr->lt("\x{459}", "\x{409}"));
ok($objSr->lt("\x{43c}", "\x{41c}"));
ok($objSr->lt("\x{43d}", "\x{41d}"));
ok($objSr->lt("\x{45a}", "\x{40a}"));
ok($objSr->lt("\x{43e}", "\x{41e}"));
ok($objSr->lt("\x{43f}", "\x{41f}"));
ok($objSr->lt("\x{440}", "\x{420}"));
ok($objSr->lt("\x{441}", "\x{421}"));
ok($objSr->lt("\x{442}", "\x{422}"));
ok($objSr->lt("\x{45b}", "\x{40b}"));
ok($objSr->lt("\x{443}", "\x{423}"));
ok($objSr->lt("\x{444}", "\x{424}"));
ok($objSr->lt("\x{445}", "\x{425}"));
ok($objSr->lt("\x{446}", "\x{426}"));
ok($objSr->lt("\x{447}", "\x{427}"));
ok($objSr->lt("\x{45f}", "\x{40f}"));
ok($objSr->lt("\x{448}", "\x{428}"));

# 126

ok($objSr->eq("\x{439}", "\x{438}\x{306}"));
ok($objSr->eq("\x{419}", "\x{418}\x{306}"));

# 128

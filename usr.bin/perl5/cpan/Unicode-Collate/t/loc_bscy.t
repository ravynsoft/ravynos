
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

my $objBsCyrl = Unicode::Collate::Locale->
    new(locale => 'BS-CYRL', normalization => undef);

ok($objBsCyrl->getlocale, 'bs_Cyrl');

$objBsCyrl->change(level => 1);

ok($objBsCyrl->lt("\x{430}z", "\x{431}"));
ok($objBsCyrl->lt("\x{431}z", "\x{432}"));
ok($objBsCyrl->lt("\x{432}z", "\x{433}"));
ok($objBsCyrl->lt("\x{433}z", "\x{434}"));
ok($objBsCyrl->lt("\x{434}z", "\x{452}"));
ok($objBsCyrl->lt("\x{452}z", "\x{435}"));
ok($objBsCyrl->lt("\x{435}z", "\x{436}"));
ok($objBsCyrl->lt("\x{436}z", "\x{437}"));
ok($objBsCyrl->lt("\x{437}z", "\x{438}"));
ok($objBsCyrl->lt("\x{438}z", "\x{458}"));
ok($objBsCyrl->lt("\x{458}z", "\x{43a}"));
ok($objBsCyrl->lt("\x{43a}z", "\x{43b}"));
ok($objBsCyrl->lt("\x{43b}z", "\x{459}"));
ok($objBsCyrl->lt("\x{459}z", "\x{43c}"));
ok($objBsCyrl->lt("\x{43c}z", "\x{43d}"));
ok($objBsCyrl->lt("\x{43d}z", "\x{45a}"));
ok($objBsCyrl->lt("\x{45a}z", "\x{43e}"));
ok($objBsCyrl->lt("\x{43e}z", "\x{43f}"));
ok($objBsCyrl->lt("\x{43f}z", "\x{440}"));
ok($objBsCyrl->lt("\x{440}z", "\x{441}"));
ok($objBsCyrl->lt("\x{441}z", "\x{442}"));
ok($objBsCyrl->lt("\x{442}z", "\x{45b}"));
ok($objBsCyrl->lt("\x{45b}z", "\x{443}"));
ok($objBsCyrl->lt("\x{443}z", "\x{444}"));
ok($objBsCyrl->lt("\x{444}z", "\x{445}"));
ok($objBsCyrl->lt("\x{445}z", "\x{446}"));
ok($objBsCyrl->lt("\x{446}z", "\x{447}"));
ok($objBsCyrl->lt("\x{447}z", "\x{45f}"));
ok($objBsCyrl->lt("\x{45f}z", "\x{448}"));

ok($objBsCyrl->lt("\x{410}z", "\x{411}"));
ok($objBsCyrl->lt("\x{411}z", "\x{412}"));
ok($objBsCyrl->lt("\x{412}z", "\x{413}"));
ok($objBsCyrl->lt("\x{413}z", "\x{414}"));
ok($objBsCyrl->lt("\x{414}z", "\x{402}"));
ok($objBsCyrl->lt("\x{402}z", "\x{415}"));
ok($objBsCyrl->lt("\x{415}z", "\x{416}"));
ok($objBsCyrl->lt("\x{416}z", "\x{417}"));
ok($objBsCyrl->lt("\x{417}z", "\x{418}"));
ok($objBsCyrl->lt("\x{418}z", "\x{408}"));
ok($objBsCyrl->lt("\x{408}z", "\x{41a}"));
ok($objBsCyrl->lt("\x{41a}z", "\x{41b}"));
ok($objBsCyrl->lt("\x{41b}z", "\x{409}"));
ok($objBsCyrl->lt("\x{409}z", "\x{41c}"));
ok($objBsCyrl->lt("\x{41c}z", "\x{41d}"));
ok($objBsCyrl->lt("\x{41d}z", "\x{40a}"));
ok($objBsCyrl->lt("\x{40a}z", "\x{41e}"));
ok($objBsCyrl->lt("\x{41e}z", "\x{41f}"));
ok($objBsCyrl->lt("\x{41f}z", "\x{420}"));
ok($objBsCyrl->lt("\x{420}z", "\x{421}"));
ok($objBsCyrl->lt("\x{421}z", "\x{422}"));
ok($objBsCyrl->lt("\x{422}z", "\x{40b}"));
ok($objBsCyrl->lt("\x{40b}z", "\x{423}"));
ok($objBsCyrl->lt("\x{423}z", "\x{424}"));
ok($objBsCyrl->lt("\x{424}z", "\x{425}"));
ok($objBsCyrl->lt("\x{425}z", "\x{426}"));
ok($objBsCyrl->lt("\x{426}z", "\x{427}"));
ok($objBsCyrl->lt("\x{427}z", "\x{40f}"));
ok($objBsCyrl->lt("\x{40f}z", "\x{428}"));

# 60

ok($objBsCyrl->eq("\x{438}", "\x{439}"));
ok($objBsCyrl->eq("\x{418}", "\x{419}"));

# 62

$objBsCyrl->change(level => 2);

ok($objBsCyrl->lt("\x{438}", "\x{439}"));
ok($objBsCyrl->lt("\x{418}", "\x{419}"));

# 64

ok($objBsCyrl->eq("\x{430}", "\x{410}"));
ok($objBsCyrl->eq("\x{431}", "\x{411}"));
ok($objBsCyrl->eq("\x{432}", "\x{412}"));
ok($objBsCyrl->eq("\x{433}", "\x{413}"));
ok($objBsCyrl->eq("\x{434}", "\x{414}"));
ok($objBsCyrl->eq("\x{452}", "\x{402}"));
ok($objBsCyrl->eq("\x{435}", "\x{415}"));
ok($objBsCyrl->eq("\x{436}", "\x{416}"));
ok($objBsCyrl->eq("\x{437}", "\x{417}"));
ok($objBsCyrl->eq("\x{438}", "\x{418}"));
ok($objBsCyrl->eq("\x{439}", "\x{419}"));
ok($objBsCyrl->eq("\x{458}", "\x{408}"));
ok($objBsCyrl->eq("\x{43a}", "\x{41a}"));
ok($objBsCyrl->eq("\x{43b}", "\x{41b}"));
ok($objBsCyrl->eq("\x{459}", "\x{409}"));
ok($objBsCyrl->eq("\x{43c}", "\x{41c}"));
ok($objBsCyrl->eq("\x{43d}", "\x{41d}"));
ok($objBsCyrl->eq("\x{45a}", "\x{40a}"));
ok($objBsCyrl->eq("\x{43e}", "\x{41e}"));
ok($objBsCyrl->eq("\x{43f}", "\x{41f}"));
ok($objBsCyrl->eq("\x{440}", "\x{420}"));
ok($objBsCyrl->eq("\x{441}", "\x{421}"));
ok($objBsCyrl->eq("\x{442}", "\x{422}"));
ok($objBsCyrl->eq("\x{45b}", "\x{40b}"));
ok($objBsCyrl->eq("\x{443}", "\x{423}"));
ok($objBsCyrl->eq("\x{444}", "\x{424}"));
ok($objBsCyrl->eq("\x{445}", "\x{425}"));
ok($objBsCyrl->eq("\x{446}", "\x{426}"));
ok($objBsCyrl->eq("\x{447}", "\x{427}"));
ok($objBsCyrl->eq("\x{45f}", "\x{40f}"));
ok($objBsCyrl->eq("\x{448}", "\x{428}"));

# 95

$objBsCyrl->change(level => 3);

ok($objBsCyrl->lt("\x{430}", "\x{410}"));
ok($objBsCyrl->lt("\x{431}", "\x{411}"));
ok($objBsCyrl->lt("\x{432}", "\x{412}"));
ok($objBsCyrl->lt("\x{433}", "\x{413}"));
ok($objBsCyrl->lt("\x{434}", "\x{414}"));
ok($objBsCyrl->lt("\x{452}", "\x{402}"));
ok($objBsCyrl->lt("\x{435}", "\x{415}"));
ok($objBsCyrl->lt("\x{436}", "\x{416}"));
ok($objBsCyrl->lt("\x{437}", "\x{417}"));
ok($objBsCyrl->lt("\x{438}", "\x{418}"));
ok($objBsCyrl->lt("\x{439}", "\x{419}"));
ok($objBsCyrl->lt("\x{458}", "\x{408}"));
ok($objBsCyrl->lt("\x{43a}", "\x{41a}"));
ok($objBsCyrl->lt("\x{43b}", "\x{41b}"));
ok($objBsCyrl->lt("\x{459}", "\x{409}"));
ok($objBsCyrl->lt("\x{43c}", "\x{41c}"));
ok($objBsCyrl->lt("\x{43d}", "\x{41d}"));
ok($objBsCyrl->lt("\x{45a}", "\x{40a}"));
ok($objBsCyrl->lt("\x{43e}", "\x{41e}"));
ok($objBsCyrl->lt("\x{43f}", "\x{41f}"));
ok($objBsCyrl->lt("\x{440}", "\x{420}"));
ok($objBsCyrl->lt("\x{441}", "\x{421}"));
ok($objBsCyrl->lt("\x{442}", "\x{422}"));
ok($objBsCyrl->lt("\x{45b}", "\x{40b}"));
ok($objBsCyrl->lt("\x{443}", "\x{423}"));
ok($objBsCyrl->lt("\x{444}", "\x{424}"));
ok($objBsCyrl->lt("\x{445}", "\x{425}"));
ok($objBsCyrl->lt("\x{446}", "\x{426}"));
ok($objBsCyrl->lt("\x{447}", "\x{427}"));
ok($objBsCyrl->lt("\x{45f}", "\x{40f}"));
ok($objBsCyrl->lt("\x{448}", "\x{428}"));

# 126

ok($objBsCyrl->eq("\x{439}", "\x{438}\x{306}"));
ok($objBsCyrl->eq("\x{419}", "\x{418}\x{306}"));

# 128

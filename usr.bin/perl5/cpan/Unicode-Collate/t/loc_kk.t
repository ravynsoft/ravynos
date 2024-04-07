
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..184\n"; }
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

my $objKk = Unicode::Collate::Locale->
    new(locale => 'KK', normalization => undef);

ok($objKk->getlocale, 'kk');

$objKk->change(level => 1);

ok($objKk->lt("\x{430}z", "\x{4d9}"));
ok($objKk->lt("\x{4d9}z", "\x{431}"));
ok($objKk->lt("\x{431}z", "\x{432}"));
ok($objKk->lt("\x{432}z", "\x{433}"));
ok($objKk->lt("\x{433}z", "\x{493}"));
ok($objKk->lt("\x{493}z", "\x{434}"));
ok($objKk->lt("\x{434}z", "\x{435}"));
ok($objKk->lt("\x{435}z", "\x{451}"));
ok($objKk->lt("\x{451}z", "\x{436}"));
ok($objKk->lt("\x{436}z", "\x{437}"));
ok($objKk->lt("\x{437}z", "\x{438}"));
ok($objKk->lt("\x{438}z", "\x{439}"));
ok($objKk->lt("\x{439}z", "\x{43a}"));
ok($objKk->lt("\x{43a}z", "\x{49b}"));
ok($objKk->lt("\x{49b}z", "\x{43b}"));
ok($objKk->lt("\x{43b}z", "\x{43c}"));
ok($objKk->lt("\x{43c}z", "\x{43d}"));
ok($objKk->lt("\x{43d}z", "\x{4a3}"));
ok($objKk->lt("\x{4a3}z", "\x{43e}"));
ok($objKk->lt("\x{43e}z", "\x{4e9}"));
ok($objKk->lt("\x{4e9}z", "\x{43f}"));
ok($objKk->lt("\x{43f}z", "\x{440}"));
ok($objKk->lt("\x{440}z", "\x{441}"));
ok($objKk->lt("\x{441}z", "\x{442}"));
ok($objKk->lt("\x{442}z", "\x{443}"));
ok($objKk->lt("\x{443}z", "\x{4b1}"));
ok($objKk->lt("\x{4b1}z", "\x{4af}"));
ok($objKk->lt("\x{4af}z", "\x{444}"));
ok($objKk->lt("\x{444}z", "\x{445}"));
ok($objKk->lt("\x{445}z", "\x{4bb}"));
ok($objKk->lt("\x{4bb}z", "\x{446}"));
ok($objKk->lt("\x{446}z", "\x{447}"));
ok($objKk->lt("\x{447}z", "\x{448}"));
ok($objKk->lt("\x{448}z", "\x{449}"));
ok($objKk->lt("\x{449}z", "\x{44a}"));
ok($objKk->lt("\x{44a}z", "\x{44b}"));
ok($objKk->lt("\x{44b}z", "\x{456}"));
ok($objKk->lt("\x{456}z", "\x{44c}"));
ok($objKk->lt("\x{44c}z", "\x{44d}"));
ok($objKk->lt("\x{44d}z", "\x{44e}"));
ok($objKk->lt("\x{44e}z", "\x{44f}"));

ok($objKk->lt("\x{410}z", "\x{4d8}"));
ok($objKk->lt("\x{4d8}z", "\x{411}"));
ok($objKk->lt("\x{411}z", "\x{412}"));
ok($objKk->lt("\x{412}z", "\x{413}"));
ok($objKk->lt("\x{413}z", "\x{492}"));
ok($objKk->lt("\x{492}z", "\x{414}"));
ok($objKk->lt("\x{414}z", "\x{415}"));
ok($objKk->lt("\x{415}z", "\x{401}"));
ok($objKk->lt("\x{401}z", "\x{416}"));
ok($objKk->lt("\x{416}z", "\x{417}"));
ok($objKk->lt("\x{417}z", "\x{418}"));
ok($objKk->lt("\x{418}z", "\x{419}"));
ok($objKk->lt("\x{419}z", "\x{41a}"));
ok($objKk->lt("\x{41a}z", "\x{49a}"));
ok($objKk->lt("\x{49a}z", "\x{41b}"));
ok($objKk->lt("\x{41b}z", "\x{41c}"));
ok($objKk->lt("\x{41c}z", "\x{41d}"));
ok($objKk->lt("\x{41d}z", "\x{4a2}"));
ok($objKk->lt("\x{4a2}z", "\x{41e}"));
ok($objKk->lt("\x{41e}z", "\x{4e8}"));
ok($objKk->lt("\x{4e8}z", "\x{41f}"));
ok($objKk->lt("\x{41f}z", "\x{420}"));
ok($objKk->lt("\x{420}z", "\x{421}"));
ok($objKk->lt("\x{421}z", "\x{422}"));
ok($objKk->lt("\x{422}z", "\x{423}"));
ok($objKk->lt("\x{423}z", "\x{4b0}"));
ok($objKk->lt("\x{4b0}z", "\x{4ae}"));
ok($objKk->lt("\x{4ae}z", "\x{424}"));
ok($objKk->lt("\x{424}z", "\x{425}"));
ok($objKk->lt("\x{425}z", "\x{4ba}"));
ok($objKk->lt("\x{4ba}z", "\x{426}"));
ok($objKk->lt("\x{426}z", "\x{427}"));
ok($objKk->lt("\x{427}z", "\x{428}"));
ok($objKk->lt("\x{428}z", "\x{429}"));
ok($objKk->lt("\x{429}z", "\x{42a}"));
ok($objKk->lt("\x{42a}z", "\x{42b}"));
ok($objKk->lt("\x{42b}z", "\x{406}"));
ok($objKk->lt("\x{406}z", "\x{42c}"));
ok($objKk->lt("\x{42c}z", "\x{42d}"));
ok($objKk->lt("\x{42d}z", "\x{42e}"));
ok($objKk->lt("\x{42e}z", "\x{42f}"));

# 84

ok($objKk->eq("\x{456}", "\x{457}"));
ok($objKk->eq("\x{406}", "\x{407}"));

# 86

$objKk->change(level => 2);

ok($objKk->lt("\x{456}", "\x{457}"));
ok($objKk->lt("\x{406}", "\x{407}"));

# 88

ok($objKk->eq("\x{430}", "\x{410}"));
ok($objKk->eq("\x{4d9}", "\x{4d8}"));
ok($objKk->eq("\x{431}", "\x{411}"));
ok($objKk->eq("\x{432}", "\x{412}"));
ok($objKk->eq("\x{433}", "\x{413}"));
ok($objKk->eq("\x{493}", "\x{492}"));
ok($objKk->eq("\x{434}", "\x{414}"));
ok($objKk->eq("\x{435}", "\x{415}"));
ok($objKk->eq("\x{451}", "\x{401}"));
ok($objKk->eq("\x{436}", "\x{416}"));
ok($objKk->eq("\x{437}", "\x{417}"));
ok($objKk->eq("\x{438}", "\x{418}"));
ok($objKk->eq("\x{439}", "\x{419}"));
ok($objKk->eq("\x{43a}", "\x{41a}"));
ok($objKk->eq("\x{49b}", "\x{49a}"));
ok($objKk->eq("\x{43b}", "\x{41b}"));
ok($objKk->eq("\x{43c}", "\x{41c}"));
ok($objKk->eq("\x{43d}", "\x{41d}"));
ok($objKk->eq("\x{4a3}", "\x{4a2}"));
ok($objKk->eq("\x{43e}", "\x{41e}"));
ok($objKk->eq("\x{4e9}", "\x{4e8}"));
ok($objKk->eq("\x{43f}", "\x{41f}"));
ok($objKk->eq("\x{440}", "\x{420}"));
ok($objKk->eq("\x{441}", "\x{421}"));
ok($objKk->eq("\x{442}", "\x{422}"));
ok($objKk->eq("\x{443}", "\x{423}"));
ok($objKk->eq("\x{4b1}", "\x{4b0}"));
ok($objKk->eq("\x{4af}", "\x{4ae}"));
ok($objKk->eq("\x{444}", "\x{424}"));
ok($objKk->eq("\x{445}", "\x{425}"));
ok($objKk->eq("\x{4bb}", "\x{4ba}"));
ok($objKk->eq("\x{446}", "\x{426}"));
ok($objKk->eq("\x{447}", "\x{427}"));
ok($objKk->eq("\x{448}", "\x{428}"));
ok($objKk->eq("\x{449}", "\x{429}"));
ok($objKk->eq("\x{44a}", "\x{42a}"));
ok($objKk->eq("\x{44b}", "\x{42b}"));
ok($objKk->eq("\x{456}", "\x{406}"));
ok($objKk->eq("\x{457}", "\x{407}"));
ok($objKk->eq("\x{457}", "\x{a676}"));
ok($objKk->eq("\x{a676}","\x{407}"));
ok($objKk->eq("\x{44c}", "\x{42c}"));
ok($objKk->eq("\x{44d}", "\x{42d}"));
ok($objKk->eq("\x{44e}", "\x{42e}"));
ok($objKk->eq("\x{44f}", "\x{42f}"));

# 133

$objKk->change(level => 3);

ok($objKk->lt("\x{430}", "\x{410}"));
ok($objKk->lt("\x{4d9}", "\x{4d8}"));
ok($objKk->lt("\x{431}", "\x{411}"));
ok($objKk->lt("\x{432}", "\x{412}"));
ok($objKk->lt("\x{433}", "\x{413}"));
ok($objKk->lt("\x{493}", "\x{492}"));
ok($objKk->lt("\x{434}", "\x{414}"));
ok($objKk->lt("\x{435}", "\x{415}"));
ok($objKk->lt("\x{451}", "\x{401}"));
ok($objKk->lt("\x{436}", "\x{416}"));
ok($objKk->lt("\x{437}", "\x{417}"));
ok($objKk->lt("\x{438}", "\x{418}"));
ok($objKk->lt("\x{439}", "\x{419}"));
ok($objKk->lt("\x{43a}", "\x{41a}"));
ok($objKk->lt("\x{49b}", "\x{49a}"));
ok($objKk->lt("\x{43b}", "\x{41b}"));
ok($objKk->lt("\x{43c}", "\x{41c}"));
ok($objKk->lt("\x{43d}", "\x{41d}"));
ok($objKk->lt("\x{4a3}", "\x{4a2}"));
ok($objKk->lt("\x{43e}", "\x{41e}"));
ok($objKk->lt("\x{4e9}", "\x{4e8}"));
ok($objKk->lt("\x{43f}", "\x{41f}"));
ok($objKk->lt("\x{440}", "\x{420}"));
ok($objKk->lt("\x{441}", "\x{421}"));
ok($objKk->lt("\x{442}", "\x{422}"));
ok($objKk->lt("\x{443}", "\x{423}"));
ok($objKk->lt("\x{4b1}", "\x{4b0}"));
ok($objKk->lt("\x{4af}", "\x{4ae}"));
ok($objKk->lt("\x{444}", "\x{424}"));
ok($objKk->lt("\x{445}", "\x{425}"));
ok($objKk->lt("\x{4bb}", "\x{4ba}"));
ok($objKk->lt("\x{446}", "\x{426}"));
ok($objKk->lt("\x{447}", "\x{427}"));
ok($objKk->lt("\x{448}", "\x{428}"));
ok($objKk->lt("\x{449}", "\x{429}"));
ok($objKk->lt("\x{44a}", "\x{42a}"));
ok($objKk->lt("\x{44b}", "\x{42b}"));
ok($objKk->lt("\x{456}", "\x{406}"));
ok($objKk->lt("\x{457}", "\x{407}"));
ok($objKk->lt("\x{457}", "\x{a676}"));
ok($objKk->lt("\x{a676}","\x{407}"));
ok($objKk->lt("\x{44c}", "\x{42c}"));
ok($objKk->lt("\x{44d}", "\x{42d}"));
ok($objKk->lt("\x{44e}", "\x{42e}"));
ok($objKk->lt("\x{44f}", "\x{42f}"));

# 178

ok($objKk->eq("\x{451}", "\x{435}\x{308}"));
ok($objKk->eq("\x{401}", "\x{415}\x{308}"));
ok($objKk->eq("\x{439}", "\x{438}\x{306}"));
ok($objKk->eq("\x{419}", "\x{418}\x{306}"));
ok($objKk->eq("\x{457}", "\x{456}\x{308}"));
ok($objKk->eq("\x{407}", "\x{406}\x{308}"));

# 184

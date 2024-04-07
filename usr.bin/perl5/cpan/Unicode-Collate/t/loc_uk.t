
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..144\n"; }
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

my $objUk = Unicode::Collate::Locale->
    new(locale => 'UK', normalization => undef);

ok($objUk->getlocale, 'uk');

$objUk->change(level => 1);

ok($objUk->lt("\x{430}z", "\x{431}"));
ok($objUk->lt("\x{431}z", "\x{432}"));
ok($objUk->lt("\x{432}z", "\x{433}"));
ok($objUk->lt("\x{433}z", "\x{491}"));
ok($objUk->lt("\x{491}z", "\x{434}"));
ok($objUk->lt("\x{434}z", "\x{435}"));
ok($objUk->lt("\x{435}z", "\x{454}"));
ok($objUk->lt("\x{454}z", "\x{436}"));
ok($objUk->lt("\x{436}z", "\x{437}"));
ok($objUk->lt("\x{437}z", "\x{438}"));
ok($objUk->lt("\x{438}z", "\x{456}"));
ok($objUk->lt("\x{456}z", "\x{a647}"));
ok($objUk->lt("\x{a647}z","\x{457}"));
ok($objUk->lt("\x{457}z", "\x{439}"));
ok($objUk->lt("\x{439}z", "\x{43a}"));
ok($objUk->lt("\x{43a}z", "\x{43b}"));
ok($objUk->lt("\x{43b}z", "\x{43c}"));
ok($objUk->lt("\x{43c}z", "\x{43d}"));
ok($objUk->lt("\x{43d}z", "\x{43e}"));
ok($objUk->lt("\x{43e}z", "\x{43f}"));
ok($objUk->lt("\x{43f}z", "\x{440}"));
ok($objUk->lt("\x{440}z", "\x{441}"));
ok($objUk->lt("\x{441}z", "\x{442}"));
ok($objUk->lt("\x{442}z", "\x{443}"));
ok($objUk->lt("\x{443}z", "\x{444}"));
ok($objUk->lt("\x{444}z", "\x{445}"));
ok($objUk->lt("\x{445}z", "\x{446}"));
ok($objUk->lt("\x{446}z", "\x{447}"));
ok($objUk->lt("\x{447}z", "\x{448}"));
ok($objUk->lt("\x{448}z", "\x{449}"));
ok($objUk->lt("\x{449}z", "\x{44c}"));
ok($objUk->lt("\x{44c}z", "\x{44e}"));
ok($objUk->lt("\x{44e}z", "\x{44f}"));

ok($objUk->lt("\x{410}z", "\x{411}"));
ok($objUk->lt("\x{411}z", "\x{412}"));
ok($objUk->lt("\x{412}z", "\x{413}"));
ok($objUk->lt("\x{413}z", "\x{490}"));
ok($objUk->lt("\x{490}z", "\x{414}"));
ok($objUk->lt("\x{414}z", "\x{415}"));
ok($objUk->lt("\x{415}z", "\x{404}"));
ok($objUk->lt("\x{404}z", "\x{416}"));
ok($objUk->lt("\x{416}z", "\x{417}"));
ok($objUk->lt("\x{417}z", "\x{418}"));
ok($objUk->lt("\x{418}z", "\x{406}"));
ok($objUk->lt("\x{406}z", "\x{a646}"));
ok($objUk->lt("\x{a646}z","\x{407}"));
ok($objUk->lt("\x{407}z", "\x{419}"));
ok($objUk->lt("\x{419}z", "\x{41a}"));
ok($objUk->lt("\x{41a}z", "\x{41b}"));
ok($objUk->lt("\x{41b}z", "\x{41c}"));
ok($objUk->lt("\x{41c}z", "\x{41d}"));
ok($objUk->lt("\x{41d}z", "\x{41e}"));
ok($objUk->lt("\x{41e}z", "\x{41f}"));
ok($objUk->lt("\x{41f}z", "\x{420}"));
ok($objUk->lt("\x{420}z", "\x{421}"));
ok($objUk->lt("\x{421}z", "\x{422}"));
ok($objUk->lt("\x{422}z", "\x{423}"));
ok($objUk->lt("\x{423}z", "\x{424}"));
ok($objUk->lt("\x{424}z", "\x{425}"));
ok($objUk->lt("\x{425}z", "\x{426}"));
ok($objUk->lt("\x{426}z", "\x{427}"));
ok($objUk->lt("\x{427}z", "\x{428}"));
ok($objUk->lt("\x{428}z", "\x{429}"));
ok($objUk->lt("\x{429}z", "\x{42c}"));
ok($objUk->lt("\x{42c}z", "\x{42e}"));
ok($objUk->lt("\x{42e}z", "\x{42f}"));

# 68

$objUk->change(level => 2);

ok($objUk->eq("\x{430}", "\x{410}"));
ok($objUk->eq("\x{431}", "\x{411}"));
ok($objUk->eq("\x{432}", "\x{412}"));
ok($objUk->eq("\x{433}", "\x{413}"));
ok($objUk->eq("\x{491}", "\x{490}"));
ok($objUk->eq("\x{434}", "\x{414}"));
ok($objUk->eq("\x{435}", "\x{415}"));
ok($objUk->eq("\x{454}", "\x{404}"));
ok($objUk->eq("\x{436}", "\x{416}"));
ok($objUk->eq("\x{437}", "\x{417}"));
ok($objUk->eq("\x{438}", "\x{418}"));
ok($objUk->eq("\x{456}", "\x{406}"));
ok($objUk->eq("\x{a647}","\x{a646}"));
ok($objUk->eq("\x{457}", "\x{407}"));
ok($objUk->eq("\x{457}", "\x{a676}"));
ok($objUk->eq("\x{a676}","\x{407}"));
ok($objUk->eq("\x{439}", "\x{419}"));
ok($objUk->eq("\x{43a}", "\x{41a}"));
ok($objUk->eq("\x{43b}", "\x{41b}"));
ok($objUk->eq("\x{43c}", "\x{41c}"));
ok($objUk->eq("\x{43d}", "\x{41d}"));
ok($objUk->eq("\x{43e}", "\x{41e}"));
ok($objUk->eq("\x{43f}", "\x{41f}"));
ok($objUk->eq("\x{440}", "\x{420}"));
ok($objUk->eq("\x{441}", "\x{421}"));
ok($objUk->eq("\x{442}", "\x{422}"));
ok($objUk->eq("\x{443}", "\x{423}"));
ok($objUk->eq("\x{444}", "\x{424}"));
ok($objUk->eq("\x{445}", "\x{425}"));
ok($objUk->eq("\x{446}", "\x{426}"));
ok($objUk->eq("\x{447}", "\x{427}"));
ok($objUk->eq("\x{448}", "\x{428}"));
ok($objUk->eq("\x{449}", "\x{429}"));
ok($objUk->eq("\x{44c}", "\x{42c}"));
ok($objUk->eq("\x{44e}", "\x{42e}"));
ok($objUk->eq("\x{44f}", "\x{42f}"));

# 104

$objUk->change(level => 3);

ok($objUk->lt("\x{430}", "\x{410}"));
ok($objUk->lt("\x{431}", "\x{411}"));
ok($objUk->lt("\x{432}", "\x{412}"));
ok($objUk->lt("\x{433}", "\x{413}"));
ok($objUk->lt("\x{491}", "\x{490}"));
ok($objUk->lt("\x{434}", "\x{414}"));
ok($objUk->lt("\x{435}", "\x{415}"));
ok($objUk->lt("\x{454}", "\x{404}"));
ok($objUk->lt("\x{436}", "\x{416}"));
ok($objUk->lt("\x{437}", "\x{417}"));
ok($objUk->lt("\x{438}", "\x{418}"));
ok($objUk->lt("\x{456}", "\x{406}"));
ok($objUk->lt("\x{a647}","\x{a646}"));
ok($objUk->lt("\x{457}", "\x{407}"));
ok($objUk->lt("\x{457}", "\x{a676}"));
ok($objUk->lt("\x{a676}","\x{407}"));
ok($objUk->lt("\x{439}", "\x{419}"));
ok($objUk->lt("\x{43a}", "\x{41a}"));
ok($objUk->lt("\x{43b}", "\x{41b}"));
ok($objUk->lt("\x{43c}", "\x{41c}"));
ok($objUk->lt("\x{43d}", "\x{41d}"));
ok($objUk->lt("\x{43e}", "\x{41e}"));
ok($objUk->lt("\x{43f}", "\x{41f}"));
ok($objUk->lt("\x{440}", "\x{420}"));
ok($objUk->lt("\x{441}", "\x{421}"));
ok($objUk->lt("\x{442}", "\x{422}"));
ok($objUk->lt("\x{443}", "\x{423}"));
ok($objUk->lt("\x{444}", "\x{424}"));
ok($objUk->lt("\x{445}", "\x{425}"));
ok($objUk->lt("\x{446}", "\x{426}"));
ok($objUk->lt("\x{447}", "\x{427}"));
ok($objUk->lt("\x{448}", "\x{428}"));
ok($objUk->lt("\x{449}", "\x{429}"));
ok($objUk->lt("\x{44c}", "\x{42c}"));
ok($objUk->lt("\x{44e}", "\x{42e}"));
ok($objUk->lt("\x{44f}", "\x{42f}"));

# 140

ok($objUk->eq("\x{457}", "\x{456}\x{308}"));
ok($objUk->eq("\x{407}", "\x{406}\x{308}"));
ok($objUk->eq("\x{439}", "\x{438}\x{306}"));
ok($objUk->eq("\x{419}", "\x{418}\x{306}"));

# 144

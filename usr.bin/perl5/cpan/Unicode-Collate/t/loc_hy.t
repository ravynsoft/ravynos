
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..25\n"; }
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

my $objHy = Unicode::Collate::Locale->
    new(locale => 'HY', normalization => undef);

ok($objHy->getlocale, 'hy');

$objHy->change(level => 1);

ok($objHy->lt("\x{584}", "\x{587}"));
ok($objHy->gt("\x{585}", "\x{587}"));

ok($objHy->lt("\x{584}\x{4E00}",  "\x{587}"));
ok($objHy->lt("\x{584}\x{20000}", "\x{587}"));
ok($objHy->lt("\x{584}\x{10FFFD}","\x{587}"));

# 7

$objHy->change(level => 2);

ok($objHy->eq("\x{587}", "\x{535}\x{582}"));

$objHy->change(level => 3);

ok($objHy->lt("\x{587}", "\x{535}\x{582}"));

$objHy->change(upper_before_lower => 1);

ok($objHy->gt("\x{587}", "\x{535}\x{582}"));

# 10

$objHy->change(level => 1);

$objHy->change(UCA_Version => 8);

ok($objHy->lt("\x{584}\x{4E00}",  "\x{587}"));
ok($objHy->lt("\x{584}\x{20000}", "\x{587}"));
ok($objHy->lt("\x{584}\x{10FFFD}","\x{587}"));

# 13

$objHy->change(UCA_Version => 22);

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

   ok($objHy->lt("\x{583}$t", "\x{584}"));
   ok($objHy->lt("\x{584}$t", "\x{587}"));
   ok($objHy->lt("\x{587}$t", "\x{585}"));

   ok($objHy->lt("\x{553}$t", "\x{554}"));
   ok($objHy->lt("\x{554}$t", "\x{535}\x{582}"));
   ok($objHy->lt("\x{535}\x{582}$t", "\x{555}"));
}

# 25

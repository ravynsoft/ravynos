
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..30\n"; }
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

my $objAr = Unicode::Collate::Locale->
    new(locale => 'AR', normalization => undef);

ok($objAr->getlocale, 'ar');

$objAr->change(level => 1);

ok($objAr->eq("\x{62A}", "\x{629}"));
ok($objAr->eq("\x{64A}", "\x{649}"));

# 4

$objAr->change(level => 2);

ok($objAr->lt("\x{62A}", "\x{629}"));
ok($objAr->eq("\x{629}", "\x{FE94}"));
ok($objAr->eq("\x{FE94}","\x{FE93}"));

ok($objAr->lt("\x{64A}", "\x{649}"));
ok($objAr->eq("\x{649}", "\x{FBE8}"));
ok($objAr->eq("\x{FBE8}","\x{FBE9}"));
ok($objAr->eq("\x{FBE9}","\x{FEF0}"));
ok($objAr->eq("\x{FEF0}","\x{FEEF}"));
ok($objAr->eq("\x{FEEF}","\x{FC90}"));
ok($objAr->eq("\x{FC90}","\x{FC5D}"));

# 14

$objAr->change(level => 3);

for my $up_lo (0, 1) {
  $objAr->change(upper_before_lower => $up_lo);

  ok($objAr->lt("\x{629}", "\x{FE94}"));
  ok($objAr->lt("\x{FE94}","\x{FE93}"));

  ok($objAr->lt("\x{649}", "\x{FBE8}"));
  ok($objAr->lt("\x{FBE8}","\x{FBE9}"));
  ok($objAr->lt("\x{FBE9}","\x{FEF0}"));
  ok($objAr->lt("\x{FEF0}","\x{FEEF}"));
  ok($objAr->lt("\x{FEEF}","\x{FC90}"));
  ok($objAr->lt("\x{FC90}","\x{FC5D}"));
}

# 30

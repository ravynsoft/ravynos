
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..41\n"; }
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

my $objPa = Unicode::Collate::Locale->
    new(locale => 'PA', normalization => undef);

my $default = Unicode::Collate::Locale->
    new(normalization => undef);

ok($objPa->getlocale, 'pa');
ok($default->getlocale, 'default');

$objPa->change(level => 1);

ok($objPa->lt("\x{A5C}", "\x{A4D}"));
ok($objPa->lt("\x{A4D}", "\x{A3E}"));

# 5

# RRA and Vowel Signs
for my $o ($objPa, $default) {
  ok($o->lt("\x{A5C}", "\x{A3E}"));
  ok($o->lt("\x{A3E}", "\x{A3F}"));
  ok($o->lt("\x{A3F}", "\x{A40}"));
  ok($o->lt("\x{A40}", "\x{A41}"));
  ok($o->lt("\x{A41}", "\x{A42}"));
  ok($o->lt("\x{A42}", "\x{A47}"));
  ok($o->lt("\x{A47}", "\x{A48}"));
  ok($o->lt("\x{A48}", "\x{A4B}"));
  ok($o->lt("\x{A4B}", "\x{A4C}"));
}

# 23

ok($default->lt("\x{A4C}", "\x{A4D}"));

# 24

ok($objPa->eq("\x{A03}", ""));
ok($objPa->eq("\x{A03}", "\x{A71}"));
ok($objPa->eq("\x{A71}", "\x{A70}"));
ok($objPa->eq("\x{A70}", "\x{A02}"));
ok($objPa->eq("\x{A02}", "\x{A01}"));
ok($objPa->eq("\x{A01}", "\x{A3C}"));

# 30

$objPa->change(level => 2);

ok($objPa->lt("\x{A03}", "\x{A71}"));
ok($objPa->lt("\x{A71}", "\x{A70}"));
ok($objPa->lt("\x{A70}", "\x{A02}"));
ok($objPa->lt("\x{A02}", "\x{A01}"));
ok($objPa->lt("\x{A01}", "\x{A3C}"));

# 35

$objPa->change(level => 3);

ok($objPa->eq("\x{A33}", "\x{A32}\x{A3C}"));
ok($objPa->eq("\x{A36}", "\x{A38}\x{A3C}"));
ok($objPa->eq("\x{A59}", "\x{A16}\x{A3C}"));
ok($objPa->eq("\x{A5A}", "\x{A17}\x{A3C}"));
ok($objPa->eq("\x{A5B}", "\x{A1C}\x{A3C}"));
ok($objPa->eq("\x{A5E}", "\x{A2B}\x{A3C}"));

# 41

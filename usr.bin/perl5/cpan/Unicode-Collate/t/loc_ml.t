
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

my $objMl = Unicode::Collate::Locale->
    new(locale => 'ML', normalization => undef);

ok($objMl->getlocale, 'ml');

$objMl->change(level => 1);

ok($objMl->lt("\x{D4B}", "\x{D57}"));

# 3

ok($objMl->eq("\x{D03}",        "\x{D3D}"));
ok($objMl->eq("\x{D15}\x{D4D}", "\x{D15}\x{D4D}\x{200D}"));
ok($objMl->eq("\x{D23}\x{D4D}", "\x{D23}\x{D4D}\x{200D}"));
ok($objMl->eq("\x{D28}\x{D4D}", "\x{D28}\x{D4D}\x{200D}"));
ok($objMl->eq("\x{D30}\x{D4D}", "\x{D30}\x{D4D}\x{200D}"));
ok($objMl->eq("\x{D32}\x{D4D}", "\x{D32}\x{D4D}\x{200D}"));
ok($objMl->eq("\x{D33}\x{D4D}", "\x{D33}\x{D4D}\x{200D}"));
ok($objMl->eq("\x{D2E}\x{D4D}", "\x{D02}"));

# 11

$objMl->change(level => 2);

ok($objMl->lt("\x{D03}",        "\x{D3D}"));
ok($objMl->lt("\x{D15}\x{D4D}", "\x{D15}\x{D4D}\x{200D}"));
ok($objMl->lt("\x{D23}\x{D4D}", "\x{D23}\x{D4D}\x{200D}"));
ok($objMl->lt("\x{D28}\x{D4D}", "\x{D28}\x{D4D}\x{200D}"));
ok($objMl->lt("\x{D30}\x{D4D}", "\x{D30}\x{D4D}\x{200D}"));
ok($objMl->lt("\x{D32}\x{D4D}", "\x{D32}\x{D4D}\x{200D}"));
ok($objMl->lt("\x{D33}\x{D4D}", "\x{D33}\x{D4D}\x{200D}"));
ok($objMl->lt("\x{D2E}\x{D4D}", "\x{D02}"));

# 19

ok($objMl->eq("\x{D57}",                "\x{D4C}"));
ok($objMl->eq("\x{D15}\x{D4D}\x{200D}", "\x{D7F}"));
ok($objMl->eq("\x{D23}\x{D4D}\x{200D}", "\x{D7A}"));
ok($objMl->eq("\x{D28}\x{D4D}\x{200D}", "\x{D7B}"));
ok($objMl->eq("\x{D30}\x{D4D}\x{200D}", "\x{D7C}"));
ok($objMl->eq("\x{D32}\x{D4D}\x{200D}", "\x{D7D}"));
ok($objMl->eq("\x{D33}\x{D4D}\x{200D}", "\x{D7E}"));
ok($objMl->eq("\x{D28}\x{D4D}",         "\x{D7B}\x{D4D}"));

# 27

$objMl->change(level => 3);

ok($objMl->lt("\x{D57}",                "\x{D4C}"));
ok($objMl->lt("\x{D15}\x{D4D}\x{200D}", "\x{D7F}"));
ok($objMl->lt("\x{D23}\x{D4D}\x{200D}", "\x{D7A}"));
ok($objMl->lt("\x{D28}\x{D4D}\x{200D}", "\x{D7B}"));
ok($objMl->lt("\x{D30}\x{D4D}\x{200D}", "\x{D7C}"));
ok($objMl->lt("\x{D32}\x{D4D}\x{200D}", "\x{D7D}"));
ok($objMl->lt("\x{D33}\x{D4D}\x{200D}", "\x{D7E}"));
ok($objMl->lt("\x{D28}\x{D4D}",         "\x{D7B}\x{D4D}"));

# 35

ok($objMl->eq("\x{D4C}", "\x{D46}\x{D57}"));

# 36

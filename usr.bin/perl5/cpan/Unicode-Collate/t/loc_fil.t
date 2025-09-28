
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..19\n"; }
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

my $objFil = Unicode::Collate::Locale->
    new(locale => 'FIL', normalization => undef);

ok($objFil->getlocale, 'fil');

$objFil->change(level => 1);

ok($objFil->lt("n", "n\x{303}"));
ok($objFil->lt("nz","n\x{303}"));
ok($objFil->gt("ng","n\x{303}"));
ok($objFil->gt("ng","n\x{303}z"));
ok($objFil->gt("o", "ng"));
ok($objFil->gt("o", "ngz"));

# 8

$objFil->change(level => 2);

ok($objFil->eq("ng", "Ng"));
ok($objFil->eq("Ng", "NG"));
ok($objFil->eq("n\x{303}", "N\x{303}"));

# 11

$objFil->change(level => 3);

ok($objFil->lt("ng", "Ng"));
ok($objFil->lt("Ng", "NG"));
ok($objFil->lt("n\x{303}", "N\x{303}"));
ok($objFil->eq("n\x{303}", _pack_U(0xF1)));
ok($objFil->eq("N\x{303}", _pack_U(0xD1)));

# 16

$objFil->change(upper_before_lower => 1);

ok($objFil->gt("ng", "Ng"));
ok($objFil->gt("Ng", "NG"));
ok($objFil->gt("n\x{303}", "N\x{303}"));

# 19

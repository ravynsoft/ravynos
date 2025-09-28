
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

my $objOm = Unicode::Collate::Locale->
    new(locale => 'OM', normalization => undef);

ok($objOm->getlocale, 'om');

$objOm->change(level => 1);

ok($objOm->lt("z",  "ch"));
ok($objOm->lt("ch", "dh"));
ok($objOm->lt("dh", "kh"));
ok($objOm->lt("kh", "ny"));
ok($objOm->lt("ny", "ph"));
ok($objOm->lt("ph", "sh"));

# 8

$objOm->change(level => 2);

ok($objOm->eq("ch", "Ch"));
ok($objOm->eq("Ch", "CH"));
ok($objOm->eq("dh", "Dh"));
ok($objOm->eq("Dh", "DH"));
ok($objOm->eq("kh", "Kh"));
ok($objOm->eq("Kh", "KH"));
ok($objOm->eq("ny", "Ny"));
ok($objOm->eq("Ny", "NY"));
ok($objOm->eq("ph", "Ph"));
ok($objOm->eq("Ph", "PH"));
ok($objOm->eq("sh", "Sh"));

# 19

$objOm->change(level => 3);

ok($objOm->lt("ch", "Ch"));
ok($objOm->lt("Ch", "CH"));
ok($objOm->lt("dh", "Dh"));
ok($objOm->lt("Dh", "DH"));
ok($objOm->lt("kh", "Kh"));
ok($objOm->lt("Kh", "KH"));
ok($objOm->lt("ny", "Ny"));
ok($objOm->lt("Ny", "NY"));
ok($objOm->lt("ph", "Ph"));
ok($objOm->lt("Ph", "PH"));
ok($objOm->lt("sh", "Sh"));

# 30

$objOm->change(upper_before_lower => 1);

ok($objOm->gt("ch", "Ch"));
ok($objOm->gt("Ch", "CH"));
ok($objOm->gt("dh", "Dh"));
ok($objOm->gt("Dh", "DH"));
ok($objOm->gt("kh", "Kh"));
ok($objOm->gt("Kh", "KH"));
ok($objOm->gt("ny", "Ny"));
ok($objOm->gt("Ny", "NY"));
ok($objOm->gt("ph", "Ph"));
ok($objOm->gt("Ph", "PH"));
ok($objOm->gt("sh", "Sh"));

# 41

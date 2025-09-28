
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..74\n"; }
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

my $objCy = Unicode::Collate::Locale->
    new(locale => 'CY', normalization => undef);

ok($objCy->getlocale, 'cy');

$objCy->change(level => 1);

ok($objCy->lt("c", "ch"));
ok($objCy->lt("cz","ch"));
ok($objCy->gt("d", "ch"));
ok($objCy->lt("d", "dd"));
ok($objCy->lt("dz","dd"));
ok($objCy->gt("e", "dd"));
ok($objCy->lt("f", "ff"));
ok($objCy->lt("fz","ff"));
ok($objCy->gt("g", "ff"));
ok($objCy->lt("g", "ng"));
ok($objCy->lt("gz","ng"));
ok($objCy->gt("h", "ng"));
ok($objCy->lt("l", "ll"));
ok($objCy->lt("lz","ll"));
ok($objCy->gt("m", "ll"));
ok($objCy->lt("p", "ph"));
ok($objCy->lt("pz","ph"));
ok($objCy->gt("q", "ph"));
ok($objCy->lt("r", "rh"));
ok($objCy->lt("rz","rh"));
ok($objCy->gt("s", "rh"));
ok($objCy->lt("t", "th"));
ok($objCy->lt("tz","th"));
ok($objCy->gt("u", "th"));

# 26

$objCy->change(level => 2);

ok($objCy->eq("ch", "Ch"));
ok($objCy->eq("Ch", "CH"));
ok($objCy->eq("dd", "Dd"));
ok($objCy->eq("Dd", "DD"));
ok($objCy->eq("ff", "Ff"));
ok($objCy->eq("Ff", "FF"));
ok($objCy->eq("ng", "Ng"));
ok($objCy->eq("Ng", "NG"));
ok($objCy->eq("ll", "Ll"));
ok($objCy->eq("Ll", "LL"));
ok($objCy->eq("ph", "Ph"));
ok($objCy->eq("Ph", "PH"));
ok($objCy->eq("rh", "Rh"));
ok($objCy->eq("Rh", "RH"));
ok($objCy->eq("th", "Th"));
ok($objCy->eq("Th", "TH"));

# 42

$objCy->change(level => 3);

ok($objCy->lt("ch", "Ch"));
ok($objCy->lt("Ch", "CH"));
ok($objCy->lt("dd", "Dd"));
ok($objCy->lt("Dd", "DD"));
ok($objCy->lt("ff", "Ff"));
ok($objCy->lt("Ff", "FF"));
ok($objCy->lt("ng", "Ng"));
ok($objCy->lt("Ng", "NG"));
ok($objCy->lt("ll", "Ll"));
ok($objCy->lt("Ll", "LL"));
ok($objCy->lt("ph", "Ph"));
ok($objCy->lt("Ph", "PH"));
ok($objCy->lt("rh", "Rh"));
ok($objCy->lt("Rh", "RH"));
ok($objCy->lt("th", "Th"));
ok($objCy->lt("Th", "TH"));

# 58

$objCy->change(upper_before_lower => 1);

ok($objCy->gt("ch", "Ch"));
ok($objCy->gt("Ch", "CH"));
ok($objCy->gt("dd", "Dd"));
ok($objCy->gt("Dd", "DD"));
ok($objCy->gt("ff", "Ff"));
ok($objCy->gt("Ff", "FF"));
ok($objCy->gt("ng", "Ng"));
ok($objCy->gt("Ng", "NG"));
ok($objCy->gt("ll", "Ll"));
ok($objCy->gt("Ll", "LL"));
ok($objCy->gt("ph", "Ph"));
ok($objCy->gt("Ph", "PH"));
ok($objCy->gt("rh", "Rh"));
ok($objCy->gt("Rh", "RH"));
ok($objCy->gt("th", "Th"));
ok($objCy->gt("Th", "TH"));

# 74

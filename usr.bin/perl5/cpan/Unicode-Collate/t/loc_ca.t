
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..54\n"; }
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

my $dot  = _pack_U(0xB7);

my $objCa = Unicode::Collate::Locale->
    new(locale => 'CA', normalization => undef);

ok($objCa->getlocale, 'ca');

$objCa->change(level => 1);

ok($objCa->lt("c", "ch"));
ok($objCa->lt("cz","ch"));
ok($objCa->gt("d", "ch"));
ok($objCa->lt("l", "ll"));
ok($objCa->lt("lz","ll"));
ok($objCa->gt("m", "ll"));

# 8

ok($objCa->eq("a\x{300}a", "aa\x{300}"));

$objCa->change(level => 2);

ok($objCa->gt("a\x{300}a", "aa\x{300}"));
ok($objCa->lt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
ok($objCa->lt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));

# 12

ok($objCa->eq("ch", "cH"));
ok($objCa->eq("cH", "Ch"));
ok($objCa->eq("Ch", "CH"));

ok($objCa->eq("ll", "lL"));
ok($objCa->eq("lL", "Ll"));
ok($objCa->eq("Ll", "LL"));
ok($objCa->eq("ll", "l${dot}l"));
ok($objCa->eq("lL", "l${dot}l"));
ok($objCa->eq("lL", "l${dot}L"));
ok($objCa->eq("Ll", "l${dot}L"));
ok($objCa->eq("Ll", "L${dot}l"));
ok($objCa->eq("LL", "L${dot}l"));
ok($objCa->eq("LL", "L${dot}L"));

# 25

$objCa->change(level => 3);

ok($objCa->lt("ch", "cH"));
ok($objCa->lt("cH", "Ch"));
ok($objCa->lt("Ch", "CH"));

ok($objCa->lt("ll", "lL"));
ok($objCa->lt("lL", "Ll"));
ok($objCa->lt("Ll", "LL"));
ok($objCa->lt("ll", "l${dot}l"));
ok($objCa->gt("lL", "l${dot}l"));
ok($objCa->lt("lL", "l${dot}L"));
ok($objCa->gt("Ll", "l${dot}L"));
ok($objCa->lt("Ll", "L${dot}l"));
ok($objCa->gt("LL", "L${dot}l"));
ok($objCa->lt("LL", "L${dot}L"));

# 38

$objCa->change(upper_before_lower => 1);

ok($objCa->gt("ch", "cH"));
ok($objCa->gt("cH", "Ch"));
ok($objCa->gt("Ch", "CH"));

ok($objCa->gt("ll", "lL"));
ok($objCa->gt("lL", "Ll"));
ok($objCa->gt("Ll", "LL"));
ok($objCa->lt("ll", "l${dot}l"));
ok($objCa->lt("lL", "l${dot}l"));
ok($objCa->lt("lL", "l${dot}L"));
ok($objCa->lt("Ll", "l${dot}L"));
ok($objCa->lt("Ll", "L${dot}l"));
ok($objCa->lt("LL", "L${dot}l"));
ok($objCa->lt("LL", "L${dot}L"));

# 51

$objCa->change(backwards => 2, level => 2);

ok($objCa->lt("a\x{300}a", "aa\x{300}"));
ok($objCa->gt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
ok($objCa->gt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));

# 54

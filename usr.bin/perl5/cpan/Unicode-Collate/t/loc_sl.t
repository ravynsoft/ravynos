
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..20\n"; }
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

my $objSl = Unicode::Collate::Locale->
    new(locale => 'SL', normalization => undef);

ok($objSl->getlocale, 'sl');

$objSl->change(level => 1);

ok($objSl->lt("c", "c\x{30C}"));
ok($objSl->gt("d", "c\x{30C}"));
ok($objSl->lt("s", "s\x{30C}"));
ok($objSl->gt("t", "s\x{30C}"));
ok($objSl->lt("z", "z\x{30C}"));
ok($objSl->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 8

$objSl->change(level => 2);

ok($objSl->eq("c\x{30C}", "C\x{30C}"));
ok($objSl->eq("s\x{30C}", "S\x{30C}"));
ok($objSl->eq("z\x{30C}", "Z\x{30C}"));

# 11

$objSl->change(level => 3);

ok($objSl->lt("c\x{30C}", "C\x{30C}"));
ok($objSl->lt("s\x{30C}", "S\x{30C}"));
ok($objSl->lt("z\x{30C}", "Z\x{30C}"));

# 14

ok($objSl->eq("c\x{30C}", "\x{10D}"));
ok($objSl->eq("C\x{30C}", "\x{10C}"));
ok($objSl->eq("s\x{30C}", "\x{161}"));
ok($objSl->eq("S\x{30C}", "\x{160}"));
ok($objSl->eq("z\x{30C}", "\x{17E}"));
ok($objSl->eq("Z\x{30C}", "\x{17D}"));

# 20

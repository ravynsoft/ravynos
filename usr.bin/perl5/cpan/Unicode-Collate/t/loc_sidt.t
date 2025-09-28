
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..14\n"; }
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

my $objSiDict = Unicode::Collate::Locale->
    new(locale => 'SI-dict', normalization => undef);

ok($objSiDict->getlocale, 'si__dictionary');

$objSiDict->change(level => 2);

ok($objSiDict->gt("\x{DA5}", "\x{DA2}\x{DCA}\x{DA4}"));

$objSiDict->change(level => 1);

ok($objSiDict->eq("\x{DA5}", "\x{DA2}\x{DCA}\x{DA4}"));

ok($objSiDict->lt("\x{DA2}", "\x{DA5}"));
ok($objSiDict->lt("\x{DA5}", "\x{DA3}"));

# 6

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objSiDict->lt("\x{D95}$t", "\x{D96}"));
    ok($objSiDict->lt("\x{D96}$t", "\x{D82}"));
    ok($objSiDict->lt("\x{D82}$t", "\x{D83}"));
    ok($objSiDict->lt("\x{D83}$t", "\x{D9A}"));
}

# 14

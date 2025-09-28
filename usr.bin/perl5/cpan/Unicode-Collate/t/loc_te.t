
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..12\n"; }
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

my $objTe = Unicode::Collate::Locale->
    new(locale => 'TE', normalization => undef);

ok($objTe->getlocale, 'te');

$objTe->change(level => 1);

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objTe->lt("\x{C13}$t", "\x{C14}"));
    ok($objTe->lt("\x{C14}$t", "\x{C01}"));
    ok($objTe->lt("\x{C01}$t", "\x{C02}"));
    ok($objTe->lt("\x{C02}$t", "\x{C03}"));
    ok($objTe->lt("\x{C03}$t", "\x{C15}"));
}

# 12

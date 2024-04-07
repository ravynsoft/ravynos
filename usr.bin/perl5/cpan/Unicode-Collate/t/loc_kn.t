
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

my $objKn = Unicode::Collate::Locale->
    new(locale => 'KN', normalization => undef);

ok($objKn->getlocale, 'kn');

$objKn->change(level => 1);

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objKn->lt("\x{C93}$t", "\x{C94}"));
    ok($objKn->lt("\x{C94}$t", "\x{C82}"));
    ok($objKn->lt("\x{C82}$t", "\x{C83}"));
    ok($objKn->lt("\x{C83}$t", "\x{CF1}"));
    ok($objKn->lt("\x{CF1}$t", "\x{CF2}"));
    ok($objKn->lt("\x{CF2}$t", "\x{C95}"));
}

# 14


BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..23\n"; }
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

my $objKok = Unicode::Collate::Locale->
    new(locale => 'KOK', normalization => undef);

ok($objKok->getlocale, 'kok');

$objKok->change(level => 2);

ok($objKok->lt("\x{902}", "\x{901}"));
ok($objKok->lt("\x{933}", "\x{934}"));

$objKok->change(level => 3);

ok($objKok->eq("\x{933}\x{93C}", "\x{934}"));

$objKok->change(level => 1);

ok($objKok->eq("\x{902}", "\x{901}"));
ok($objKok->eq("\x{933}", "\x{934}"));

# 7

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objKok->lt("\x{950}$t", "\x{902}"));
    ok($objKok->lt("\x{902}$t", "\x{903}"));
    ok($objKok->lt("\x{903}$t", "\x{A8FD}"));
    ok($objKok->lt("\x{903}$t", "\x{972}"));

    ok($objKok->lt("\x{938}$t", "\x{939}"));
    ok($objKok->lt("\x{939}$t", "\x{933}"));
    ok($objKok->lt("\x{933}$t", "\x{915}\x{94D}\x{937}"));
    ok($objKok->lt("\x{915}\x{94D}\x{937}$t", "\x{93D}"));
}

# 23

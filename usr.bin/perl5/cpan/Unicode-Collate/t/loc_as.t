
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..29\n"; }
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

my $objAs = Unicode::Collate::Locale->
    new(locale => 'AS', normalization => undef);

ok($objAs->getlocale, 'as');

$objAs->change(level => 1);

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : 'z';

    ok($objAs->lt("\x{993}$t", "\x{994}"));
    ok($objAs->lt("\x{994}$t", "\x{982}"));
    ok($objAs->lt("\x{982}$t", "\x{981}"));
    ok($objAs->lt("\x{981}$t", "\x{983}"));
    ok($objAs->lt("\x{983}$t", "\x{995}"));

    ok($objAs->lt("\x{9A2}$t", "\x{9A3}"));
    ok($objAs->lt("\x{9A3}$t", "\x{9A4}\x{9CD}\x{200D}"));
    ok($objAs->lt("\x{9A4}\x{9CD}\x{200D}$t", "\x{9A4}"));

    ok($objAs->lt("\x{9A3}$t", "\x{9CE}"));
    ok($objAs->lt("\x{9CE}$t", "\x{9A4}"));

    ok($objAs->lt("\x{9B8}$t", "\x{9B9}"));
    ok($objAs->lt("\x{9B9}$t", "\x{995}\x{9CD}\x{9B7}"));
    ok($objAs->lt("\x{995}\x{9CD}\x{9B7}$t", "\x{9BD}"));
}

# 28

$objAs->change(level => 3);

ok($objAs->eq("\x{9A4}\x{9CD}\x{200D}", "\x{9CE}"));

# 29

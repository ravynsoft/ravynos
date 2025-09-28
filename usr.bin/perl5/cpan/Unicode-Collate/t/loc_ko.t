
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..52\n"; }
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

my $objKo = Unicode::Collate::Locale->
    new(locale => 'KO', normalization => undef);

ok($objKo->getlocale, 'ko');

$objKo->change(level => 1);

ok($objKo->eq("\x{AC00}", "\x{4F3D}"));
ok($objKo->eq("\x{4F3D}", "\x{4F73}"));
ok($objKo->eq("\x{4F73}", "\x{5047}"));
ok($objKo->eq("\x{5047}", "\x{50F9}"));
ok($objKo->eq("\x{50F9}", "\x{52A0}"));
ok($objKo->eq("\x{52A0}", "\x{53EF}"));
ok($objKo->lt("\x{53EF}", "\x{AC01}"));

ok($objKo->eq("\x{AC1D}", "\x{5580}"));
ok($objKo->eq("\x{5580}", "\x{5BA2}"));
ok($objKo->lt("\x{5BA2}", "\x{AC31}"));

ok($objKo->eq("\x{C77C}", "\x{4E00}"));
ok($objKo->eq("\x{4E00}", "\x{4F5A}"));
ok($objKo->eq("\x{4F5A}", "\x{4F7E}"));

ok($objKo->lt("\x{993C}", "\x{D790}"));
ok($objKo->eq("\x{D790}", "\x{8A70}"));
ok($objKo->eq("\x{8A70}", "\x{72B5}"));
ok($objKo->eq("\x{72B5}", "\x{7E88}"));
ok($objKo->eq("\x{7E88}", "\x{896D}"));
ok($objKo->eq("\x{896D}", "\x{9821}"));
ok($objKo->eq("\x{9821}", "\x{9EE0}"));

# 22

# Ext.B
ok($objKo->lt("\x{20000}", "\x{20001}"));
ok($objKo->lt("\x{20001}", "\x{20002}"));
ok($objKo->lt("\x{20002}", "\x{20003}"));
ok($objKo->lt("\x{20003}", "\x{20004}"));
ok($objKo->lt("\x{20004}", "\x{20005}"));

# 27

$objKo->change(level => 2);

ok($objKo->lt("\x{AC00}", "\x{4F3D}"));
ok($objKo->lt("\x{4F3D}", "\x{4F73}"));
ok($objKo->lt("\x{4F73}", "\x{5047}"));
ok($objKo->lt("\x{5047}", "\x{50F9}"));
ok($objKo->lt("\x{50F9}", "\x{52A0}"));
ok($objKo->lt("\x{52A0}", "\x{53EF}"));
ok($objKo->lt("\x{53EF}", "\x{AC01}"));

ok($objKo->lt("\x{AC1D}", "\x{5580}"));
ok($objKo->lt("\x{5580}", "\x{5BA2}"));
ok($objKo->lt("\x{5BA2}", "\x{AC31}"));

ok($objKo->lt("\x{C77C}", "\x{4E00}"));
ok($objKo->lt("\x{4E00}", "\x{4F5A}"));
ok($objKo->lt("\x{4F5A}", "\x{4F7E}"));

ok($objKo->lt("\x{993C}", "\x{D790}"));
ok($objKo->lt("\x{D790}", "\x{8A70}"));
ok($objKo->lt("\x{8A70}", "\x{72B5}"));
ok($objKo->lt("\x{72B5}", "\x{7E88}"));
ok($objKo->lt("\x{7E88}", "\x{896D}"));
ok($objKo->lt("\x{896D}", "\x{9821}"));
ok($objKo->lt("\x{9821}", "\x{9EE0}"));

# 47

# Ext.B
ok($objKo->lt("\x{20000}", "\x{20001}"));
ok($objKo->lt("\x{20001}", "\x{20002}"));
ok($objKo->lt("\x{20002}", "\x{20003}"));
ok($objKo->lt("\x{20003}", "\x{20004}"));
ok($objKo->lt("\x{20004}", "\x{20005}"));

# 52

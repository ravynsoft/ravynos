
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

use Unicode::Collate;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

use Unicode::Collate::CJK::Korean;

my $collator = Unicode::Collate->new(
    normalization => undef,
    overrideCJK => \&Unicode::Collate::CJK::Korean::weightKorean
);

sub hex_sort {
    my @source = map _pack_U(hex $_), split ' ', shift;
    my @sorted = $collator->sort(@source);
    return join " ", map sprintf("%04X", _unpack_U($_)), @sorted;
}

# 1

$collator->change(level => 1);

ok($collator->eq("\x{AC00}", "\x{4F3D}"));
ok($collator->eq("\x{4F3D}", "\x{4F73}"));
ok($collator->eq("\x{4F73}", "\x{5047}"));
ok($collator->eq("\x{5047}", "\x{50F9}"));
ok($collator->eq("\x{50F9}", "\x{52A0}"));
ok($collator->eq("\x{52A0}", "\x{53EF}"));
ok($collator->lt("\x{53EF}", "\x{AC01}"));

ok($collator->eq("\x{AC1D}", "\x{5580}"));
ok($collator->eq("\x{5580}", "\x{5BA2}"));
ok($collator->lt("\x{5BA2}", "\x{AC31}"));

ok($collator->eq("\x{C77C}", "\x{4E00}"));
ok($collator->eq("\x{4E00}", "\x{4F5A}"));
ok($collator->eq("\x{4F5A}", "\x{4F7E}"));

ok($collator->lt("\x{993C}", "\x{D790}"));
ok($collator->eq("\x{D790}", "\x{8A70}"));
ok($collator->eq("\x{8A70}", "\x{72B5}"));
ok($collator->eq("\x{72B5}", "\x{7E88}"));
ok($collator->eq("\x{7E88}", "\x{896D}"));
ok($collator->eq("\x{896D}", "\x{9821}"));
ok($collator->eq("\x{9821}", "\x{9EE0}"));

# Ext.B
ok($collator->lt("\x{20000}", "\x{20001}"));
ok($collator->lt("\x{20001}", "\x{20002}"));
ok($collator->lt("\x{20002}", "\x{20003}"));
ok($collator->lt("\x{20003}", "\x{20004}"));
ok($collator->lt("\x{20004}", "\x{20005}"));

# 26

$collator->change(level => 2);

ok($collator->lt("\x{AC00}", "\x{4F3D}"));
ok($collator->lt("\x{4F3D}", "\x{4F73}"));
ok($collator->lt("\x{4F73}", "\x{5047}"));
ok($collator->lt("\x{5047}", "\x{50F9}"));
ok($collator->lt("\x{50F9}", "\x{52A0}"));
ok($collator->lt("\x{52A0}", "\x{53EF}"));
ok($collator->lt("\x{53EF}", "\x{AC01}"));

ok($collator->lt("\x{AC1D}", "\x{5580}"));
ok($collator->lt("\x{5580}", "\x{5BA2}"));
ok($collator->lt("\x{5BA2}", "\x{AC31}"));

ok($collator->lt("\x{C77C}", "\x{4E00}"));
ok($collator->lt("\x{4E00}", "\x{4F5A}"));
ok($collator->lt("\x{4F5A}", "\x{4F7E}"));

ok($collator->lt("\x{993C}", "\x{D790}"));
ok($collator->lt("\x{D790}", "\x{8A70}"));
ok($collator->lt("\x{8A70}", "\x{72B5}"));
ok($collator->lt("\x{72B5}", "\x{7E88}"));
ok($collator->lt("\x{7E88}", "\x{896D}"));
ok($collator->lt("\x{896D}", "\x{9821}"));
ok($collator->lt("\x{9821}", "\x{9EE0}"));

# Ext.B
ok($collator->lt("\x{20000}", "\x{20001}"));
ok($collator->lt("\x{20001}", "\x{20002}"));
ok($collator->lt("\x{20002}", "\x{20003}"));
ok($collator->lt("\x{20003}", "\x{20004}"));
ok($collator->lt("\x{20004}", "\x{20005}"));

# 51

ok(hex_sort('4E00 4E8C 4E09 56DB 4E94 516D 4E03 516B 4E5D 5341'),
            '4E5D 516D 56DB 4E09 5341 4E94 4E8C 4E00 4E03 516B');

# 52

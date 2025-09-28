
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..19\n"; }
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

use Unicode::Collate::CJK::Pinyin;

my $collator = Unicode::Collate->new(
    table => undef,
    normalization => undef,
    overrideCJK => \&Unicode::Collate::CJK::Pinyin::weightPinyin
);

sub hex_sort {
    my @source = map _pack_U(hex $_), split ' ', shift;
    my @sorted = $collator->sort(@source);
    return join " ", map sprintf("%04X", _unpack_U($_)), @sorted;
}

# 1

$collator->change(level => 1);

ok($collator->lt("\x{963F}", "\x{5730}"));
ok($collator->lt("\x{5730}", "\x{7ACB}"));
ok($collator->lt("\x{7ACB}", "\x{4EBA}"));
ok($collator->lt("\x{4EBA}", "\x{65E5}"));
ok($collator->lt("\x{65E5}", "\x{4E0A}"));
ok($collator->lt("\x{4E0A}", "\x{5929}"));
ok($collator->lt("\x{5929}", "\x{4E0B}"));
ok($collator->lt("\x{4E0B}", "\x{65BC}"));
ok($collator->lt("\x{65BC}", "\x{4E2D}"));
ok($collator->lt("\x{4E2D}", "\x{7AFA}"));
ok($collator->lt("\x{7AFA}", "\x{5750}"));

# Ext.B
ok($collator->lt("\x{20000}", "\x{20001}"));
ok($collator->lt("\x{20001}", "\x{20002}"));
ok($collator->lt("\x{20002}", "\x{20003}"));
ok($collator->lt("\x{20003}", "\x{20004}"));
ok($collator->lt("\x{20004}", "\x{20005}"));

# 17

ok(hex_sort('4E00 4E8C 4E09 56DB 4E94 516D 4E03 516B 4E5D 5341'),
            '516B 4E8C 4E5D 516D 4E03 4E09 5341 56DB 4E94 4E00');

ok(hex_sort('4E0C 4E8D 4F5C 5140 554A 5750 57C3 5EA7 963F 9F3D 9F3E 9F44'),
            '963F 554A 57C3 4E8D 9F3E 4E0C 9F3D 5140 9F44 4F5C 5750 5EA7');

# 19

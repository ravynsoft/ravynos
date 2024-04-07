
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..33\n"; }
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

use Unicode::Collate::CJK::JISX0208;

my $collator = Unicode::Collate->new(
    table => undef,
    normalization => undef,
    overrideCJK => \&Unicode::Collate::CJK::JISX0208::weightJISX0208
);

sub hex_sort {
    my @source = map _pack_U(hex $_), split ' ', shift;
    my @sorted = $collator->sort(@source);
    return join " ", map sprintf("%04X", _unpack_U($_)), @sorted;
}

# 1

$collator->change(level => 1);

# first ten kanji
ok($collator->lt("\x{4E9C}", "\x{5516}"));
ok($collator->lt("\x{5516}", "\x{5A03}"));
ok($collator->lt("\x{5A03}", "\x{963F}"));
ok($collator->lt("\x{963F}", "\x{54C0}"));
ok($collator->lt("\x{54C0}", "\x{611B}"));
ok($collator->lt("\x{611B}", "\x{6328}"));
ok($collator->lt("\x{6328}", "\x{59F6}"));
ok($collator->lt("\x{59F6}", "\x{9022}"));
ok($collator->lt("\x{9022}", "\x{8475}"));

# last five kanji and undef
ok($collator->lt("\x{69C7}", "\x{9059}"));
ok($collator->lt("\x{9059}", "\x{7464}"));
ok($collator->lt("\x{7464}", "\x{51DC}"));
ok($collator->lt("\x{51DC}", "\x{7199}"));
ok($collator->lt("\x{7199}", "\x{4E02}")); # 4E02: UIdeo undef in JIS X 0208
ok($collator->lt("\x{4E02}", "\x{3400}")); # 3400: Ext.A undef in JIS X 0208

# Ext.B
ok($collator->lt("\x{20000}", "\x{20001}"));
ok($collator->lt("\x{20001}", "\x{20002}"));
ok($collator->lt("\x{20002}", "\x{20003}"));
ok($collator->lt("\x{20003}", "\x{20004}"));
ok($collator->lt("\x{20004}", "\x{20005}"));

# 21

ok(hex_sort('4E00 4E8C 4E09 56DB 4E94 516D 4E03 516B 4E5D 5341'),
            '4E00 4E5D 4E94 4E09 56DB 4E03 5341 4E8C 516B 516D');

# 22

$collator->change(overrideCJK => undef);

ok($collator->lt("\x{4E00}", "\x{4E01}"));
ok($collator->lt("\x{4E01}", "\x{4E02}"));
ok($collator->lt("\x{4E02}", "\x{4E03}"));
ok($collator->lt("\x{4E03}", "\x{4E04}"));
ok($collator->lt("\x{4E04}", "\x{4E05}"));

ok($collator->lt("\x{9F9B}", "\x{9F9C}"));
ok($collator->lt("\x{9F9C}", "\x{9F9D}"));
ok($collator->lt("\x{9F9D}", "\x{9F9E}"));
ok($collator->lt("\x{9F9E}", "\x{9F9F}"));
ok($collator->lt("\x{9F9F}", "\x{9FA0}"));

# 32

ok(hex_sort('4E00 4E8C 4E09 56DB 4E94 516D 4E03 516B 4E5D 5341'),
            '4E00 4E03 4E09 4E5D 4E8C 4E94 516B 516D 5341 56DB');

# 33

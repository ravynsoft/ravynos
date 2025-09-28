
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..26\n"; }
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

use Unicode::Collate::CJK::Stroke;

my $collator = Unicode::Collate->new(
    table => undef,
    normalization => undef,
    overrideCJK => \&Unicode::Collate::CJK::Stroke::weightStroke
);

sub hex_sort {
    my @source = map _pack_U(hex $_), split ' ', shift;
    my @sorted = $collator->sort(@source);
    return join " ", map sprintf("%04X", _unpack_U($_)), @sorted;
}

# 1

$collator->change(level => 1);

ok($collator->lt("\x{4E00}", "\x{4E59}"));
ok($collator->lt("\x{4E59}", "\x{4E01}"));
ok($collator->lt("\x{4E01}", "\x{4E03}"));
ok($collator->lt("\x{4E03}", "\x{4E43}"));
ok($collator->lt("\x{4E43}", "\x{4E5D}"));
ok($collator->lt("\x{4E5D}", "\x{4E86}"));
ok($collator->lt("\x{4E86}", "\x{4E8C}"));
ok($collator->lt("\x{4E8C}", "\x{4EBA}"));
ok($collator->lt("\x{4EBA}", "\x{513F}"));
ok($collator->lt("\x{513F}", "\x{5165}"));

ok($collator->lt("\x{9E1D}", "\x{7069}"));
ok($collator->lt("\x{7069}", "\x{706A}"));
ok($collator->lt("\x{706A}", "\x{7C72}"));
ok($collator->lt("\x{7C72}", "\x{7229}"));
ok($collator->lt("\x{7229}", "\x{9EA4}"));
ok($collator->lt("\x{9EA4}", "\x{9F7E}"));
ok($collator->lt("\x{9F7E}", "\x{9F49}"));
ok($collator->lt("\x{9F49}", "\x{9F98}"));

# Ext.B
ok($collator->lt("\x{20000}", "\x{20001}"));
ok($collator->lt("\x{20001}", "\x{20002}"));
ok($collator->lt("\x{20002}", "\x{20003}"));
ok($collator->lt("\x{20003}", "\x{20004}"));
ok($collator->lt("\x{20004}", "\x{20005}"));

# 24

ok(hex_sort('4E00 4E8C 4E09 56DB 4E94 516D 4E03 516B 4E5D 5341'),
            '4E00 4E03 4E5D 4E8C 516B 5341 4E09 4E94 516D 56DB');

ok(hex_sort('4E0C 4E8D 4F5C 5140 554A 5750 57C3 5EA7 963F 9F3D 9F3E 9F44'),
            '4E0C 4E8D 5140 4F5C 5750 963F 57C3 5EA7 554A 9F3D 9F3E 9F44');

# 26

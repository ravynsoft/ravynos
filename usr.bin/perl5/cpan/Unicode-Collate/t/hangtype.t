
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..951\n"; } # 1 + 50 x @Versions
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

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

for my $v (@Versions) {
    ok(Unicode::Collate::getHST(0x0000, $v), '');
    ok(Unicode::Collate::getHST(0x0100, $v), '');
    ok(Unicode::Collate::getHST(0x1000, $v), '');
    ok(Unicode::Collate::getHST(0x10FF, $v), '');
    ok(Unicode::Collate::getHST(0x1100, $v), 'L');
    ok(Unicode::Collate::getHST(0x1101, $v), 'L');
    ok(Unicode::Collate::getHST(0x1159, $v), 'L');
    ok(Unicode::Collate::getHST(0x115A, $v), ($v >= 20 ? 'L' : ''));
    ok(Unicode::Collate::getHST(0x115E, $v), ($v >= 20 ? 'L' : ''));
    ok(Unicode::Collate::getHST(0x115F, $v), 'L');
    ok(Unicode::Collate::getHST(0x1160, $v), 'V');
    ok(Unicode::Collate::getHST(0x1161, $v), 'V');
    ok(Unicode::Collate::getHST(0x11A0, $v), 'V');
    ok(Unicode::Collate::getHST(0x11A2, $v), 'V');
    ok(Unicode::Collate::getHST(0x11A3, $v), ($v >= 20 ? 'V' : ''));
    ok(Unicode::Collate::getHST(0x11A7, $v), ($v >= 20 ? 'V' : ''));
    ok(Unicode::Collate::getHST(0x11A8, $v), 'T');
    ok(Unicode::Collate::getHST(0x11AF, $v), 'T');
    ok(Unicode::Collate::getHST(0x11E0, $v), 'T');
    ok(Unicode::Collate::getHST(0x11F9, $v), 'T');
    ok(Unicode::Collate::getHST(0x11FA, $v), ($v >= 20 ? 'T' : ''));
    ok(Unicode::Collate::getHST(0x11FF, $v), ($v >= 20 ? 'T' : ''));
    ok(Unicode::Collate::getHST(0x3011, $v), '');
    ok(Unicode::Collate::getHST(0xA960, $v), ($v >= 20 ? 'L' : ''));
    ok(Unicode::Collate::getHST(0xA961, $v), ($v >= 20 ? 'L' : ''));
    ok(Unicode::Collate::getHST(0xA97C, $v), ($v >= 20 ? 'L' : ''));
    ok(Unicode::Collate::getHST(0xA97F, $v), '');
    ok(Unicode::Collate::getHST(0xABFF, $v), '');
    ok(Unicode::Collate::getHST(0xAC00, $v), 'LV');
    ok(Unicode::Collate::getHST(0xAC01, $v), 'LVT');
    ok(Unicode::Collate::getHST(0xAC1B, $v), 'LVT');
    ok(Unicode::Collate::getHST(0xAC1C, $v), 'LV');
    ok(Unicode::Collate::getHST(0xD7A3, $v), 'LVT');
    ok(Unicode::Collate::getHST(0xD7A4, $v), '');
    ok(Unicode::Collate::getHST(0xD7AF, $v), '');
    ok(Unicode::Collate::getHST(0xD7B0, $v), ($v >= 20 ? 'V' : ''));
    ok(Unicode::Collate::getHST(0xD7C0, $v), ($v >= 20 ? 'V' : ''));
    ok(Unicode::Collate::getHST(0xD7C6, $v), ($v >= 20 ? 'V' : ''));
    ok(Unicode::Collate::getHST(0xD7C7, $v), '');
    ok(Unicode::Collate::getHST(0xD7CA, $v), '');
    ok(Unicode::Collate::getHST(0xD7CB, $v), ($v >= 20 ? 'T' : ''));
    ok(Unicode::Collate::getHST(0xD7DD, $v), ($v >= 20 ? 'T' : ''));
    ok(Unicode::Collate::getHST(0xD7FB, $v), ($v >= 20 ? 'T' : ''));
    ok(Unicode::Collate::getHST(0xD7FC, $v), '');
    ok(Unicode::Collate::getHST(0xD7FF, $v), '');
    ok(Unicode::Collate::getHST(0xFFFF, $v), '');
    ok(Unicode::Collate::getHST(0x11100, $v), '');
    ok(Unicode::Collate::getHST(0x111FF, $v), '');
    ok(Unicode::Collate::getHST(0x2AC00, $v), '');
    ok(Unicode::Collate::getHST(0x10D7A3, $v), '');
}


# -*- mode: perl; -*-

# Test broot function (and bsqrt() function, since it is used by broot()).

# It is too slow to be simple included in bigfltpm.inc, where it would get
# executed 3 times.

# But it is better to test the numerical functionality, instead of not testing
# it at all.

use strict;                     # restrict unsafe constructs
use warnings;                   # enable optional warnings

use Test::More tests => 16;

use Math::BigFloat only => 'FastCalc';
use Math::BigInt;

my $mbf = "Math::BigFloat";
my $mbi = "Math::BigInt";

# 2 ** 240 =
# 1766847064778384329583297500742918515827483896875618958121606201292619776

test_broot('2', '240', 8, undef,
           '1073741824');
test_broot('2', '240', 9, undef,
           '106528681.3099908308759836475139583940127');
test_broot('2', '120', 9, undef,
           '10321.27324073880096577298929482324664787');
test_broot('2', '120', 17, undef,
           '133.3268493632747279600707813049418888729');

test_broot('2', '120', 8, undef,
           '32768');
test_broot('2', '60', 8, undef,
           '181.0193359837561662466161566988413540569');
test_broot('2', '60', 9, undef,
           '101.5936673259647663841091609134277286651');
test_broot('2', '60', 17, undef,
           '11.54672461623965153271017217302844672562');

sub test_broot {
    my ($x, $n, $y, $scale, $expected) = @_;

    my $s = $scale || 'undef';
    is($mbf->new($x)->bpow($n)->broot($y, $scale), $expected,
       "Try: $mbf->new($x)->bpow($n)->broot($y, $s) == $expected");

    # Math::BigInt returns the truncated integer part of the output, so remove
    # the dot an anything after it before comparing.

    $expected =~ s/\..*//;
    is($mbi->new($x)->bpow($n)->broot($y, $scale), $expected,
       "Try: $mbi->new($x)->bpow($n)->broot($y, $s) == $expected");
}

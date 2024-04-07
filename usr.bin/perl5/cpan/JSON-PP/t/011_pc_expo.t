# copied over from JSON::PC and modified to use JSON::PP
# copied over from JSON::XS and modified to use JSON::PP

use Test::More;
use strict;
use warnings;
BEGIN { plan tests => 8 + 2 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

#########################
my ($js,$obj);
my $pc = JSON::PP->new;

$js  = q|[-12.34]|;
$obj = $pc->decode($js);
is($obj->[0], -12.34, 'digit -12.34');
$js = $pc->encode($obj);
is($js,'[-12.34]', 'digit -12.34');

$js  = q|[-1.234e5]|;
$obj = $pc->decode($js);
is($obj->[0], -123400, 'digit -1.234e5');
{ #SKIP_IF_CPANEL
$js = $pc->encode($obj);
is($js,'[-123400]', 'digit -1.234e5');
}

$js  = q|[1.23E-4]|;
$obj = $pc->decode($js);
is($obj->[0], 0.000123, 'digit 1.23E-4');
$js = $pc->encode($obj);
is($js,'[0.000123]', 'digit 1.23E-4');


$js  = q|[1.01e+30]|;
$obj = $pc->decode($js);
is($obj->[0], 1.01e+30, 'digit 1.01e+30');
$js = $pc->encode($obj);
like($js,qr/\[(?:1.01[Ee]\+0?30|1010000000000000000000000000000)]/, 'digit 1.01e+30'); # RT-128589 (-Duselongdouble or -Dquadmath) 

my $vax_float = (pack("d",1) =~ /^[\x80\x10]\x40/);

if ($vax_float) {
    # VAX has smaller float range.
    $js  = q|[1.01e+37]|;
    $obj = $pc->decode($js);
    is($obj->[0], eval '1.01e+37', 'digit 1.01e+37');
    $js = $pc->encode($obj);
    like($js,qr/\[1.01[Ee]\+0?37\]/, 'digit 1.01e+37');
} else {
    $js  = q|[1.01e+67]|; # 30 -> 67 ... patched by H.Merijn Brand
    $obj = $pc->decode($js);
    is($obj->[0], eval '1.01e+67', 'digit 1.01e+67');
    $js = $pc->encode($obj);
    like($js,qr/\[1.01[Ee]\+0?67\]/, 'digit 1.01e+67');
}

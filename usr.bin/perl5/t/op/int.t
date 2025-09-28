#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
}

plan 19;

# compile time evaluation

my $test1_descr = 'compile time evaluation 1.234';
if (int(1.234) == 1) {pass($test1_descr)} else {fail($test1_descr)}

my $test2_descr = 'compile time evaluation -1.234';
if (int(-1.234) == -1) {pass($test2_descr)} else {fail($test2_descr)}

# run time evaluation

$x = 1.234;
cmp_ok(int($x), '==', 1, 'run time evaluation 1');
cmp_ok(int(-$x), '==', -1, 'run time evaluation -1');

$x = length("abc") % -10;
cmp_ok($x, '==', -7, 'subtract from string length');

{
    my $fail;
    use integer;
    $x = length("abc") % -10;
    $y = (3/-10)*-10;
    ok($x+$y == 3, 'x+y equals 3') or ++$fail;
    ok(abs($x) < 10, 'abs(x) < 10') or ++$fail;
    if ($fail) {
	diag("\$x == $x", "\$y == $y");
    }
}

@x = ( 6, 8, 10);
cmp_ok($x["1foo"], '==', 8, 'check bad strings still get converted');

# 4,294,967,295 is largest unsigned 32 bit integer

$x = 4294967303.15;
$y = int ($x);
is($y, "4294967303", 'check values > 32 bits work');

$y = int (-$x);

is($y, "-4294967303", 'negative value more than maximum unsigned 32 bit value');

$x = 4294967294.2;
$y = int ($x);

is($y, "4294967294", 'floating point value slightly less than the largest unsigned 32 bit');

$x = 4294967295.7;
$y = int ($x);

is($y, "4294967295", 'floating point value slightly more than largest unsigned 32 bit');

$x = 4294967296.11312;
$y = int ($x);

is($y, "4294967296", 'floating point value more than largest unsigned 32 bit');

$y = int(279964589018079/59);
cmp_ok($y, '==', 4745162525730, 'compile time division, result of about 42 bits');

$y = 279964589018079;
$y = int($y/59);
cmp_ok($y, '==', 4745162525730, 'run time divison, result of about 42 bits');

SKIP:
{   # see #126635
    my $large;
    $large = eval "0xffff_ffff" if $Config::Config{ivsize} == 4;
    $large = eval "0xffff_ffff_ffff_ffff" if $Config::Config{ivsize} == 8;
    $large or skip "Unusual ivsize", 1;
    for my $x ($large, -1) {
        cmp_ok($x, "==", int($x), "check $x == int($x)");
    }
}

is(1+"0x10", 1, "check string '0x' prefix not treated as hex");
is(1+"0b10", 1, "check string '0b' prefix not treated as binary");

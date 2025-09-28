#!perl -T
use Test::More;
use version;

BEGIN {
    eval "use Test::Taint";
    if ($@) {
	plan skip_all => "No Test::Taint available";
    } else {
	plan tests => 6;
    }
}

taint_checking_ok();
my $v = 'v1.2.3';
taint($v);
tainted_ok($v, 'Set string as tainted');
my $v2 = version->parse($v);
isnt("$v2", '', 'Correctly parsed the tainted string');
tainted_ok($v2, 'Resulting version object is tainted');

my $vs = "$v2";
tainted_ok($vs, 'Stringified object still tainted');
is $v2, 'v1.2.3', 'Comparison to tainted object';

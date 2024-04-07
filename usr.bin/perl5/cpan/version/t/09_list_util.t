# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use strict;
use_ok("version", 0.9929);
use Test::More;

BEGIN {
    eval "use List::Util qw(reduce)";
    if ($@) {
	plan skip_all => "No List::Util::reduce() available";
    } else {
	plan tests => 3;
    }
}

# do strict lax tests in a sub to isolate a package to test importing
# use again to get the import()
use List::Util qw(reduce);
{
    my $fail = 0;
    my $ret = reduce {
	version->parse($a);
	$fail++ unless defined $a;
	1
    } "0.039", "0.035";
    is $fail, 0, 'reduce() with parse';
}

{
    my $fail = 0;
    my $ret = reduce {
	version->qv($a);
	$fail++ unless defined $a;
	1
    } "0.039", "0.035";
    is $fail, 0, 'reduce() with qv';
}

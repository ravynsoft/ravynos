use strict;
use warnings;

use lib 't/lib';

use Test::Tester;

use MyTest;

my $test = Test::Builder->new;
$test->plan(tests => 2);

sub deeper
{
	MyTest::ok(1);
}

{

	my @results = run_tests(
		sub {
			MyTest::ok(1);
			deeper();
		}
	);

	local $Test::Builder::Level = 0;
	$test->is_num($results[1]->{depth}, 1, "depth 1");
	$test->is_num($results[2]->{depth}, 2, "deeper");
}


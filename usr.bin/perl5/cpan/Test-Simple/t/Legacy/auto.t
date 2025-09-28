use strict;
use warnings;

use lib 't/lib';

use Test::Tester tests => 6;

use SmallTest;

use MyTest;

{
	my ($prem, @results) = run_tests(
		sub { MyTest::ok(1, "run pass")}
	);

	is_eq($results[0]->{name}, "run pass");
	is_num($results[0]->{ok}, 1);
}

{
	my ($prem, @results) = run_tests(
		sub { MyTest::ok(0, "run fail")}
	);

	is_eq($results[0]->{name}, "run fail");
	is_num($results[0]->{ok}, 0);
}

is_eq(ref(SmallTest::getTest()), "Test::Tester::Delegate");

is_eq(
	SmallTest::getTest()->can('ok'),
	Test::Builder->can('ok'),
	"Delegate->can() returns the sub from the inner object",
);

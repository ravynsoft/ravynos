use strict;

use Test::Tester;

use Data::Dumper qw(Dumper);

my $test = Test::Builder->new;
$test->plan(tests => 54);

my $cap;

{
	$cap = Test::Tester->capture;
	my ($prem, @results) = run_tests(
		sub {$cap->ok(1, "run pass")}
	);

	local $Test::Builder::Level = 0;

	$test->is_eq($prem, "", "run pass no prem");
	$test->is_num(scalar (@results), 1, "run pass result count");

	my $res = $results[0];

	$test->is_eq($res->{name}, "run pass", "run pass name");
	$test->is_eq($res->{ok}, 1, "run pass ok");
	$test->is_eq($res->{actual_ok}, 1, "run pass actual_ok");
	$test->is_eq($res->{reason}, "", "run pass reason");
	$test->is_eq($res->{type}, "", "run pass type");
	$test->is_eq($res->{diag}, "", "run pass diag");
	$test->is_num($res->{depth}, 0, "run pass depth");
}

{
	my ($prem, @results) = run_tests(
		sub {$cap->ok(0, "run fail")}
	);

	local $Test::Builder::Level = 0;

	$test->is_eq($prem, "", "run fail no prem");
	$test->is_num(scalar (@results), 1, "run fail result count");

	my $res = $results[0];

	$test->is_eq($res->{name}, "run fail", "run fail name");
	$test->is_eq($res->{actual_ok}, 0, "run fail actual_ok");
	$test->is_eq($res->{ok}, 0, "run fail ok");
	$test->is_eq($res->{reason}, "", "run fail reason");
	$test->is_eq($res->{type}, "", "run fail type");
	$test->is_eq($res->{diag}, "", "run fail diag");
	$test->is_num($res->{depth}, 0, "run fail depth");
}

{
	my ($prem, @results) = run_tests(
		sub {$cap->skip("just because")}
	);

	local $Test::Builder::Level = 0;

	$test->is_eq($prem, "", "skip no prem");
	$test->is_num(scalar (@results), 1, "skip result count");

	my $res = $results[0];

	$test->is_eq($res->{name}, "", "skip name");
	$test->is_eq($res->{actual_ok}, 1, "skip actual_ok");
	$test->is_eq($res->{ok}, 1, "skip ok");
	$test->is_eq($res->{reason}, "just because", "skip reason");
	$test->is_eq($res->{type}, "skip", "skip type");
	$test->is_eq($res->{diag}, "", "skip diag");
	$test->is_num($res->{depth}, 0, "skip depth");
}

{
	my ($prem, @results) = run_tests(
		sub {$cap->todo_skip("just because")}
	);

	local $Test::Builder::Level = 0;

	$test->is_eq($prem, "", "todo_skip no prem");
	$test->is_num(scalar (@results), 1, "todo_skip result count");

	my $res = $results[0];

	$test->is_eq($res->{name}, "", "todo_skip name");
	$test->is_eq($res->{actual_ok}, 0, "todo_skip actual_ok");
	$test->is_eq($res->{ok}, 1, "todo_skip ok");
	$test->is_eq($res->{reason}, "just because", "todo_skip reason");
	$test->is_eq($res->{type}, "todo_skip", "todo_skip type");
	$test->is_eq($res->{diag}, "", "todo_skip diag");
	$test->is_num($res->{depth}, 0, "todo_skip depth");
}

{
	my ($prem, @results) = run_tests(
		sub {$cap->diag("run diag")}
	);

	local $Test::Builder::Level = 0;

	$test->is_eq($prem, "run diag\n", "run diag prem");
	$test->is_num(scalar (@results), 0, "run diag result count");
}

{
	my ($prem, @results) = run_tests(
		sub {
			$cap->ok(1, "multi pass");
			$cap->diag("multi pass diag1");
			$cap->diag("multi pass diag2");
			$cap->ok(0, "multi fail");
			$cap->diag("multi fail diag");
		}
	);

	local $Test::Builder::Level = 0;

	$test->is_eq($prem, "", "run multi no prem");
	$test->is_num(scalar (@results), 2, "run multi result count");

	my $res_pass = $results[0];

	$test->is_eq($res_pass->{name}, "multi pass", "run multi pass name");
	$test->is_eq($res_pass->{actual_ok}, 1, "run multi pass actual_ok");
	$test->is_eq($res_pass->{ok}, 1, "run multi pass ok");
	$test->is_eq($res_pass->{reason}, "", "run multi pass reason");
	$test->is_eq($res_pass->{type}, "", "run multi pass type");
	$test->is_eq($res_pass->{diag}, "multi pass diag1\nmulti pass diag2\n",
		"run multi pass diag");
	$test->is_num($res_pass->{depth}, 0, "run multi pass depth");

	my $res_fail = $results[1];

	$test->is_eq($res_fail->{name}, "multi fail", "run multi fail name");
	$test->is_eq($res_pass->{actual_ok}, 1, "run multi fail actual_ok");
	$test->is_eq($res_fail->{ok}, 0, "run multi fail ok");
	$test->is_eq($res_pass->{reason}, "", "run multi fail reason");
	$test->is_eq($res_pass->{type}, "", "run multi fail type");
	$test->is_eq($res_fail->{diag}, "multi fail diag\n", "run multi fail diag");
	$test->is_num($res_pass->{depth}, 0, "run multi fail depth");
}


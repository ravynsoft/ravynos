use strict;

use Test::Tester;

use Data::Dumper qw(Dumper);

my $test = Test::Builder->new;
$test->plan(tests => 139);

my $cap;

$cap = Test::Tester->capture;

my @tests = (
	[
		'pass',
		'$cap->ok(1, "pass");',
		{
			name => "pass",
			ok => 1,
			actual_ok => 1,
			reason => "",
			type => "",
			diag => "",
			depth => 0,
		},
	],
	[
		'pass diag',
		'$cap->ok(1, "pass diag");
		$cap->diag("pass diag1");
		$cap->diag("pass diag2");',
		{
			name => "pass diag",
			ok => 1,
			actual_ok => 1,
			reason => "",
			type => "",
			diag => "pass diag1\npass diag2\n",
			depth => 0,
		},
	],
	[
		'pass diag no \\n',
		'$cap->ok(1, "pass diag");
		$cap->diag("pass diag1");
		$cap->diag("pass diag2");',
		{
			name => "pass diag",
			ok => 1,
			actual_ok => 1,
			reason => "",
			type => "",
			diag => "pass diag1\npass diag2",
			depth => 0,
		},
	],
	[
		'fail',
		'$cap->ok(0, "fail");
		$cap->diag("fail diag");',
		{
			name => "fail",
			ok => 0,
			actual_ok => 0,
			reason => "",
			type => "",
			diag => "fail diag\n",
			depth => 0,
		},
	],
	[
		'skip',
		'$cap->skip("just because");',
		{
			name => "",
			ok => 1,
			actual_ok => 1,
			reason => "just because",
			type => "skip",
			diag => "",
			depth => 0,
		},
	],
	[
		'todo_skip',
		'$cap->todo_skip("why not");',
		{
			name => "",
			ok => 1,
			actual_ok => 0,
			reason => "why not",
			type => "todo_skip",
			diag => "",
			depth => 0,
		},
	],
    [
        'pass diag qr',
        '$cap->ok(1, "pass diag qr");
        $cap->diag("pass diag qr");',
        {
            name => "pass diag qr",
            ok => 1,
            actual_ok => 1,
            reason => "",
            type => "",
            diag => qr/pass diag qr/,
            depth => 0,
        },
    ],
    [
        'fail diag qr',
        '$cap->ok(0, "fail diag qr");
        $cap->diag("fail diag qr");',
        {
            name => "fail diag qr",
            ok => 0,
            actual_ok => 0,
            reason => "",
            type => "",
            diag => qr/fail diag qr/,
            depth => 0,
        },
    ],
);

my $big_code = "";
my @big_expect;

foreach my $test (@tests)
{
	my ($name, $code, $expect) = @$test;

	$big_code .= "$code\n";
	push(@big_expect, $expect);

	my $test_sub = eval "sub {$code}";

	check_test($test_sub, $expect, $name);
}

my $big_test_sub = eval "sub {$big_code}";

check_tests($big_test_sub, \@big_expect, "run all");

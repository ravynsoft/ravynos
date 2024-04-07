use strict; use warnings;
use Memoize;
use Test::More tests => 17;

# here we test whether memoization actually has the desired effect

my ($fib, $ns1_calls, $ns2_calls, $total_calls) = ([0,1], 1, 1, 1+1);
while (@$fib < 23) {
	push @$fib, $$fib[-1] + $$fib[-2];
	my $n_calls = 1 + $ns1_calls + $ns2_calls;
	$total_calls += $n_calls;
	($ns2_calls, $ns1_calls) = ($ns1_calls, $n_calls);
}

my $num_calls;
sub fib {
	++$num_calls;
	my $n = shift;
	return $n if $n < 2;
	fib($n-1) + fib($n-2);
}

my @s1 = map 0+fib($_), 0 .. $#$fib;
is_deeply \@s1, $fib, 'unmemoized Fibonacci works';
is $num_calls, $total_calls, '... with the expected amount of calls';

undef $num_calls;
memoize 'fib';

my @f1 = map 0+fib($_), 0 .. $#$fib;
my @f2 = map 0+fib($_), 0 .. $#$fib;
is_deeply \@f1, $fib, 'memoized Fibonacci works';
is $num_calls, @$fib, '... with a minimal amount of calls';

########################################################################

my $timestamp;
sub timelist { (++$timestamp) x $_[0] }

memoize('timelist');

my $t1 = [timelist(1)];
is_deeply [timelist(1)], $t1, 'memoizing a volatile function makes it stable';
my $t7 = [timelist(7)];
isnt @$t1, @$t7, '... unless the arguments change';
is_deeply $t7, [($$t7[0]) x 7], '... which leads to the expected new return value';
is_deeply [timelist(7)], $t7, '... which then also stays stable';

sub con { wantarray ? 'list' : 'scalar' }
memoize('con');
is scalar(con(1)), 'scalar', 'scalar context propgates properly';
is_deeply [con(1)], ['list'], 'list context propgates properly';

########################################################################

my %underlying;
sub ExpireTest::TIEHASH { bless \%underlying, shift }
sub ExpireTest::EXISTS  { exists $_[0]{$_[1]} }
sub ExpireTest::FETCH   { $_[0]{$_[1]} }
sub ExpireTest::STORE   { $_[0]{$_[1]} = $_[2] }

my %CALLS;
sub id {
	my($arg) = @_;
	++$CALLS{$arg};
	$arg;
}

tie my %cache => 'ExpireTest';
memoize 'id',
	SCALAR_CACHE => [HASH => \%cache],
	LIST_CACHE => 'FAULT';

my $arg = [1..3, 1, 2, 1];
is_deeply [map scalar(id($_)), @$arg], $arg, 'memoized function sanity check';
is_deeply \%CALLS, {1=>1,2=>1,3=>1}, 'amount of initial calls per arg as expected';

delete $underlying{1};
$arg = [1..3];
is_deeply [map scalar(id($_)), @$arg], $arg, 'memoized function sanity check';
is_deeply \%CALLS, {1=>2,2=>1,3=>1}, 'amount of calls per arg after expiring 1 as expected';

delete @underlying{1,2};
is_deeply [map scalar(id($_)), @$arg], $arg, 'memoized function sanity check';
is_deeply \%CALLS, {1=>3,2=>2,3=>1}, 'amount of calls per arg after expiring 1 & 2 as expected';

########################################################################

my $fail;
$SIG{__WARN__} = sub { if ( $_[0] =~ /^Deep recursion/ ) { $fail = 1 } else { warn $_[0] } };

my $limit;
sub deep_probe { deep_probe() if ++$limit < 100_000 and not $fail }
sub deep_test { no warnings "recursion"; deep_test() if $limit-- > 0 }
memoize "deep_test";

SKIP: {
	deep_probe();
	skip "no warning after $limit recursive calls (maybe PERL_SUB_DEPTH_WARN was raised?)", 1 if not $fail;
	undef $fail;
	deep_test();
	ok !$fail, 'no recursion warning thrown from Memoize';
}

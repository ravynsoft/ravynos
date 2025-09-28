use strict; use warnings;

use Memoize qw(memoize unmemoize);
use Test::More
	("$]" < 5.009 || "$]" >= 5.010001) && eval { require threads; 1 }
		? ( tests => 8 )
		: ( skip_all => $@ );

my $i;
sub count_up { ++$i }

memoize('count_up');
my $cached = count_up();

is count_up(), $cached, 'count_up() is memoized';

my $got = threads->new(sub {
	local $@ = '';
	my $v = eval { count_up() };
	+{ E => $@, V => $v };
})->join;

is $got->{E}, '', 'calling count_up() in another thread works';
is $got->{V}, $cached, '... and returns the same result';
is count_up(), $cached, '... whereas count_up() on the main thread is unaffected';

$got = threads->new(sub {
	local $@ = '';
	my $u = eval { unmemoize('count_up') };
	my $v = eval { count_up() };
	+{ E => $@, U => $u, V => $v };
})->join;

is $got->{E}, '', 'unmemoizing count_up() in another thread works';
is ref($got->{U}), 'CODE', '... and returns a coderef as expected';
is $got->{V}, 1+$cached, '... and does in fact unmemoize the function';
is count_up(), $cached, '... whereas count_up() on the main thread is unaffected';

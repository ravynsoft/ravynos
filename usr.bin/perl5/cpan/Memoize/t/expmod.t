use strict; use warnings;
use Memoize;
use Memoize::Expire;
use Test::More tests => 22;

tie my %h => 'Memoize::Expire', HASH => \my %backing;

$h{foo} = 1;
my $num_keys = keys %backing;
my $num_refs = grep ref, values %backing;

is $h{foo}, 1, 'setting and getting a plain scalar value works';
cmp_ok $num_keys, '>', 0, 'HASH option is effective';
is $num_refs, 0, 'backing storage contains only plain scalars';

$h{bar} = my $bar = {};
my $num_keys_step2 = keys %backing;
$num_refs = grep ref, values %backing;

is ref($h{bar}), ref($bar), 'setting and getting a reference value works';
cmp_ok $num_keys, '<', $num_keys_step2, 'HASH option is effective';
is $num_refs, 1, 'backing storage contains only one reference';

my $contents = eval { +{ %h } };

ok defined $contents, 'dumping the tied hash works';
is_deeply $contents, { foo => 1, bar => $bar }, ' ... with the expected contents';

########################################################################

my $RETURN = 1;
my %CALLS;

tie my %cache => 'Memoize::Expire', NUM_USES => 2;
memoize sub { ++$CALLS{$_[0]}; $RETURN },
	SCALAR_CACHE => [ HASH => \%cache ],
	LIST_CACHE => 'FAULT',
	INSTALL => 'call';

is call($_), 1, "$_ gets new val" for 0..3;

is_deeply \%CALLS, {0=>1,1=>1,2=>1,3=>1}, 'memoized function called once per argument';

$RETURN = 2;
is call(1), 1, '1 expires';
is call(1), 2, '1 gets new val';
is call(2), 1, '2 expires';

is_deeply \%CALLS, {0=>1,1=>2,2=>1,3=>1}, 'memoized function called for expired argument';

$RETURN = 3;
is call(0), 1, '0 expires';
is call(1), 2, '1 expires';
is call(2), 3, '2 gets new val';
is call(3), 1, '3 expires';

is_deeply \%CALLS, {0=>1,1=>2,2=>2,3=>1}, 'memoized function called for other expired argument';

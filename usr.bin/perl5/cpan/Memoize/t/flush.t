use strict; use warnings;
use Memoize qw(flush_cache memoize);
use Test::More tests => 9;

my $V = 100;
sub VAL { $V }

ok eval { memoize('VAL'); 1 }, 'memozing the test function';

is VAL(), 100, '... with the expected return value';
$V = 200;
is VAL(), 100, '... which is expectedly sticky';

ok eval { flush_cache('VAL'); 1 }, 'flusing the cache by name works';

is VAL(), 200, '... with the expected new return value';
$V = 300;
is VAL(), 200, '... which is expectedly sticky';

ok eval { flush_cache(\&VAL); 1 }, 'flusing the cache by name works';

is VAL(), 300, '... with the expected new return value';
$V = 400;
is VAL(), 300, '... which is expectedly sticky';
